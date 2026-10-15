"""Deterministic validation for `.uefs` multi-target system contracts."""

from __future__ import annotations

from pathlib import Path
from typing import Any

from uef_gen.language.frontends.project.ast import Identifier, Literal, Reference, TypeSpec
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic, source_error, source_warning
from uef_gen.language.frontends.project.registry_contracts import load_authoring_catalogue
from uef_gen.language.frontends.system.ast import SystemDocument, SystemRecordSpec

_INTEGER_WIDTHS = {"u8": 8, "u16": 16, "u32": 32, "u64": 64, "i8": 8, "i16": 16, "i32": 32, "i64": 64}
_PRIMITIVES = {"bool", "f32", "f64", *_INTEGER_WIDTHS}


def validate_system(document: SystemDocument, *, uetd_root: Path) -> list[LanguageDiagnostic]:
    """Run structural and source-contract checks in stable source order."""
    diagnostics: list[LanguageDiagnostic] = []
    diagnostics.extend(validate_system_nodes(document, system_root=_system_root(document)))
    diagnostics.extend(validate_system_shared_types(document, uetd_root=uetd_root))
    diagnostics.extend(validate_system_variants(document))
    diagnostics.extend(validate_system_flags(document))
    diagnostics.extend(validate_system_fixed_point_types(document, uetd_root=uetd_root))
    diagnostics.extend(validate_system_protocols(document, uetd_root=uetd_root))
    diagnostics.extend(validate_system_links(document, uetd_root=uetd_root))
    diagnostics.extend(validate_hybrid_system_relations(document))
    diagnostics.extend(validate_delivery_policies(document, uetd_root=uetd_root))
    return _deduplicate(diagnostics)


def validate_system_nodes(document: SystemDocument, *, system_root: Path) -> list[LanguageDiagnostic]:
    """Reject missing, ambiguous, or escaping `.uef` node project paths."""
    diagnostics: list[LanguageDiagnostic] = []
    root = system_root.expanduser().resolve(strict=False)
    seen_paths: dict[Path, str] = {}
    seen_names: set[str] = set()
    for node in document.nodes:
        if node.name in seen_names:
            diagnostics.append(source_error("duplicate_system_node", f"Node {node.name!r} is declared more than once.", node.source_span))
        seen_names.add(node.name)
        relative = node.project_path
        if relative.is_absolute() or relative.drive or ".." in relative.parts:
            diagnostics.append(source_error("system_node_path_escape", "Node project paths must be relative and cannot contain '..'.", node.source_span))
            continue
        candidate = (root / relative).resolve(strict=False)
        if not candidate.is_relative_to(root):
            diagnostics.append(source_error("system_node_path_escape", "Node project resolves outside the .uefs workspace.", node.source_span))
            continue
        if candidate.suffix.casefold() != ".uef":
            diagnostics.append(source_error("system_node_suffix", "Node project paths must end in .uef.", node.source_span))
            continue
        prior = seen_paths.get(candidate)
        if prior:
            diagnostics.append(source_error("system_node_path_duplicate", f"Nodes {prior!r} and {node.name!r} point to the same project file.", node.source_span))
        else:
            seen_paths[candidate] = node.name
        if not candidate.is_file():
            diagnostics.append(source_error("system_node_project_missing", f"Node project does not exist: {relative.as_posix()}.", node.source_span))
    endpoint_names = [item.name for item in document.nodes] + [item.name for item in document.external_endpoints]
    for endpoint in document.external_endpoints:
        if endpoint.attached_to:
            owner, separator, member = endpoint.attached_to.partition(".")
            if not separator or not member or owner not in {item.name for item in document.nodes}:
                diagnostics.append(source_error("external_endpoint_attachment", "attached_to must name a member on a declared node (node.member).", endpoint.source_span))
    if len(endpoint_names) != len(set(endpoint_names)):
        duplicates = {name for name in endpoint_names if endpoint_names.count(name) > 1}
        for name in sorted(duplicates):
            span = next((item.source_span for item in (*document.nodes, *document.external_endpoints) if item.name == name), document.source_span)
            diagnostics.append(source_error("duplicate_system_endpoint", f"Endpoint name {name!r} is used more than once.", span))
    return diagnostics


def validate_system_shared_types(document: SystemDocument, *, uetd_root: Path) -> list[LanguageDiagnostic]:
    """Resolve shared record fields and reject duplicate or recursive layouts."""
    diagnostics: list[LanguageDiagnostic] = []
    try:
        catalogue = load_authoring_catalogue(uetd_root)
    except (OSError, ValueError) as exc:
        return [source_error("system_uetd_catalogue_unavailable", str(exc), document.source_span)]
    declared = {item.name for item in (*document.records, *document.enums, *document.variants, *document.flags_types, *document.fixed_point_types)}
    known = _known_types(catalogue) | declared | {item.name for item in document.protocols}
    graph: dict[str, set[str]] = {}
    for record in document.records:
        names: set[str] = set()
        graph[record.name] = set()
        for field_name, type_spec, _ in record.fields:
            if field_name in names:
                diagnostics.append(source_error("system_record_field_duplicate", f"Record {record.name!r} repeats field {field_name!r}.", record.source_span))
            names.add(field_name)
            if type_spec.name not in known:
                diagnostics.append(source_error("system_record_field_type_unknown", f"Field {record.name}.{field_name} uses unknown type {type_spec.name!r}.", type_spec.span or record.source_span))
            if type_spec.name in declared:
                graph[record.name].add(type_spec.name)
            if type_spec.unit and type_spec.unit != "1" and type_spec.unit not in catalogue.units:
                diagnostics.append(source_error("system_record_unit_unknown", f"Field {record.name}.{field_name} uses unregistered unit {type_spec.unit!r}.", type_spec.span or record.source_span))
    for cycle in _type_cycles(graph):
        record = next((item for item in document.records if item.name == cycle[0]), None)
        diagnostics.append(source_error("system_record_recursive_layout", f"Shared records form a recursive by-value layout: {' -> '.join(cycle)}.", record.source_span if record else document.source_span))
    for enum in document.enums:
        diagnostics.extend(_validate_enum(enum.name, enum.storage_type, [(case.name, case.value, case.span) for case in enum.variants], enum.span, explicit=True))
    return diagnostics


def validate_system_variants(document: SystemDocument) -> list[LanguageDiagnostic]:
    """Require unambiguous fixed tags and nonrecursive payloads in shared variants."""
    diagnostics: list[LanguageDiagnostic] = []
    for variant in document.variants:
        diagnostics.extend(_validate_enum(variant.name, variant.tag_type,
                                          [(case.name, case.tag, case.span) for case in variant.cases],
                                          variant.span, explicit=True))
        case_names: set[str] = set()
        for case in variant.cases:
            if case.name in case_names:
                diagnostics.append(source_error("system_variant_case_duplicate", f"Variant {variant.name!r} repeats case {case.name!r}.", case.span))
            case_names.add(case.name)
            field_names = [field.name for field in case.fields]
            if len(field_names) != len(set(field_names)):
                diagnostics.append(source_error("system_variant_field_duplicate", f"Variant case {variant.name}.{case.name} repeats a field.", case.span))
            if any(field.type_spec and field.type_spec.name == variant.name for field in case.fields):
                diagnostics.append(source_error("system_variant_recursive_payload", f"Variant {variant.name!r} cannot contain itself by value.", case.span))
    return diagnostics


def validate_system_flags(document: SystemDocument) -> list[LanguageDiagnostic]:
    """Check shared bitset widths, unique names, and unique bit positions."""
    diagnostics: list[LanguageDiagnostic] = []
    for flags in document.flags_types:
        width = _INTEGER_WIDTHS.get(flags.storage_type)
        if width is None:
            diagnostics.append(source_error("system_flags_storage", f"Flags {flags.name!r} requires an integer storage type.", flags.span))
            continue
        names: set[str] = set()
        positions: set[int] = set()
        for member in flags.members:
            if member.name in names:
                diagnostics.append(source_error("system_flags_member_duplicate", f"Flags {flags.name!r} repeats member {member.name!r}.", member.span))
            if member.bit in positions:
                diagnostics.append(source_error("system_flags_bit_duplicate", f"Flags {flags.name!r} assigns bit {member.bit} more than once.", member.span))
            if not 0 <= member.bit < width:
                diagnostics.append(source_error("system_flags_bit_range", f"Bit {member.bit} does not fit {flags.storage_type}.", member.span))
            names.add(member.name); positions.add(member.bit)
    return diagnostics


def validate_system_fixed_point_types(document: SystemDocument, *, uetd_root: Path) -> list[LanguageDiagnostic]:
    """Validate portable fixed-point storage, scale, units, and arithmetic policy."""
    try:
        catalogue = load_authoring_catalogue(uetd_root)
    except (OSError, ValueError) as exc:
        return [source_error("system_uetd_catalogue_unavailable", str(exc), document.source_span)]
    diagnostics: list[LanguageDiagnostic] = []
    names: set[str] = set()
    for item in document.fixed_point_types:
        if item.name in names:
            diagnostics.append(source_error("system_fixed_name_duplicate", f"Fixed-point type {item.name!r} is duplicated.", item.span))
        names.add(item.name)
        width = _INTEGER_WIDTHS.get(item.storage_type, 0)
        if item.storage_type not in {"i8", "i16", "i32", "i64"}:
            diagnostics.append(source_error("system_fixed_storage", "Shared fixed-point storage must be signed.", item.span))
        if item.fractional_bits < 0 or item.fractional_bits >= width:
            diagnostics.append(source_error("system_fixed_fraction", "fractional_bits must be in [0, storage width).", item.span))
        if item.unit not in catalogue.units:
            diagnostics.append(source_error("system_fixed_unit", f"Unit {item.unit!r} is not registered by UETD.", item.span))
        if item.rounding not in {"nearest_even", "toward_zero", "floor", "ceil"}:
            diagnostics.append(source_error("system_fixed_rounding", f"Unsupported rounding mode {item.rounding!r}.", item.span))
        if item.overflow not in {"reject", "saturate"}:
            diagnostics.append(source_error("system_fixed_overflow", f"Unsupported overflow policy {item.overflow!r}.", item.span))
    return diagnostics


def validate_system_protocols(document: SystemDocument, *, uetd_root: Path) -> list[LanguageDiagnostic]:
    """Check protocol references, message IDs, field types, and bounded offsets."""
    try:
        catalogue = load_authoring_catalogue(uetd_root)
    except (OSError, ValueError) as exc:
        return [source_error("system_uetd_catalogue_unavailable", str(exc), document.source_span)]
    diagnostics: list[LanguageDiagnostic] = []
    known = _known_types(catalogue) | {item.name for item in document.records} | {item.name for item in document.enums} | {item.name for item in document.variants} | {item.name for item in document.flags_types} | {item.name for item in document.fixed_point_types}
    for protocol in document.protocols:
        messages = protocol.messages
        message_names: set[str] = set()
        message_ids: set[int] = set()
        for message in messages:
            name = str(message.get("name", "")); message_id = message.get("id")
            span = message.get("source_span", protocol.source_span)
            if name in message_names:
                diagnostics.append(source_error("system_protocol_message_duplicate", f"Protocol {protocol.name!r} repeats message {name!r}.", span))
            message_names.add(name)
            if not isinstance(message_id, int):
                diagnostics.append(source_error("system_protocol_message_id", f"Message {name!r} requires an explicit integer id.", span))
            elif message_id < 0 or message_id > 0xFFFF:
                diagnostics.append(source_error("system_protocol_message_id_range", f"Message id {message_id} must fit u16.", span))
            elif message_id in message_ids:
                diagnostics.append(source_error("system_protocol_message_id_duplicate", f"Protocol {protocol.name!r} repeats message id {message_id}.", span))
            else:
                message_ids.add(message_id)
            fields = list(message.get("fields", ()))
            fragments = list(message.get("fragments", ()))
            if fields and fragments:
                diagnostics.append(source_error("system_protocol_layout_mixed", f"Message {name!r} cannot mix flat fields and fragments.", span))
            used_names: set[str] = set()
            layouts = [("message", fields)] + [(f"fragment {item.get('index')}", list(item.get("fields", ()))) for item in fragments]
            for layout_name, layout_fields in layouts:
                occupied: list[tuple[int, int, str]] = []
                for field in layout_fields:
                    field_name = str(field.get("name", "")); field_type = field.get("type")
                    field_span = field.get("source_span", span)
                    if field_name in used_names:
                        diagnostics.append(source_error("system_protocol_field_duplicate", f"Message {name!r} repeats field {field_name!r}.", field_span))
                    used_names.add(field_name)
                    if isinstance(field_type, TypeSpec) and field_type.name not in known:
                        diagnostics.append(source_error("system_protocol_field_type", f"Protocol field {name}.{field_name} uses unknown type {field_type.name!r}.", field_span))
                    props = field.get("properties", {})
                    offset = _number(props.get("offset")) if isinstance(props, dict) else None
                    width = _number(props.get("width")) if isinstance(props, dict) else None
                    if offset is not None and (offset < 0 or int(offset) != offset):
                        diagnostics.append(source_error("system_protocol_offset", f"Field {name}.{field_name} needs a nonnegative integral offset.", field_span))
                    if width is not None and (width <= 0 or int(width) != width):
                        diagnostics.append(source_error("system_protocol_width", f"Field {name}.{field_name} needs a positive integral width.", field_span))
                    if offset is not None and width is not None and offset >= 0 and width > 0:
                        start, end = int(offset), int(offset + width)
                        for old_start, old_end, old_name in occupied:
                            if start < old_end and old_start < end:
                                diagnostics.append(source_error("system_protocol_field_overlap", f"Fields {old_name!r} and {field_name!r} overlap within {layout_name} of message {name!r}.", field_span))
                        occupied.append((start, end, field_name))
    return diagnostics


def validate_system_links(document: SystemDocument, *, uetd_root: Path) -> list[LanguageDiagnostic]:
    """Resolve link endpoints and their named record/protocol contracts."""
    del uetd_root
    diagnostics: list[LanguageDiagnostic] = []
    endpoints = {item.name for item in document.nodes} | {item.name for item in document.external_endpoints}
    records = {item.name for item in document.records}
    protocols = {item.name for item in document.protocols}
    link_names: set[str] = set()
    for link in document.links:
        if link.name in link_names:
            diagnostics.append(source_error("system_link_duplicate", f"Link {link.name!r} is declared more than once.", link.source_span))
        link_names.add(link.name)
        for field_name, endpoint in (("from", link.source_endpoint), ("to", link.destination_endpoint)):
            root = endpoint.split(".", 1)[0]
            if root not in endpoints:
                diagnostics.append(source_error("system_link_endpoint_unknown", f"Link {link.name!r} has unknown {field_name} endpoint {endpoint!r}.", link.source_span))
        if link.contract_name and link.contract_name not in records:
            diagnostics.append(source_error("system_link_contract_unknown", f"Link {link.name!r} names unknown record contract {link.contract_name!r}.", link.source_span))
        if link.protocol_name and link.protocol_name not in protocols:
            diagnostics.append(source_error("system_link_protocol_unknown", f"Link {link.name!r} names unknown protocol {link.protocol_name!r}.", link.source_span))
        if link.protocol_name and not _reference_name(link.properties.get("transport")):
            diagnostics.append(source_error("system_link_transport_missing", f"Link {link.name!r} must declare a transport for protocol {link.protocol_name!r}.", link.source_span))
        if link.source_endpoint == link.destination_endpoint:
            diagnostics.append(source_error("system_link_self_connection", f"Link {link.name!r} cannot connect an endpoint to itself.", link.source_span))
    return diagnostics


def validate_hybrid_system_relations(document: SystemDocument) -> list[LanguageDiagnostic]:
    """Check cross-target declarations refer only to declared nodes/endpoints/links."""
    diagnostics: list[LanguageDiagnostic] = []
    known_roots = {item.name for item in document.nodes} | {item.name for item in document.external_endpoints}
    links = {item.name for item in document.links}
    names: set[str] = set()
    for hybrid in document.hybrid_systems:
        if hybrid.name in names:
            diagnostics.append(source_error("hybrid_system_duplicate", f"Hybrid system {hybrid.name!r} is duplicated.", hybrid.source_span))
        names.add(hybrid.name)
        for name, value in hybrid.properties.items():
            reference = _reference_name(value)
            if not reference:
                continue
            root = reference.split(".", 1)[0]
            if name.endswith("_link") or name == "link":
                if reference not in links:
                    diagnostics.append(source_error("hybrid_system_link_unknown", f"Hybrid binding {name!r} refers to unknown link {reference!r}.", hybrid.source_span))
            elif "." in reference and root not in known_roots and root not in links:
                diagnostics.append(source_error("hybrid_system_binding_unknown", f"Hybrid binding {name!r} refers to undeclared endpoint or link {reference!r}.", hybrid.source_span))
    return diagnostics


def validate_delivery_policies(document: SystemDocument, *, uetd_root: Path) -> list[LanguageDiagnostic]:
    """Check delivery policy declarations and link references for bounded semantics."""
    del uetd_root
    diagnostics: list[LanguageDiagnostic] = []
    policies = {item.get("name"): item for item in document.delivery_policies if isinstance(item.get("name"), str)}
    for policy in document.delivery_policies:
        props = policy.get("properties", {})
        semantics = _reference_name(props.get("semantics")) if isinstance(props, dict) else None
        overrun = _reference_name(props.get("overrun")) if isinstance(props, dict) else None
        if semantics not in {"fifo", "latest_value", "ordered_frames", "event_queue"}:
            diagnostics.append(source_error("system_delivery_semantics", f"Delivery policy {policy.get('name')!r} has unsupported semantics {semantics!r}.", policy.get("source_span")))
        if overrun is None:
            diagnostics.append(source_error("system_delivery_overrun_missing", f"Delivery policy {policy.get('name')!r} must define overrun behavior.", policy.get("source_span")))
    for link in document.links:
        reference = _reference_name(link.properties.get("delivery"))
        if reference in policies:
            continue
        if reference in {"fifo", "latest_value", "ordered_frames", "event_queue"}:
            # Inline semantics are a concise contract and are legal without a
            # separately named policy declaration.
            continue
        if reference and reference not in policies:
            diagnostics.append(source_error("system_delivery_policy_unknown", f"Link {link.name!r} refers to unknown delivery policy {reference!r}.", link.source_span))
    return diagnostics


def _system_root(document: SystemDocument) -> Path:
    return document.source_path.parent if document.source_path else Path.cwd()


def _known_types(catalogue: Any) -> set[str]:
    names = set(_PRIMITIVES) | set(catalogue.types)
    for definitions in catalogue.types.values():
        for item in definitions:
            names.add(item.c_name)
            if item.dsl_name:
                names.add(item.dsl_name)
    return names


def _validate_enum(name: str, storage: str, cases: list[tuple[str, int | None, Any]], span: Any, *, explicit: bool) -> list[LanguageDiagnostic]:
    diagnostics: list[LanguageDiagnostic] = []
    width = _INTEGER_WIDTHS.get(storage)
    if width is None:
        return [source_error("system_enum_storage", f"Type {name!r} requires fixed-width integer storage.", span)]
    signed = storage.startswith("i")
    low = -(1 << (width - 1)) if signed else 0
    high = (1 << (width - (1 if signed else 0))) - 1
    names: set[str] = set(); values: set[int] = set()
    next_value = 0
    for case_name, value, case_span in cases:
        if case_name in names:
            diagnostics.append(source_error("system_enum_case_duplicate", f"Type {name!r} repeats case {case_name!r}.", case_span))
        names.add(case_name)
        if value is None:
            if explicit:
                diagnostics.append(source_error("system_enum_tag_required", f"System contract case {name}.{case_name} requires an explicit stable integer value.", case_span))
                value = next_value
            else:
                value = next_value
        if not low <= value <= high:
            diagnostics.append(source_error("system_enum_value_range", f"Value {value} for {name}.{case_name} does not fit {storage}.", case_span))
        if value in values:
            diagnostics.append(source_error("system_enum_value_duplicate", f"Type {name!r} repeats value {value}.", case_span))
        values.add(value); next_value = value + 1
    return diagnostics


def _protocol_fields(message: dict[str, Any]):
    """Yield flat fields and each fragment's fields in independent layouts."""
    yield from message.get("fields", ())
    for fragment in message.get("fragments", ()):
        yield from fragment.get("fields", ())


def _number(value: Any) -> int | float | None:
    if isinstance(value, Literal) and isinstance(value.value, (int, float)) and not isinstance(value.value, bool):
        return value.value
    return None


def _reference_name(value: Any) -> str | None:
    if isinstance(value, Reference):
        return ".".join(value.path)
    if isinstance(value, Identifier):
        return value.value
    if isinstance(value, Literal) and isinstance(value.value, str):
        return value.value
    return None


def _type_cycles(graph: dict[str, set[str]]) -> list[list[str]]:
    cycles: list[list[str]] = []
    done: set[str] = set()
    stack: list[str] = []
    active: set[str] = set()
    def visit(name: str) -> None:
        if name in active:
            start = stack.index(name)
            cycles.append([*stack[start:], name])
            return
        if name in done:
            return
        active.add(name); stack.append(name)
        for child in sorted(graph.get(name, ())):
            if child in graph:
                visit(child)
        stack.pop(); active.remove(name); done.add(name)
    for name in sorted(graph):
        visit(name)
    return cycles


def _deduplicate(items: list[LanguageDiagnostic]) -> list[LanguageDiagnostic]:
    seen: set[tuple[str, str, str, Any]] = set()
    result: list[LanguageDiagnostic] = []
    for item in items:
        span = item.span
        key = (item.severity, item.code, item.message, (span.path, span.start.offset) if span else None)
        if key not in seen:
            seen.add(key); result.append(item)
    return result
