"""Load UETD-owned, checksummed host tool package records.

This module validates registry data only. It does not download or execute any
package, and a ``scaffold_only`` record cannot be selected for a real build.
"""

from __future__ import annotations
import json
from pathlib import Path
from typing import Any
from uef_gen.uetd_adapter.registry import UETDRegistryError, expand_entry_list, load_registry_document

class ToolchainCatalogueError(ValueError):
    """Raised when UETD tool package metadata is absent, unsafe, or inconsistent."""

def _declared_path(root: Path, declaration: dict[str, Any], key: str) -> Path:
    """Resolve one API-declared path without allowing traversal outside UETD."""
    try:
        relative = Path(declaration[key])
    except (KeyError, TypeError) as exc:
        raise ToolchainCatalogueError(f"UETD toolchain_catalogue declaration is missing {key!r}") from exc
    if relative.is_absolute() or ".." in relative.parts:
        raise ToolchainCatalogueError("Tool package registry paths must stay inside UETD")
    resolved_root = root.resolve()
    resolved = (resolved_root / relative).resolve()
    if not resolved.is_relative_to(resolved_root):
        raise ToolchainCatalogueError("Tool package registry path resolves outside UETD")
    return resolved

def load_toolchain_catalogue(uetd_root: Path) -> dict[str, dict[str, Any]]:
    """Load exact package records keyed by stable ID from UETD's declared registry.

    TODO(toolchain-catalogue-versioning): define forward/backward compatibility
    rules before changing schema versions; keep board references and generated
    provenance tied to the exact registry schema and record revision.
    """
    root = uetd_root.expanduser().resolve()
    try:
        api = json.loads((root / "api.json").read_text(encoding="utf-8"))
        declaration = api["uhal_api"]["toolchain_catalogue"]
    except (OSError, json.JSONDecodeError, KeyError, TypeError) as exc:
        raise ToolchainCatalogueError(f"UETD API does not provide a readable toolchain_catalogue declaration: {exc}") from exc
    if not isinstance(declaration, dict):
        raise ToolchainCatalogueError("UETD toolchain_catalogue declaration must be an object")
    index_path = _declared_path(root, declaration, "path")
    record_schema_path = _declared_path(root, declaration, "schema_path")
    index_schema_path = _declared_path(root, declaration, "registry_schema_path")
    try:
        index = load_registry_document(root, index_path.relative_to(root).as_posix())
        expanded = expand_entry_list(root, index, "entries")
        record_schema = json.loads(record_schema_path.read_text(encoding="utf-8"))
        index_schema = json.loads(index_schema_path.read_text(encoding="utf-8"))
        import jsonschema
        jsonschema.validate(instance=index, schema=index_schema)
        records = expanded.get("entries")
        if not isinstance(records, list):
            raise ToolchainCatalogueError("UETD tool package index entries must expand to an array")
        for record in records:
            jsonschema.validate(instance=record, schema=record_schema)
    except UETDRegistryError as exc:
        raise ToolchainCatalogueError(str(exc)) from exc
    except ImportError as exc:
        raise ToolchainCatalogueError("Tool package validation requires jsonschema") from exc
    except jsonschema.SchemaError as exc:
        raise ToolchainCatalogueError(f"UETD tool package schema is invalid: {exc.message}") from exc
    except jsonschema.ValidationError as exc:
        location = ".".join(str(part) for part in exc.absolute_path) or "<root>"
        raise ToolchainCatalogueError(f"UETD tool package registry violates its schema at {location}: {exc.message}") from exc
    except (OSError, json.JSONDecodeError) as exc:
        raise ToolchainCatalogueError(f"Cannot read UETD tool package metadata: {exc}") from exc
    if expanded.get("schema_version") != declaration.get("schema_version"):
        raise ToolchainCatalogueError("UETD tool package registry schema version disagrees with api.json")
    by_id: dict[str, dict[str, Any]] = {}
    for record in records:
        identifier = record["id"].casefold()
        if identifier in by_id:
            raise ToolchainCatalogueError(f"Duplicate UETD tool package id: {record['id']}")
        by_id[identifier] = record
    for identifier, record in by_id.items():
        for dependency in record.get("dependencies", []):
            if dependency.casefold() not in by_id:
                raise ToolchainCatalogueError(f"UETD tool package {identifier} references missing dependency {dependency!r}")
    _validate_dependency_graph(by_id)
    return by_id

def _validate_dependency_graph(records: dict[str, dict[str, Any]]) -> None:
    """Reject dependency cycles before a bootstrap plan can recurse forever."""
    visiting: set[str] = set()
    visited: set[str] = set()
    def visit(identifier: str) -> None:
        if identifier in visiting:
            raise ToolchainCatalogueError(f"UETD tool package dependency cycle includes {identifier!r}")
        if identifier in visited:
            return
        visiting.add(identifier)
        for dependency in records[identifier].get("dependencies", []):
            visit(dependency.casefold())
        visiting.remove(identifier)
        visited.add(identifier)
    for identifier in records:
        visit(identifier)
