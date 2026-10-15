"""Read UETD-owned exact-part chip records and materialize resolver specs.

Records may progress from sourced identity facts to complete hardware profiles.
Physical generation stays gated on package pins, DMA, clocks, interrupts,
memory maps, startup/linker data, errata review, and a ready board record.
"""

from __future__ import annotations
import json
from pathlib import Path
from typing import Any
from uef_gen.chips.database import ChipDatabase
from uef_gen.chips.specs import ArchSpec, ChipSpec, DMASpec, FamilySpec, PeripheralInstanceSpec, PeripheralTypeSpec
from uef_gen.chips.target_catalogue import load_target_family_catalogue
from uef_gen.uetd_adapter.registry import UETDRegistryError, expand_entry_list, load_registry_document
from urllib.parse import urlparse

class ChipCatalogueError(ValueError):
    """Raised when UETD's declared chip catalogue is missing or invalid."""

def _read_json(path: Path, description: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ChipCatalogueError(f"Cannot read {description} at {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ChipCatalogueError(f"{description} must be a JSON object")
    return value

def _declared_path(root: Path, declaration: dict[str, Any], key: str) -> Path:
    try:
        relative = Path(declaration[key])
    except (KeyError, TypeError) as exc:
        raise ChipCatalogueError(f"UETD chip declaration is missing {key}") from exc
    if relative.is_absolute() or ".." in relative.parts:
        raise ChipCatalogueError("UETD chip catalogue path must stay inside the UETD checkout")
    resolved = (root / relative).resolve()
    if not resolved.is_relative_to(root):
        raise ChipCatalogueError("UETD chip catalogue resolves outside the UETD checkout")
    return resolved

def load_chip_catalogue(uetd_root: Path) -> dict[str, Any]:
    """Read and validate the chip catalogue path published in UETD's API."""
    root = uetd_root.expanduser().resolve()
    api = _read_json(root / "api.json", "UETD API manifest")
    try:
        declaration = api["uhal_api"]["chip_catalogue"]
    except (KeyError, TypeError) as exc:
        raise ChipCatalogueError("UETD API manifest does not declare a chip_catalogue") from exc
    if not isinstance(declaration, dict):
        raise ChipCatalogueError("UETD chip_catalogue declaration must be an object")
    catalogue_path = _declared_path(root, declaration, "path")
    try:
        catalogue_index = load_registry_document(root, catalogue_path.relative_to(root).as_posix())
        catalogue = expand_entry_list(root, catalogue_index, "entries")
    except UETDRegistryError as exc:
        raise ChipCatalogueError(str(exc)) from exc
    catalogue["records"] = catalogue.pop("entries")
    expected_schema = declaration.get("schema_version")
    if catalogue.get("schema_version") != expected_schema:
        raise ChipCatalogueError(f"UETD chip schema does not match API declaration ({expected_schema})")
    if catalogue.get("catalogue_kind") != "uetd_chip_records":
        raise ChipCatalogueError("UETD chip catalogue has an unexpected kind")
    if catalogue.get("canonical_owner") != "UETD":
        raise ChipCatalogueError("UETD must remain the canonical owner of chip records")
    policy = catalogue.get("record_policy")
    if not isinstance(policy, dict):
        raise ChipCatalogueError("UETD chip record_policy must be an object")
    if policy.get("records_are_exact_order_codes") is not True:
        raise ChipCatalogueError("Chip records must identify exact order codes")
    if policy.get("all_records_generation_ready") is not False:
        raise ChipCatalogueError("all_records_generation_ready must remain false; readiness is per record")
    blockers = policy.get("generation_blockers")
    if not isinstance(blockers, list) or not blockers or any(not isinstance(item, str) or not item.strip() for item in blockers):
        raise ChipCatalogueError("Chip record_policy must list concrete generation blockers")
    records = catalogue.get("records")
    if not isinstance(records, list):
        raise ChipCatalogueError("UETD chip records must be an array")
    schema_path = _declared_path(root, declaration, "schema_path")
    schema = _read_json(schema_path, "UETD chip record JSON Schema")
    registry_schema_path = _declared_path(root, declaration, "registry_schema_path")
    registry_schema = _read_json(registry_schema_path, "UETD chip registry JSON Schema")
    try:
        import jsonschema
    except ImportError as exc:
        raise ChipCatalogueError("Validating UETD chip records requires the declared jsonschema dependency") from exc
    try:
        jsonschema.validate(instance=catalogue_index, schema=registry_schema)
        for record in records:
            jsonschema.validate(instance=record, schema=schema)
    except jsonschema.SchemaError as exc:
        raise ChipCatalogueError(f"UETD chip JSON Schema is invalid: {exc.message}") from exc
    except jsonschema.ValidationError as exc:
        location = ".".join(str(part) for part in exc.absolute_path) or "<root>"
        raise ChipCatalogueError(f"UETD chip registry or record violates its schema at {location}: {exc.message}") from exc
    family_catalogue = load_target_family_catalogue(root)
    family_entries = {item["id"]: item for item in family_catalogue["entries"]}
    identifiers: set[str] = set()
    names: set[str] = set()
    for record in records:
        if not isinstance(record, dict):
            raise ChipCatalogueError("Each chip record must be an object")
        for field in ("id", "vendor", "family_id", "series", "part_number"):
            if not isinstance(record.get(field), str) or not record[field].strip():
                raise ChipCatalogueError(f"Chip record is missing a non-empty {field}")
        family = family_entries.get(record["family_id"])
        if family is None:
            raise ChipCatalogueError(f"Chip record {record['id']} references unknown family {record['family_id']}")
        identifier = record["id"].casefold()
        if identifier in identifiers:
            raise ChipCatalogueError(f"Duplicate chip id: {record['id']}")
        identifiers.add(identifier)
        aliases = record.get("aliases", [])
        if not isinstance(aliases, list):
            raise ChipCatalogueError(f"Chip record {record['id']} aliases must be an array")
        for name in (record["part_number"], *aliases):
            if not isinstance(name, str) or not name.strip():
                raise ChipCatalogueError(f"Chip record {record['id']} has an invalid alias")
            normalized = name.casefold()
            if normalized in names:
                raise ChipCatalogueError(f"Duplicate chip part name or alias: {name}")
            names.add(normalized)
        if not isinstance(record.get("generation_available"), bool):
            raise ChipCatalogueError(f"Chip record {record['id']} must declare generation_available as a boolean")
        target_kind = record.get("target_kind", "hardware")
        if target_kind not in {"hardware", "pipeline_fixture"}:
            raise ChipCatalogueError(f"Chip record {record['id']} has an invalid target_kind")
        if record["generation_available"] and target_kind == "hardware":
            resources = record.get("resources")
            required_resources = {
                "architecture",
                "peripheral_types",
                "peripherals",
                "dma",
                "clock_domains",
                "memory_regions",
                "startup",
                "linker_script",
                "errata_review",
            }
            if not isinstance(resources, dict) or not required_resources.issubset(resources):
                missing = sorted(required_resources - set(resources or {}))
                raise ChipCatalogueError(f"Generation-ready chip {record['id']} is missing resource sections: {', '.join(missing)}")
        phase = record.get("roadmap_phase")
        if phase is not None and (not isinstance(phase, int) or isinstance(phase, bool) or phase not in range(5)):
            raise ChipCatalogueError(f"Chip record {record['id']} has an invalid roadmap phase")
        core = record.get("core")
        package = record.get("package")
        memory = record.get("memory")
        if not isinstance(core, dict) or not isinstance(package, dict) or not isinstance(memory, dict):
            raise ChipCatalogueError(f"Chip record {record['id']} must define core, package and memory objects")
        if not isinstance(core.get("name"), str) or not core["name"].strip():
            raise ChipCatalogueError(f"Chip record {record['id']} has no core name")
        if not isinstance(core.get("count"), int) or isinstance(core["count"], bool) or core["count"] < 1:
            raise ChipCatalogueError(f"Chip record {record['id']} has an invalid core count")
        if not isinstance(core.get("fpu"), str) or not core["fpu"].strip():
            raise ChipCatalogueError(f"Chip record {record['id']} must state the FPU status")
        if not isinstance(core.get("max_frequency_hz"), int) or isinstance(core["max_frequency_hz"], bool) or core["max_frequency_hz"] <= 0:
            raise ChipCatalogueError(f"Chip record {record['id']} has no positive maximum core frequency")
        if not isinstance(package.get("name"), str) or not package["name"].strip():
            raise ChipCatalogueError(f"Chip record {record['id']} has no package name")
        if not isinstance(package.get("pin_count"), int) or isinstance(package["pin_count"], bool) or package["pin_count"] <= 0:
            raise ChipCatalogueError(f"Chip record {record['id']} has no positive package pin count")
        for memory_key in ("flash_bytes", "sram_bytes"):
            value = memory.get(memory_key)
            if not isinstance(value, int) or isinstance(value, bool) or value < 0:
                raise ChipCatalogueError(f"Chip record {record['id']} has invalid {memory_key}")
        peripheral_groups = record.get("peripheral_groups", [])
        if not isinstance(peripheral_groups, list) or any(not isinstance(item, str) or not item.strip() for item in peripheral_groups):
            raise ChipCatalogueError(f"Chip record {record['id']} has invalid peripheral_groups")
        sources = record.get("source_references")
        if not isinstance(sources, list) or (not sources and target_kind != "pipeline_fixture"):
            raise ChipCatalogueError(f"Chip record {record['id']} must cite at least one vendor source")
        for source in sources:
            if not isinstance(source, dict) or not isinstance(source.get("title"), str) or not source["title"].strip():
                raise ChipCatalogueError(f"Chip record {record['id']} has a malformed source reference")
            parsed = urlparse(str(source.get("url", "")))
            if parsed.scheme != "https" or not parsed.netloc:
                raise ChipCatalogueError(f"Chip record {record['id']} source URLs must use HTTPS")
        verified_fields = record.get("source_verified_fields")
        if not isinstance(verified_fields, list) or not verified_fields or any(not isinstance(item, str) or not item.strip() for item in verified_fields):
            raise ChipCatalogueError(f"Chip record {record['id']} must list the fields verified against sources")
    catalogue["records"] = records
    return catalogue

def load_chip_database(uetd_root: Path) -> ChipDatabase:
    """Materialize resolver records only from the located UETD chip registry."""
    catalogue = load_chip_catalogue(uetd_root)
    database = ChipDatabase()
    for record in catalogue["records"]:
        resources = record.get("resources", {})
        if not isinstance(resources, dict):
            resources = {}
        core = record["core"]
        architecture = resources.get("architecture", {})
        if not isinstance(architecture, dict):
            architecture = {}
        word_bits = architecture.get("word_bits", core.get("word_bits", 0))
        arch = ArchSpec(
            name=str(core["name"]),
            word_bits=int(word_bits),
            fpu=str(core.get("fpu", "none")),
            dsp_simd=bool(architecture.get("dsp_simd", False)),
            dcache=bool(architecture.get("dcache", False)),
            icache=bool(architecture.get("icache", False)),
            mpu=bool(architecture.get("mpu", False)),
            dwt=bool(architecture.get("dwt", False)),
            hw_divide=bool(architecture.get("hw_divide", False)),
            cmsis_dsp=bool(architecture.get("cmsis_dsp", False)),
            capabilities=frozenset(str(item) for item in architecture.get("capabilities", [])),
            compiler_flags=tuple(str(item) for item in architecture.get("compiler_flags", [])),
        )
        raw_dma = resources.get("dma", {})
        if not isinstance(raw_dma, dict):
            raw_dma = {}
        dma = DMASpec(
            controller_count=int(raw_dma.get("controller_count", 0)),
            channels_per_controller=int(raw_dma.get("channels_per_controller", 0)),
            muxed=bool(raw_dma.get("muxed", False)),
            request_map=dict(raw_dma.get("request_map", {})),
        )
        raw_types = resources.get("peripheral_types", {})
        if not isinstance(raw_types, dict):
            raw_types = {}
        peripheral_types = {
            str(name).casefold(): PeripheralTypeSpec(
                type_name=str(name),
                generic_signals=tuple(str(item) for item in spec.get("generic_signals", [])),
                dma_directions=tuple(str(item).upper() for item in spec.get("dma_directions", [])),
                type_caps=dict(spec.get("type_caps", {})),
            )
            for name, spec in raw_types.items()
            if isinstance(spec, dict)
        }
        raw_peripherals = resources.get("peripherals", {})
        if not isinstance(raw_peripherals, dict):
            raw_peripherals = {}
        peripherals: dict[str, PeripheralInstanceSpec] = {}
        for name, spec in raw_peripherals.items():
            if not isinstance(spec, dict):
                continue
            type_name = str(spec.get("type", ""))
            type_spec = peripheral_types.get(type_name.casefold(), PeripheralTypeSpec(type_name))
            pins = spec.get("pins", {})
            af = spec.get("af", {})
            peripheral_dma = spec.get("dma_requests", {})
            peripherals[str(name)] = PeripheralInstanceSpec(
                instance_name=str(name),
                type=type_spec,
                clock_domain=str(spec.get("clock_domain", "")),
                pins={str(signal): tuple(str(pin) for pin in choices) for signal, choices in pins.items()},
                af={str(route).upper(): int(number) for route, number in af.items()},
                dma_requests={str(direction).upper(): str(request) for direction, request in peripheral_dma.items()},
                irqs=tuple(str(item) for item in spec.get("irqs", [])),
                instance_caps=dict(spec.get("instance_caps", {})),
            )
        capability_overrides = resources.get("capabilities", {})
        if not isinstance(capability_overrides, dict):
            capability_overrides = {}
        memory_regions = resources.get("memory_regions", {})
        if not isinstance(memory_regions, dict):
            memory_regions = {}
        overrides = {str(key): value for key, value in capability_overrides.items()}
        overrides["memory_regions"] = {
            str(name): int(region.get("length_bytes", 0)) for name, region in memory_regions.items() if isinstance(region, dict)
        }
        family = FamilySpec(
            name=str(record["family_id"]),
            arch=arch,
            dma=dma,
            peripheral_types=tuple(peripheral_types.values()),
            hrtim=bool(capability_overrides.get("hrtim", False)),
            fdcan=bool(capability_overrides.get("fdcan", False)),
            cordic=bool(capability_overrides.get("cordic", False)),
            fmac=bool(capability_overrides.get("fmac", False)),
            opamp=bool(capability_overrides.get("opamp", False)),
            comp=bool(capability_overrides.get("comp", False)),
            usb_fs=bool(capability_overrides.get("usb_fs", False)),
            dma_mux=bool(capability_overrides.get("dma_mux", False)),
            clock_domains=dict(resources.get("clock_domains", {})),
        )
        chip = ChipSpec(
            name=str(record["part_number"]),
            family=family,
            package=str(record["package"]["name"]),
            pin_count=int(record["package"]["pin_count"]),
            flash_bytes=int(record["memory"]["flash_bytes"]),
            sram_bytes=int(record["memory"]["sram_bytes"]),
            peripherals=peripherals,
            overrides=overrides,
            verified=bool(record["generation_available"] and record.get("target_kind") != "pipeline_fixture"),
            source_note=str(record.get("record_note", "")),
            registry_id=str(record["id"]),
            generation_available=bool(record["generation_available"]),
            target_kind=str(record.get("target_kind", "hardware")),
            memory_regions=dict(memory_regions),
            resources=resources,
        )
        database.register(chip, tuple(str(alias) for alias in record.get("aliases", [])))
    return database

def find_chip_record(catalogue: dict[str, Any], name: str) -> dict[str, Any] | None:
    """Find a part by canonical id, full order code, or documented alias."""
    needle = name.casefold()
    for record in catalogue["records"]:
        names = (record["id"], record["part_number"], *record.get("aliases", []))
        if any(value.casefold() == needle for value in names):
            return record
    return None
