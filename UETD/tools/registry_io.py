"""Read and write UETD's split JSON registries for repository maintenance tools."""

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
        raise RegistryFormatError(f"{label} must stay inside the UETD checkout")
    candidate = (root / path).resolve()
    if not candidate.is_relative_to(root.resolve()):
        raise RegistryFormatError(f"{label} resolves outside the UETD checkout")
    return candidate

def _read_index(root: Path, relative: str) -> dict[str, Any]:
    path = _under_root(root, relative, "registry index path")
    return _read(path)

def load_module_registry(root: Path = ROOT) -> dict[str, Any]:
    """Return the combined module document shape from index and detail files."""
    root = root.resolve()
    api = _read(root / "api.json")
    registry_path = api.get("module_registry")
    if not isinstance(registry_path, str) or not registry_path.strip():
        raise RegistryFormatError("api.json must declare module_registry")
    document = _read_index(root, registry_path)
    references = document.get("modules")
    if not isinstance(references, dict):
        raise RegistryFormatError("module registry must contain a modules object")
    if references and all(isinstance(value, dict) and "manifest" not in value and "path" not in value for value in references.values()):
        return document
    modules: dict[str, Any] = {}
    seen_descriptors: dict[str, str] = {}
    for key, reference in references.items():
        if not isinstance(key, str):
            raise RegistryFormatError("module registry entry names must be strings")
        if isinstance(reference, dict):
            relative = reference.get("manifest", reference.get("path"))
            descriptor = reference.get("DSL_descriptor")
            if not isinstance(descriptor, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", descriptor):
                raise RegistryFormatError(f"module registry entry {key!r} requires an identifier-shaped DSL_descriptor")
        else:
            raise RegistryFormatError(f"module registry entry {key!r} must include a manifest and DSL_descriptor")
        detail = _read(_under_root(root, relative, f"module manifest {key}"))
        if descriptor in seen_descriptors:
            raise RegistryFormatError(f"DSL_descriptor {descriptor!r} is assigned to both {seen_descriptors[descriptor]!r} and {key!r}")
        seen_descriptors[descriptor] = key
        if "DSL_descriptor" in detail:
            raise RegistryFormatError(f"module {key!r} duplicates DSL_descriptor in its manifest and registry index")
        detail["DSL_descriptor"] = descriptor
        modules[key] = detail
    result = dict(document)
    result["modules"] = modules
    return result

def write_module_registry(document: dict[str, Any], root: Path = ROOT) -> None:
    """Persist module records as one JSON file per module plus the small index."""
    root = root.resolve()
    modules = document.get("modules")
    if not isinstance(modules, dict):
        raise RegistryFormatError("module document must contain a modules object")
    seen_descriptors: dict[str, str] = {}
    for key, record in modules.items():
        if not isinstance(key, str) or not isinstance(record, dict):
            raise RegistryFormatError("module names and records must be strings and objects")
        if key.startswith("/") or "\\" in key or any(part in {"", ".", ".."} for part in key.split("/")):
            raise RegistryFormatError(f"unsafe module name: {key!r}")
        descriptor = record.get("DSL_descriptor")
        if not isinstance(descriptor, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", descriptor):
            raise RegistryFormatError(f"module {key!r} requires an identifier-shaped DSL_descriptor")
        if descriptor in seen_descriptors:
            raise RegistryFormatError(f"DSL_descriptor {descriptor!r} is assigned to both {seen_descriptors[descriptor]!r} and {key!r}")
        seen_descriptors[descriptor] = key
    index = dict(document)
    references: dict[str, Any] = {}
    for key, record in sorted(modules.items()):
        if not isinstance(key, str) or not isinstance(record, dict):
            raise RegistryFormatError("module names and records must be strings and objects")
        if key.startswith("/") or "\\" in key or any(part in {"", ".", ".."} for part in key.split("/")):
            raise RegistryFormatError(f"unsafe module name: {key!r}")
        relative = f"registry/modules/manifest/{key}.json"
        detail = dict(record)
        descriptor = detail.pop("DSL_descriptor", None)
        if isinstance(descriptor, str) and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", descriptor):
            references[key] = {"manifest": relative, "DSL_descriptor": descriptor}
        else:
            raise RegistryFormatError(f"module {key!r} has an invalid DSL_descriptor")
        _write(_under_root(root, relative, "module manifest path"), detail)
    index["modules"] = references
    _write(root / "registry" / "modules" / "registry.json", index)
