"""Load UEF's indexed registries and their individual detail manifests.

All manifest paths are repository-root-relative. The loader keeps the historic
in-memory document shapes so selection and rendering code can remain focused on
UEF records rather than file layout.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any


class UEFRegistryError(ValueError):
    """Raised when a registry index or one of its detail manifests is invalid."""


def _read_json(path: Path, description: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise UEFRegistryError(f"Cannot read {description} at {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise UEFRegistryError(f"{description} must be a JSON object: {path}")
    return value


def _under_root(root: Path, relative: Any, description: str) -> Path:
    if not isinstance(relative, str) or not relative.strip():
        raise UEFRegistryError(f"{description} must be a non-empty repository-relative path")
    path = Path(relative.replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise UEFRegistryError(f"{description} must stay inside the UEF checkout")
    resolved_root = root.resolve()
    resolved = (resolved_root / path).resolve()
    if not resolved.is_relative_to(resolved_root):
        raise UEFRegistryError(f"{description} resolves outside the UEF checkout")
    return resolved


def load_registry_document(uef_root: Path, path: str | Path) -> dict[str, Any]:
    """Read one canonical index using a repository-relative path."""
    root = uef_root.expanduser().resolve()
    relative = path.as_posix() if isinstance(path, Path) else path
    current = _under_root(root, relative, "UEF registry path")
    return _read_json(current, "UEF registry index")


def _load_detail(root: Path, relative: Any, identity: str, expected_id: str | None = None) -> dict[str, Any]:
    path = _under_root(root, relative, f"manifest path for {identity}")
    document = _read_json(path, f"manifest for {identity}")
    if expected_id is not None and document.get("id") != expected_id:
        raise UEFRegistryError(
            f"Manifest id {document.get('id')!r} does not match registry id {expected_id!r}"
        )
    return document


def expand_entry_list(
    uef_root: Path, document: dict[str, Any], section: str, *, id_key: str = "id"
) -> dict[str, Any]:
    """Expand an index list of ``{id, path}`` references into legacy records."""
    entries = document.get(section)
    if not isinstance(entries, list):
        raise UEFRegistryError(f"UEF registry section {section!r} must be an array")
    # A pre-indexed UEF checkout stores complete records directly in this list.
    if entries and all(isinstance(item, dict) and "path" not in item for item in entries):
        return dict(document)
    expanded: list[dict[str, Any]] = []
    identifiers: set[str] = set()
    for reference in entries:
        if not isinstance(reference, dict):
            raise UEFRegistryError(f"UEF registry {section!r} references must be objects")
        identifier = reference.get(id_key)
        if not isinstance(identifier, str) or not identifier.strip():
            raise UEFRegistryError(f"UEF registry {section!r} entry is missing {id_key!r}")
        normalized = identifier.casefold()
        if normalized in identifiers:
            raise UEFRegistryError(f"Duplicate UEF registry id in {section}: {identifier}")
        identifiers.add(normalized)
        expanded.append(_load_detail(uef_root, reference.get("path"), identifier, identifier))
    result = dict(document)
    result[section] = expanded
    return result


def expand_entry_map(
    uef_root: Path, document: dict[str, Any], section: str, *, alias_values: bool = False
) -> dict[str, Any]:
    """Expand a key-to-path registry table into its original key-to-record map."""
    references = document.get(section)
    if not isinstance(references, dict):
        raise UEFRegistryError(f"UEF registry section {section!r} must be an object")
    # Older UEF releases store module/control records directly in these maps.
    if section == "aliases" and references and all(
        isinstance(value, (str, list)) and not (isinstance(value, str) and value.endswith(".json"))
        for value in references.values()
    ):
        return dict(document)
    if references and all(isinstance(value, dict) for value in references.values()):
        return dict(document)
    expanded: dict[str, Any] = {}
    for key, relative in references.items():
        if not isinstance(key, str) or not key.strip():
            raise UEFRegistryError(f"UEF registry {section!r} contains an empty key")
        detail = _load_detail(uef_root, relative, f"{section}.{key}")
        if alias_values:
            target = detail.get("target")
            if not isinstance(target, (str, list)):
                raise UEFRegistryError(f"UEF alias manifest {key!r} must contain a string or list target")
            expanded[key] = target
        else:
            expanded[key] = detail
    result = dict(document)
    result[section] = expanded
    return result


def load_module_manifest(uef_root: Path) -> dict[str, Any]:
    """Return the combined UEF module document from its index and entry files."""
    root = uef_root.expanduser().resolve()
    api = _read_json(root / "uef_api.json", "UEF API manifest")
    declared_path = api.get("module_registry")
    if not isinstance(declared_path, str) or not declared_path.strip():
        raise UEFRegistryError("UEF API manifest does not declare module_registry")
    document = load_registry_document(root, declared_path)
    return expand_entry_map(root, document, "modules")


def load_control_ir_manifest(uef_root: Path, registry_path: str | Path | None = None) -> dict[str, Any]:
    """Return the combined ControlIR dispatch document from its per-entry files."""
    root = uef_root.expanduser().resolve()
    if registry_path is None:
        api = _read_json(root / "uef_api.json", "UEF API manifest")
        try:
            registry_path = api["ucon_api"]["catalogue_manifest"]
        except (KeyError, TypeError) as exc:
            raise UEFRegistryError("UEF API manifest does not declare the ControlIR registry") from exc
    document = load_registry_document(root, registry_path)
    for section in ("control_nodes", "algorithms"):
        document = expand_entry_map(root, document, section)
    document = expand_entry_map(root, document, "aliases", alias_values=True)
    return document
