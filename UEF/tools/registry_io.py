"""Read and write UEF's split JSON registries for repository maintenance tools."""
from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]


class RegistryFormatError(ValueError):
    """Raised when a registry index or one of its detail files is malformed."""


def _read(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise RegistryFormatError(f"cannot read registry document {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise RegistryFormatError(f"registry document must be a JSON object: {path}")
    return value


def _write(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def _under_root(root: Path, relative: str, label: str) -> Path:
    path = Path(relative.replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise RegistryFormatError(f"{label} must stay inside the UEF checkout")
    candidate = (root / path).resolve()
    if not candidate.is_relative_to(root.resolve()):
        raise RegistryFormatError(f"{label} resolves outside the UEF checkout")
    return candidate


def _read_index(root: Path, relative: str) -> dict[str, Any]:
    path = _under_root(root, relative, "registry index path")
    return _read(path)


def load_module_registry(root: Path = ROOT) -> dict[str, Any]:
    """Return the combined module document shape from index and detail files."""
    root = root.resolve()
    api = _read(root / "uef_api.json")
    registry_path = api.get("module_registry")
    if not isinstance(registry_path, str) or not registry_path.strip():
        raise RegistryFormatError("uef_api.json must declare module_registry")
    document = _read_index(root, registry_path)
    references = document.get("modules")
    if not isinstance(references, dict):
        raise RegistryFormatError("module registry must contain a modules object")
    if references and all(isinstance(value, dict) for value in references.values()):
        return document
    modules: dict[str, Any] = {}
    for key, relative in references.items():
        if not isinstance(key, str) or not isinstance(relative, str):
            raise RegistryFormatError("module registry entries must map names to manifest paths")
        detail = _read(_under_root(root, relative, f"module manifest {key}"))
        modules[key] = detail
    result = dict(document)
    result["modules"] = modules
    return result


def load_control_ir_registry(root: Path = ROOT) -> dict[str, Any]:
    """Return the combined ControlIR data shape used by scaffold maintenance."""
    root = root.resolve()
    document = _read_index(root, "registry/control_ir/registry.json")
    for section in ("control_nodes", "algorithms", "aliases"):
        references = document.get(section)
        if not isinstance(references, dict):
            raise RegistryFormatError(f"ControlIR registry {section} must be an object")
        if section == "aliases" and references and all(
            isinstance(value, (str, list)) and not (isinstance(value, str) and value.endswith(".json"))
            for value in references.values()
        ):
            continue
        if references and all(isinstance(value, dict) for value in references.values()):
            continue
        expanded: dict[str, Any] = {}
        for key, relative in references.items():
            if not isinstance(key, str) or not isinstance(relative, str):
                raise RegistryFormatError(f"ControlIR registry {section} entries must map keys to paths")
            detail = _read(_under_root(root, relative, f"ControlIR manifest {key}"))
            expanded[key] = detail.get("target") if section == "aliases" else detail
        document[section] = expanded
    return document


def _slug(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.casefold()).strip("-")


def write_module_registry(document: dict[str, Any], root: Path = ROOT) -> None:
    """Persist module records as one JSON file per module plus the small index."""
    root = root.resolve()
    modules = document.get("modules")
    if not isinstance(modules, dict):
        raise RegistryFormatError("module document must contain a modules object")
    index = dict(document)
    references: dict[str, str] = {}
    for key, record in sorted(modules.items()):
        if not isinstance(key, str) or not isinstance(record, dict):
            raise RegistryFormatError("module names and records must be strings and objects")
        if key.startswith("/") or "\\" in key or any(part in {"", ".", ".."} for part in key.split("/")):
            raise RegistryFormatError(f"unsafe module name: {key!r}")
        relative = f"registry/modules/manifest/{key}.json"
        _write(_under_root(root, relative, "module manifest path"), record)
        references[key] = relative
    index["modules"] = references
    _write(root / "registry" / "modules" / "registry.json", index)


def write_control_ir_registry(document: dict[str, Any], root: Path = ROOT) -> None:
    """Persist ControlIR node, alias, and algorithm records as individual JSON files."""
    root = root.resolve()
    index = dict(document)
    for section in ("control_nodes", "algorithms", "aliases"):
        records = document.get(section)
        if not isinstance(records, dict):
            raise RegistryFormatError(f"ControlIR document {section} must be an object")
        references: dict[str, str] = {}
        for key, record in sorted(records.items()):
            if not isinstance(key, str) or not key.strip():
                raise RegistryFormatError(f"invalid ControlIR key in {section}: {key!r}")
            if section == "algorithms":
                family = record.get("family", "unclassified") if isinstance(record, dict) else "unclassified"
                parts = [_slug(part) for part in str(family).split("/")]
                relative = "registry/control_ir/manifest/algorithms/" + "/".join(parts + [_slug(key) + ".json"])
                detail = record
            elif section == "aliases":
                relative = f"registry/control_ir/manifest/aliases/{_slug(key)}.json"
                detail = {"target": record}
            else:
                relative = f"registry/control_ir/manifest/control_nodes/{_slug(key)}.json"
                detail = record
            if not isinstance(detail, dict):
                raise RegistryFormatError(f"ControlIR entry {key!r} must be an object")
            _write(_under_root(root, relative, "ControlIR manifest path"), detail)
            if relative in references.values():
                raise RegistryFormatError(f"duplicate ControlIR manifest path: {relative}")
            references[key] = relative
        index[section] = references
    _write(root / "registry" / "control_ir" / "registry.json", index)
