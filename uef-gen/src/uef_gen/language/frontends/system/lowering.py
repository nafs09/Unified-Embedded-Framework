"""Lower validated `.uefs` declarations into deterministic build plans."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from uef_gen.language.frontends.project.compiler import CompilationResult
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic, source_error
from uef_gen.language.frontends.system.ast import SystemDocument


@dataclass(frozen=True)
class SystemNodeBuild:
    """One independently compiled target project in a system build plan."""

    name: str
    project_path: Path
    target_id: str
    application: Any
    hardware_configuration: dict[str, Any]
    source_span: Any = None


@dataclass(frozen=True)
class SystemLinkBuild:
    """Resolved typed transport link between declared endpoints."""

    name: str
    source_endpoint: str
    destination_endpoint: str
    protocol_name: str | None
    contract_name: str | None
    contract: dict[str, Any]
    source_span: Any = None


@dataclass(frozen=True)
class SystemBuildPlan:
    """Aggregate plan; each node retains its own generated application input."""

    system_name: str
    source_path: Path | None
    nodes: tuple[SystemNodeBuild, ...]
    links: tuple[SystemLinkBuild, ...]
    external_endpoints: tuple[dict[str, Any], ...]
    contracts: dict[str, Any] = field(default_factory=dict)
    source_map: dict[str, Any] = field(default_factory=dict)


def lower_system_document(
    document: SystemDocument,
    node_results: dict[str, CompilationResult],
    *,
    source_path: Path | None,
) -> tuple[SystemBuildPlan | None, list[LanguageDiagnostic]]:
    """Build a complete system plan; reject missing or failed node results."""
    diagnostics: list[LanguageDiagnostic] = []
    nodes: list[SystemNodeBuild] = []
    root = source_path.parent if source_path else Path.cwd()
    for node in document.nodes:
        compiled = node_results.get(node.name)
        if compiled is None:
            diagnostics.append(source_error("system_node_result_missing", f"Node {node.name!r} has no standalone compilation result.", node.source_span))
            continue
        if not compiled.ok or compiled.application_spec is None or compiled.hardware_configuration is None:
            diagnostics.append(source_error("system_node_result_invalid", f"Node {node.name!r} did not produce a valid application and hardware configuration.", node.source_span))
            continue
        nodes.append(SystemNodeBuild(
            node.name,
            (root / node.project_path).resolve(strict=False),
            compiled.application_spec.target_id,
            compiled.application_spec,
            compiled.hardware_configuration,
            node.source_span,
        ))
    if len(nodes) != len(document.nodes):
        return None, diagnostics

    records = {item.name: _record_dict(item) for item in document.records}
    enums = {item.name: _enum_dict(item) for item in document.enums}
    protocols = {item.name: _protocol_dict(item) for item in document.protocols}
    links: list[SystemLinkBuild] = []
    for item in document.links:
        contract: dict[str, Any] = {
            "properties": {name: _value(value) for name, value in item.properties.items()},
        }
        if item.contract_name:
            contract["record"] = records.get(item.contract_name)
        if item.protocol_name:
            contract["protocol"] = protocols.get(item.protocol_name)
        links.append(SystemLinkBuild(item.name, item.source_endpoint, item.destination_endpoint,
                                     item.protocol_name, item.contract_name, contract, item.source_span))

    endpoints = tuple({
        "name": item.name,
        "implementation": item.implementation,
        "role": item.role,
        "attached_to": item.attached_to,
        "properties": {name: _value(value) for name, value in item.properties.items()},
        "source_span": item.source_span,
    } for item in document.external_endpoints)
    contracts = {
        "records": records,
        "enums": enums,
        "variants": {item.name: _variant_dict(item) for item in document.variants},
        "flags": {item.name: _flags_dict(item) for item in document.flags_types},
        "fixed_point_types": {item.name: _fixed_dict(item) for item in document.fixed_point_types},
        "protocols": protocols,
        "hybrid_systems": [{"name": item.name, "properties": {key: _value(value) for key, value in item.properties.items()}, "source_span": item.source_span} for item in document.hybrid_systems],
        "delivery_policies": [{"name": item.get("name"), "properties": {key: _value(value) for key, value in item.get("properties", {}).items()}, "source_span": item.get("source_span")} for item in document.delivery_policies],
    }
    source_map: dict[str, Any] = {}
    for item in document.nodes:
        source_map[f"nodes.{item.name}"] = item.source_span
    for item in document.links:
        source_map[f"links.{item.name}"] = item.source_span
    for name, item in records.items():
        source_map[f"contracts.records.{name}"] = item["source_span"]
    for item in document.protocols:
        source_map[f"contracts.protocols.{item.name}"] = item.source_span
    return SystemBuildPlan(document.name, source_path, tuple(nodes), tuple(links), endpoints, contracts, source_map), diagnostics


def lower_imported_contract_view(
    document: SystemDocument,
    *,
    importing_project: Path,
    system_path: Path,
) -> tuple[dict[str, Any] | None, list[LanguageDiagnostic]]:
    """Select the shared facts and local links visible to one node project."""
    diagnostics: list[LanguageDiagnostic] = []
    system_file = system_path.expanduser().resolve(strict=False)
    root = system_file.parent
    importer = importing_project.expanduser().resolve(strict=False)
    matches = []
    for node in document.nodes:
        candidate = (root / node.project_path).resolve(strict=False)
        if candidate == importer:
            matches.append(node)
    if len(matches) != 1:
        return None, [source_error("system_import_node_match", "The importing project must match exactly one node in the .uefs contract.", document.source_span)]
    local_node = matches[0]
    local_endpoints = [item for item in document.external_endpoints if item.attached_to and item.attached_to.split(".", 1)[0] == local_node.name]
    local_links = [item for item in document.links if _endpoint_owner(item.source_endpoint) == local_node.name
                   or _endpoint_owner(item.destination_endpoint) == local_node.name
                   or item.source_endpoint in {endpoint.name for endpoint in local_endpoints}
                   or item.destination_endpoint in {endpoint.name for endpoint in local_endpoints}]
    try:
        from uef_gen.language.frontends.project.contract_imports import system_document_digest
        text = system_file.read_text(encoding="utf-8-sig")
        digest = system_document_digest(document, text)
    except (OSError, UnicodeError, ValueError) as exc:
        return None, [source_error("system_import_digest", f"Cannot establish imported contract identity: {exc}", document.source_span)]
    return {
        "system_name": document.name,
        "system_path": str(system_file),
        "content_digest": digest,
        "local_node": local_node.name,
        "records": {item.name: _record_dict(item) for item in document.records},
        "enums": {item.name: _enum_dict(item) for item in document.enums},
        "variants": {item.name: _variant_dict(item) for item in document.variants},
        "flags": {item.name: _flags_dict(item) for item in document.flags_types},
        "fixed_point_types": {item.name: _fixed_dict(item) for item in document.fixed_point_types},
        "protocols": {item.name: _protocol_dict(item) for item in document.protocols},
        "local_endpoints": [{"name": item.name, "implementation": item.implementation, "role": item.role,
                             "attached_to": item.attached_to, "properties": {key: _value(value) for key, value in item.properties.items()},
                             "source_span": item.source_span} for item in local_endpoints],
        "local_links": [_link_dict(item) for item in local_links],
    }, diagnostics


def _record_dict(record: Any) -> dict[str, Any]:
    return {"name": record.name, "fields": [{"name": name, "type": _type_dict(type_spec),
                                               "properties": {key: value for key, value in props.items() if key != "source_span"}}
                                              for name, type_spec, props in record.fields],
            "boundary": _boundary_dict(record.boundary), "source_span": record.source_span}


def _enum_dict(enum: Any) -> dict[str, Any]:
    return {"name": enum.name, "storage_type": enum.storage_type,
            "cases": [{"name": item.name, "value": item.value} for item in enum.variants], "source_span": enum.span}


def _variant_dict(item: Any) -> dict[str, Any]:
    return {"name": item.name, "tag_type": item.tag_type,
            "cases": [{"name": case.name, "tag": case.tag, "fields": [{"name": field.name, "type": _type_dict(field.type_spec)} for field in case.fields]} for case in item.cases],
            "source_span": item.span}


def _flags_dict(item: Any) -> dict[str, Any]:
    return {"name": item.name, "storage_type": item.storage_type, "members": [{"name": member.name, "bit": member.bit} for member in item.members], "source_span": item.span}


def _fixed_dict(item: Any) -> dict[str, Any]:
    return {"name": item.name, "storage_type": item.storage_type, "fractional_bits": item.fractional_bits,
            "unit": item.unit, "rounding": item.rounding, "overflow": item.overflow, "source_span": item.span}


def _protocol_dict(item: Any) -> dict[str, Any]:
    return {"name": item.name, "transport": item.transport, "framing": _value(item.framing), "header": _value(item.header),
            "sequence": _value(item.sequence), "checksum": _value(item.checksum), "messages": _value(item.messages),
            "receiver": _value(item.receiver), "properties": _value(item.properties), "source_span": item.source_span}


def _link_dict(item: Any) -> dict[str, Any]:
    return {"name": item.name, "from": item.source_endpoint, "to": item.destination_endpoint,
            "contract": item.contract_name, "protocol": item.protocol_name,
            "properties": {key: _value(value) for key, value in item.properties.items()}, "source_span": item.source_span}


def _type_dict(type_spec: Any) -> dict[str, Any] | None:
    if type_spec is None:
        return None
    return {"name": type_spec.name, "shape": list(type_spec.shape), "unit": type_spec.unit,
            "arguments": [_type_dict(item) for item in type_spec.type_arguments]}


def _boundary_dict(boundary: Any) -> dict[str, Any] | None:
    if boundary is None:
        return None
    return {key: getattr(boundary, key) for key in ("kind", "version", "schema_id", "alignment", "field_offsets", "field_encodings", "atomic_fields", "producer", "consumer", "memory_order", "memory_barriers", "cache_policy", "publication_sequence_field", "integrity", "migration_policy")}


def _endpoint_owner(endpoint: str) -> str | None:
    return endpoint.split(".", 1)[0] if "." in endpoint else None


def _value(value: Any) -> Any:
    from dataclasses import fields, is_dataclass
    from enum import Enum
    from pathlib import Path
    if value is None or isinstance(value, (str, int, float, bool)):
        return value
    if isinstance(value, Path):
        return str(value)
    if isinstance(value, Enum):
        return value.value
    if isinstance(value, dict):
        return {str(key): _value(item) for key, item in value.items()}
    if isinstance(value, (tuple, list)):
        return [_value(item) for item in value]
    if is_dataclass(value):
        return {item.name: _value(getattr(value, item.name)) for item in fields(value)}
    return str(value)
