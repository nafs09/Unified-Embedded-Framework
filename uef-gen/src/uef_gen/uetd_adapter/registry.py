"""Load UETD's indexed registries and their individual detail manifests.

All manifest paths are repository-root-relative. The loader keeps the historic
in-memory document shapes so selection and rendering code can remain focused on
UETD records rather than file layout.
"""

from __future__ import annotations
import json
import re
from pathlib import Path
from typing import Any

class UETDRegistryError(ValueError):
    """Raised when a registry index or one of its detail manifests is invalid."""

def _read_json(path: Path, description: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise UETDRegistryError(f"Cannot read {description} at {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise UETDRegistryError(f"{description} must be a JSON object: {path}")
    return value

def _under_root(root: Path, relative: Any, description: str) -> Path:
    if not isinstance(relative, str) or not relative.strip():
        raise UETDRegistryError(f"{description} must be a non-empty repository-relative path")
    path = Path(relative.replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise UETDRegistryError(f"{description} must stay inside the UETD checkout")
    resolved_root = root.resolve()
    resolved = (resolved_root / path).resolve()
    if not resolved.is_relative_to(resolved_root):
        raise UETDRegistryError(f"{description} resolves outside the UETD checkout")
    return resolved

def load_registry_document(uetd_root: Path, path: str | Path) -> dict[str, Any]:
    """Read one canonical index using a repository-relative path."""
    root = uetd_root.expanduser().resolve()
    relative = path.as_posix() if isinstance(path, Path) else path
    current = _under_root(root, relative, "UETD registry path")
    return _read_json(current, "UETD registry index")

def _load_detail(root: Path, relative: Any, identity: str, expected_id: str | None = None) -> dict[str, Any]:
    path = _under_root(root, relative, f"manifest path for {identity}")
    document = _read_json(path, f"manifest for {identity}")
    if expected_id is not None and document.get("id") != expected_id:
        raise UETDRegistryError(f"Manifest id {document.get('id')!r} does not match registry id {expected_id!r}")
    return document

def expand_entry_list(uetd_root: Path, document: dict[str, Any], section: str, *, id_key: str = "id") -> dict[str, Any]:
    """Expand an index list of ``{id, path}`` references into legacy records."""
    entries = document.get(section)
    if not isinstance(entries, list):
        raise UETDRegistryError(f"UETD registry section {section!r} must be an array")
    # A pre-indexed UETD checkout stores complete records directly in this list.
    if entries and all(isinstance(item, dict) and "path" not in item for item in entries):
        return dict(document)
    expanded: list[dict[str, Any]] = []
    identifiers: set[str] = set()
    for reference in entries:
        if not isinstance(reference, dict):
            raise UETDRegistryError(f"UETD registry {section!r} references must be objects")
        identifier = reference.get(id_key)
        if not isinstance(identifier, str) or not identifier.strip():
            raise UETDRegistryError(f"UETD registry {section!r} entry is missing {id_key!r}")
        normalized = identifier.casefold()
        if normalized in identifiers:
            raise UETDRegistryError(f"Duplicate UETD registry id in {section}: {identifier}")
        identifiers.add(normalized)
        expanded.append(_load_detail(uetd_root, reference.get("path"), identifier, identifier))
    result = dict(document)
    result[section] = expanded
    return result

def expand_entry_map(uetd_root: Path, document: dict[str, Any], section: str, *, alias_values: bool = False) -> dict[str, Any]:
    """Expand a key-to-path registry table into its original key-to-record map."""
    references = document.get(section)
    if not isinstance(references, dict):
        raise UETDRegistryError(f"UETD registry section {section!r} must be an object")
    # Older UETD releases store module/control records directly in these maps.
    if (
        section == "aliases"
        and references
        and all(isinstance(value, (str, list)) and not (isinstance(value, str) and value.endswith(".json")) for value in references.values())
    ):
        return dict(document)
    if references and all(isinstance(value, dict) and "manifest" not in value and "path" not in value for value in references.values()):
        return dict(document)
    expanded: dict[str, Any] = {}
    descriptor_owners: dict[str, str] = {}
    for key, reference in references.items():
        if not isinstance(key, str) or not key.strip():
            raise UETDRegistryError(f"UETD registry {section!r} contains an empty key")
        descriptor = None
        if section == "modules":
            if not isinstance(reference, dict):
                raise UETDRegistryError(f"UETD module {key!r} registry entry must include manifest and DSL_descriptor")
            relative = reference.get("manifest", reference.get("path"))
            descriptor = reference.get("DSL_descriptor")
            if not isinstance(descriptor, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", descriptor):
                raise UETDRegistryError(f"UETD module {key!r} registry entry requires an identifier-shaped DSL_descriptor")
        else:
            relative = reference
        detail = _load_detail(uetd_root, relative, f"{section}.{key}")
        if descriptor is not None:
            if descriptor in descriptor_owners:
                raise UETDRegistryError(f"UETD DSL_descriptor {descriptor!r} is assigned to both {descriptor_owners[descriptor]!r} and {key!r}")
            descriptor_owners[descriptor] = key
            if "DSL_descriptor" in detail:
                raise UETDRegistryError(f"UETD module {key!r} duplicates DSL_descriptor in its manifest and registry index")
            detail["DSL_descriptor"] = descriptor
        if alias_values:
            target = detail.get("target")
            if not isinstance(target, (str, list)):
                raise UETDRegistryError(f"UETD alias manifest {key!r} must contain a string or list target")
            expanded[key] = target
        else:
            expanded[key] = detail
    result = dict(document)
    result[section] = expanded
    return result

def load_module_manifest(uetd_root: Path) -> dict[str, Any]:
    """Return the combined UETD module document from its index and entry files."""
    root = uetd_root.expanduser().resolve()
    api = _read_json(root / "api.json", "UETD API manifest")
    declared_path = api.get("module_registry")
    if not isinstance(declared_path, str) or not declared_path.strip():
        raise UETDRegistryError("UETD API manifest does not declare module_registry")
    document = load_registry_document(root, declared_path)
    return expand_entry_map(root, document, "modules")

def load_future_catalogue(uetd_root: Path) -> dict[str, Any]:
    """Load UETD's non-selectable design catalogue and verify linked contracts.

    Paths normally resolve beneath the UETD checkout. ``@repo/`` paths resolve
    beneath the containing repository root so UETD can link to future language
    and deployment contracts owned by the sibling uef-gen package without
    permitting arbitrary path traversal.
    """
    from jsonschema import Draft202012Validator
    root = uetd_root.expanduser().resolve()
    api = _read_json(root / "api.json", "UETD API manifest")
    declared_path = api.get("future_catalogue")
    if not isinstance(declared_path, str) or not declared_path.strip():
        raise UETDRegistryError("UETD API manifest does not declare future_catalogue")
    catalogue = load_registry_document(root, declared_path)
    schema_path = catalogue.get("registry_schema")
    schema_file = _under_root(root, schema_path, "future catalogue schema path")
    schema = _read_json(schema_file, "UETD future catalogue schema")
    errors = sorted(Draft202012Validator(schema).iter_errors(catalogue), key=lambda error: list(error.absolute_path))
    if errors:
        first = errors[0]
        location = "/".join(str(item) for item in first.absolute_path) or "<root>"
        raise UETDRegistryError(f"UETD future catalogue is invalid at {location}: {first.message}")
    repository_root = root.parent.resolve()
    for item_id, item in catalogue["items"].items():
        for field in ("contract_file", "api_outline"):
            referenced = item.get(field)
            if referenced is None:
                continue
            referenced = referenced.split("#", 1)[0]
            if referenced.startswith("@repo/"):
                relative = referenced.removeprefix("@repo/")
                owner_root = repository_root
            else:
                relative = referenced
                owner_root = root
            referenced_path = _under_root(owner_root, relative, f"future {field} for {item_id}")
            if not referenced_path.is_file():
                raise UETDRegistryError(f"UETD future {field} for {item_id!r} does not exist: {referenced_path}")
    for schema_id, relative in catalogue.get("schemas", {}).items():
        schema_file = _under_root(root, relative, f"future schema {schema_id}")
        if not schema_file.is_file():
            raise UETDRegistryError(f"UETD future schema {schema_id!r} does not exist: {schema_file}")
    return catalogue
