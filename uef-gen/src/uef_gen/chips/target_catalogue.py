"""Load the target-family roadmap from the located UEF checkout.

Family records are planning metadata. They are never ChipSpecs and cannot
participate in resource resolution or project generation.
"""
from __future__ import annotations
import json
from pathlib import Path
from typing import Any
from uef_gen.registry import UEFRegistryError, expand_entry_list, load_registry_document

class TargetCatalogueError(ValueError):
    """Raised when UEF's published target-family catalogue is missing or invalid."""

def load_target_family_catalogue(uef_root: Path) -> dict[str, Any]:
    """Read and validate the family catalogue declared by UEF's API manifest."""
    root = uef_root.expanduser().resolve()
    api_path = root / "uef_api.json"
    if not api_path.is_file():
        raise TargetCatalogueError(f"UEF API manifest is missing: {api_path}")
    try:
        api = json.loads(api_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise TargetCatalogueError(
            f"Cannot read UEF API manifest {api_path}: {exc}"
        ) from exc
    if not isinstance(api, dict):
        raise TargetCatalogueError("UEF API manifest must be a JSON object")

    try:
        declaration = api["uhal_api"]["target_profiles"]["target_family_catalogue"]
        relative = Path(declaration["path"])
        expected_schema = declaration["schema_version"]
        registry_schema_relative = Path(declaration["registry_schema_path"])
    except (KeyError, TypeError) as exc:
        raise TargetCatalogueError(
            "UEF API manifest does not declare a complete target_family_catalogue"
        ) from exc
    if not isinstance(declaration, dict):
        raise TargetCatalogueError(
            "UEF target_family_catalogue declaration must be an object"
        )
    if relative.is_absolute() or ".." in relative.parts:
        raise TargetCatalogueError(
            "UEF target catalogue path must stay inside the UEF checkout"
        )
    catalogue_path = (root / relative).resolve()
    if not catalogue_path.is_relative_to(root):
        raise TargetCatalogueError(
            "UEF target catalogue resolves outside the UEF checkout"
        )
    if registry_schema_relative.is_absolute() or ".." in registry_schema_relative.parts:
        raise TargetCatalogueError("UEF target registry schema path must stay inside the checkout")
    registry_schema_path = (root / registry_schema_relative).resolve()
    if not registry_schema_path.is_relative_to(root):
        raise TargetCatalogueError("UEF target registry schema resolves outside the checkout")
    try:
        catalogue_index = load_registry_document(root, relative)
        catalogue = expand_entry_list(root, catalogue_index, "entries")
        schema = json.loads(registry_schema_path.read_text(encoding="utf-8"))
        import jsonschema
        jsonschema.validate(instance=catalogue_index, schema=schema)
    except UEFRegistryError as exc:
        raise TargetCatalogueError(str(exc)) from exc
    except (OSError, json.JSONDecodeError) as exc:
        raise TargetCatalogueError(f"Cannot read target registry schema {registry_schema_path}: {exc}") from exc
    except ImportError as exc:
        raise TargetCatalogueError("Validating the UEF target registry requires jsonschema") from exc
    except jsonschema.SchemaError as exc:
        raise TargetCatalogueError(f"UEF target registry schema is invalid: {exc.message}") from exc
    except jsonschema.ValidationError as exc:
        location = ".".join(str(part) for part in exc.absolute_path) or "<root>"
        raise TargetCatalogueError(f"UEF target registry violates its schema at {location}: {exc.message}") from exc

    if not isinstance(catalogue, dict):
        raise TargetCatalogueError(
            "UEF target-family catalogue must be a JSON object"
        )
    if catalogue.get("schema_version") != expected_schema:
        raise TargetCatalogueError(
            f"UEF target catalogue schema does not match API declaration ({expected_schema})"
        )
    if catalogue.get("catalogue_kind") != "uef_target_family_roadmap":
        raise TargetCatalogueError(
            "UEF target catalogue has an unexpected catalogue_kind"
        )
    if catalogue.get("canonical_owner") != "UEF":
        raise TargetCatalogueError(
            "UEF must remain the canonical owner of target-family records"
        )
    entries = catalogue.get("entries")
    if not isinstance(entries, list):
        raise TargetCatalogueError("UEF target catalogue entries must be an array")

    source_references = catalogue.get("source_references")
    if not isinstance(source_references, dict):
        raise TargetCatalogueError(
            "UEF target catalogue source_references must be an object"
        )
    identifiers: set[str] = set()
    for entry in entries:
        if (
            not isinstance(entry, dict)
            or not isinstance(entry.get("id"), str)
            or not entry["id"].strip()
        ):
            raise TargetCatalogueError(
                "Each target catalogue entry must have a non-empty string id"
            )
        identifier = entry["id"].casefold()
        if identifier in identifiers:
            raise TargetCatalogueError(
                f"Duplicate target catalogue id: {entry['id']}"
            )
        identifiers.add(identifier)
        if entry.get("generation_available") is not False:
            raise TargetCatalogueError(
                f"Family catalogue entry {entry['id']} must not be generation-available"
            )
        if entry.get("verified_hardware_data") is not False:
            raise TargetCatalogueError(
                f"Family catalogue entry {entry['id']} must not claim verified hardware data"
            )
        if entry.get("representative_parts_are_examples_only") is not True:
            raise TargetCatalogueError(
                f"Family catalogue entry {entry['id']} must mark representative parts as examples"
            )
        phase = entry.get("roadmap_phase")
        if phase is not None and (
            not isinstance(phase, int)
            or isinstance(phase, bool)
            or phase not in range(5)
        ):
            raise TargetCatalogueError(
                f"Family catalogue entry {entry['id']} has an invalid roadmap phase"
            )
        references = entry.get("source_references")
        if not isinstance(references, list) or not references or any(
            not isinstance(reference, str) or reference not in source_references
            for reference in references
        ):
            raise TargetCatalogueError(
                f"Family catalogue entry {entry['id']} has missing or unknown source references"
            )

    catalogue["entries"] = entries
    return catalogue
