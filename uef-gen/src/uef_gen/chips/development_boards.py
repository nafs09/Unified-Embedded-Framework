"""Read UETD-owned development-board bindings."""

from __future__ import annotations
import json
from pathlib import Path
from typing import Any
from uef_gen.chips.registry_records import load_chip_catalogue
from uef_gen.uetd_adapter.registry import UETDRegistryError, expand_entry_list, load_registry_document
from urllib.parse import urlparse

class DevelopmentBoardCatalogueError(ValueError):
    """Raised when UETD's development-board registry is missing or invalid."""

def _declared_path(root: Path, declaration: dict[str, Any], key: str) -> Path:
    try:
        relative = Path(declaration[key])
    except (KeyError, TypeError) as exc:
        raise DevelopmentBoardCatalogueError(f"UETD development_boards declaration is missing {key}") from exc
    if relative.is_absolute() or ".." in relative.parts:
        raise DevelopmentBoardCatalogueError("Development-board registry paths must stay inside UETD")
    resolved = (root / relative).resolve()
    if not resolved.is_relative_to(root):
        raise DevelopmentBoardCatalogueError("Development-board registry path escapes UETD")
    return resolved

def load_development_boards(uetd_root: Path) -> dict[str, dict[str, Any]]:
    """Load board records keyed by canonical ID and aliases from UETD's API-declared index."""
    root = uetd_root.expanduser().resolve()
    try:
        api = json.loads((root / "api.json").read_text(encoding="utf-8"))
        declaration = api["uhal_api"]["development_boards"]
    except (OSError, json.JSONDecodeError, KeyError, TypeError) as exc:
        raise DevelopmentBoardCatalogueError(f"UETD API does not provide a readable development_boards declaration: {exc}") from exc
    if not isinstance(declaration, dict):
        raise DevelopmentBoardCatalogueError("UETD development_boards declaration must be an object")
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
        for record in expanded["entries"]:
            jsonschema.validate(instance=record, schema=record_schema)
    except UETDRegistryError as exc:
        raise DevelopmentBoardCatalogueError(str(exc)) from exc
    except ImportError as exc:
        raise DevelopmentBoardCatalogueError("Development-board validation requires jsonschema") from exc
    except jsonschema.SchemaError as exc:
        raise DevelopmentBoardCatalogueError(f"UETD development-board schema is invalid: {exc.message}") from exc
    except jsonschema.ValidationError as exc:
        location = ".".join(str(part) for part in exc.absolute_path) or "<root>"
        raise DevelopmentBoardCatalogueError(f"UETD development-board registry violates its schema at {location}: {exc.message}") from exc
    except (OSError, json.JSONDecodeError) as exc:
        raise DevelopmentBoardCatalogueError(f"Cannot read UETD development-board metadata: {exc}") from exc
    if expanded.get("schema_version") != declaration.get("schema_version"):
        raise DevelopmentBoardCatalogueError("UETD development-board schema version disagrees with api.json")
    chips = load_chip_catalogue(root)
    chip_ids = {item["id"] for item in chips["records"]}
    boards: dict[str, dict[str, Any]] = {}
    for record in expanded["entries"]:
        if record["chip_id"] not in chip_ids:
            raise DevelopmentBoardCatalogueError(f"Development board {record['id']} references unknown chip id {record['chip_id']}")
        for name in (record["id"], *record.get("aliases", [])):
            key = str(name).casefold()
            if key in boards:
                raise DevelopmentBoardCatalogueError(f"Duplicate development-board name or alias: {name}")
            boards[key] = record
        if record.get("generation_available"):
            for section in ("build", "flash"):
                if not isinstance(record.get(section), dict):
                    raise DevelopmentBoardCatalogueError(f"Generation-ready development board {record['id']} needs a {section} profile")
        for reference in record.get("source_references", []):
            parsed = urlparse(str(reference.get("url", "")))
            if parsed.scheme != "https" or not parsed.netloc:
                raise DevelopmentBoardCatalogueError(f"Development board {record['id']} source URLs must use HTTPS")
    return boards
