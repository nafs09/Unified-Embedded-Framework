"""Read UEF-owned exact-part chip-base records without making them targets.

A base record captures sourced identity and headline device facts. It is not a
complete ChipSpec: resource resolution remains gated on package pin, DMA, clock,
interrupt, memory-map, startup, and board data.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any
from urllib.parse import urlparse
from uef_gen.chips.target_catalogue import load_target_family_catalogue
from uef_gen.registry import UEFRegistryError, expand_entry_list, load_registry_document


class ChipBaseCatalogueError(ValueError):
    """Raised when UEF's declared chip-base catalogue is missing or invalid."""


def _read_json(path: Path, description: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ChipBaseCatalogueError(f"Cannot read {description} at {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ChipBaseCatalogueError(f"{description} must be a JSON object")
    return value


def _declared_path(root: Path, declaration: dict[str, Any], key: str) -> Path:
    try:
        relative = Path(declaration[key])
    except (KeyError, TypeError) as exc:
        raise ChipBaseCatalogueError(f"UEF chip-base declaration is missing {key}") from exc
    if relative.is_absolute() or ".." in relative.parts:
        raise ChipBaseCatalogueError(
            "UEF chip-base catalogue path must stay inside the UEF checkout"
        )
    resolved = (root / relative).resolve()
    if not resolved.is_relative_to(root):
        raise ChipBaseCatalogueError("UEF chip-base catalogue resolves outside the UEF checkout")
    return resolved


def load_chip_base_catalogue(uef_root: Path) -> dict[str, Any]:
    """Read and validate the chip-base catalogue path published in UEF's API."""
    root = uef_root.expanduser().resolve()
    api = _read_json(root / "uef_api.json", "UEF API manifest")
    try:
        declaration = api["uhal_api"]["chip_base_catalogue"]
    except (KeyError, TypeError) as exc:
        raise ChipBaseCatalogueError(
            "UEF API manifest does not declare a chip_base_catalogue"
        ) from exc
    if not isinstance(declaration, dict):
        raise ChipBaseCatalogueError("UEF chip_base_catalogue declaration must be an object")

    catalogue_path = _declared_path(root, declaration, "path")
    try:
        catalogue_index = load_registry_document(root, catalogue_path.relative_to(root).as_posix())
        catalogue = expand_entry_list(root, catalogue_index, "entries")
    except UEFRegistryError as exc:
        raise ChipBaseCatalogueError(str(exc)) from exc
    catalogue["records"] = catalogue.pop("entries")
    expected_schema = declaration.get("schema_version")
    if catalogue.get("schema_version") != expected_schema:
        raise ChipBaseCatalogueError(
            f"UEF chip-base schema does not match API declaration ({expected_schema})"
        )
    if catalogue.get("catalogue_kind") != "uef_chip_base_records":
        raise ChipBaseCatalogueError("UEF chip-base catalogue has an unexpected kind")
    if catalogue.get("canonical_owner") != "UEF":
        raise ChipBaseCatalogueError("UEF must remain the canonical owner of chip-base records")

    policy = catalogue.get("record_policy")
    if not isinstance(policy, dict):
        raise ChipBaseCatalogueError("UEF chip-base record_policy must be an object")
    if policy.get("records_are_exact_order_codes") is not True:
        raise ChipBaseCatalogueError("Chip-base records must identify exact order codes")
    if policy.get("generation_available_for_all_records") is not False:
        raise ChipBaseCatalogueError("Chip-base records must remain non-generatable")
    blockers = policy.get("generation_blockers")
    if not isinstance(blockers, list) or not blockers or any(
        not isinstance(item, str) or not item.strip() for item in blockers
    ):
        raise ChipBaseCatalogueError(
            "Chip-base record_policy must list concrete generation blockers"
        )

    records = catalogue.get("records")
    if not isinstance(records, list):
        raise ChipBaseCatalogueError("UEF chip-base records must be an array")

    if declaration.get("generation_ready") is not False:
        raise ChipBaseCatalogueError("Chip-base catalogue declaration must remain non-generatable")
    schema_path = _declared_path(root, declaration, "schema_path")
    schema = _read_json(schema_path, "UEF chip-base record JSON Schema")
    registry_schema_path = _declared_path(root, declaration, "registry_schema_path")
    registry_schema = _read_json(registry_schema_path, "UEF chip-base registry JSON Schema")
    try:
        import jsonschema
    except ImportError as exc:
        raise ChipBaseCatalogueError(
            "Validating UEF chip-base records requires the declared jsonschema dependency"
        ) from exc
    try:
        jsonschema.validate(instance=catalogue_index, schema=registry_schema)
        for record in records:
            jsonschema.validate(instance=record, schema=schema)
    except jsonschema.SchemaError as exc:
        raise ChipBaseCatalogueError(
            f"UEF chip-base JSON Schema is invalid: {exc.message}"
        ) from exc
    except jsonschema.ValidationError as exc:
        location = ".".join(str(part) for part in exc.absolute_path) or "<root>"
        raise ChipBaseCatalogueError(
            f"UEF chip-base registry or record violates its schema at {location}: {exc.message}"
        ) from exc

    family_catalogue = load_target_family_catalogue(root)
    family_entries = {item["id"]: item for item in family_catalogue["entries"]}

    identifiers: set[str] = set()
    names: set[str] = set()
    for record in records:
        if not isinstance(record, dict):
            raise ChipBaseCatalogueError("Each chip-base record must be an object")
        for field in ("id", "vendor", "family_id", "series", "part_number"):
            if not isinstance(record.get(field), str) or not record[field].strip():
                raise ChipBaseCatalogueError(
                    f"Chip-base record is missing a non-empty {field}"
                )
        family = family_entries.get(record["family_id"])
        if family is None:
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} references unknown family {record['family_id']}"
            )

        identifier = record["id"].casefold()
        if identifier in identifiers:
            raise ChipBaseCatalogueError(f"Duplicate chip-base id: {record['id']}")
        identifiers.add(identifier)
        aliases = record.get("aliases", [])
        if not isinstance(aliases, list):
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} aliases must be an array"
            )
        for name in (record["part_number"], *aliases):
            if not isinstance(name, str) or not name.strip():
                raise ChipBaseCatalogueError(
                    f"Chip-base record {record['id']} has an invalid alias"
                )
            normalized = name.casefold()
            if normalized in names:
                raise ChipBaseCatalogueError(f"Duplicate chip-base part name or alias: {name}")
            names.add(normalized)

        if record.get("generation_available") is not False:
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} must remain non-generatable"
            )
        phase = record.get("roadmap_phase")
        if phase is not None and (
            not isinstance(phase, int)
            or isinstance(phase, bool)
            or phase not in range(5)
        ):
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} has an invalid roadmap phase"
            )
        core = record.get("core")
        package = record.get("package")
        memory = record.get("memory")
        if (
            not isinstance(core, dict)
            or not isinstance(package, dict)
            or not isinstance(memory, dict)
        ):
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} must define core, package and memory objects"
            )
        if not isinstance(core.get("name"), str) or not core["name"].strip():
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} has no core name"
            )
        if (
            not isinstance(core.get("count"), int)
            or isinstance(core["count"], bool)
            or core["count"] < 1
        ):
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} has an invalid core count"
            )
        if not isinstance(core.get("fpu"), str) or not core["fpu"].strip():
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} must state the FPU status"
            )
        if (
            not isinstance(core.get("max_frequency_hz"), int)
            or isinstance(core["max_frequency_hz"], bool)
            or core["max_frequency_hz"] <= 0
        ):
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} has no positive maximum core frequency"
            )
        if not isinstance(package.get("name"), str) or not package["name"].strip():
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} has no package name"
            )
        if (
            not isinstance(package.get("pin_count"), int)
            or isinstance(package["pin_count"], bool)
            or package["pin_count"] <= 0
        ):
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} has no positive package pin count"
            )
        for memory_key in ("flash_bytes", "sram_bytes"):
            value = memory.get(memory_key)
            if not isinstance(value, int) or isinstance(value, bool) or value < 0:
                raise ChipBaseCatalogueError(
                    f"Chip-base record {record['id']} has invalid {memory_key}"
                )

        peripheral_groups = record.get("peripheral_groups", [])
        if not isinstance(peripheral_groups, list) or any(
            not isinstance(item, str) or not item.strip() for item in peripheral_groups
        ):
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} has invalid peripheral_groups"
            )

        sources = record.get("source_references")
        if not isinstance(sources, list) or not sources:
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} must cite at least one vendor source"
            )
        for source in sources:
            if (
                not isinstance(source, dict)
                or not isinstance(source.get("title"), str)
                or not source["title"].strip()
            ):
                raise ChipBaseCatalogueError(
                    f"Chip-base record {record['id']} has a malformed source reference"
                )
            parsed = urlparse(str(source.get("url", "")))
            if parsed.scheme != "https" or not parsed.netloc:
                raise ChipBaseCatalogueError(
                    f"Chip-base record {record['id']} source URLs must use HTTPS"
                )

        verified_fields = record.get("source_verified_fields")
        if not isinstance(verified_fields, list) or not verified_fields or any(
            not isinstance(item, str) or not item.strip() for item in verified_fields
        ):
            raise ChipBaseCatalogueError(
                f"Chip-base record {record['id']} must list the fields verified against sources"
            )


    catalogue["records"] = records
    return catalogue


def find_chip_base_record(
    catalogue: dict[str, Any], name: str
) -> dict[str, Any] | None:
    """Find a part by canonical id, full order code, or documented alias."""
    needle = name.casefold()
    for record in catalogue["records"]:
        names = (record["id"], record["part_number"], *record.get("aliases", []))
        if any(value.casefold() == needle for value in names):
            return record
    return None
