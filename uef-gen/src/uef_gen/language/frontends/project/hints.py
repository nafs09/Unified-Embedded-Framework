"""Registry-backed completion, hover, and signature information for UEF-PL."""

from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Literal
from uef_gen.language.registry.loader import LanguageBuiltin, LanguageRegistry
from uef_gen.language.frontends.project.registry_contracts import AuthoringCatalogue

HintKind = Literal["keyword", "descriptor", "builtin", "field", "value", "function", "type", "target", "runtime"]

@dataclass(frozen=True)
class LanguageHint:
    """Editor-neutral completion, hover, or signature item."""

    kind: HintKind
    label: str
    detail: str
    documentation: str = ""
    insert_text: str | None = None
    sort_text: str | None = None
    data: dict[str, Any] | None = None

@dataclass(frozen=True)
class HintContext:
    """Source cursor context computed by an editor adapter or caller."""

    mode: Literal["declaration", "implementation", "implementation_descriptor", "descriptor", "component_field", "builtin_field", "expression", "type", "target", "runtime", "import_path"]
    prefix: str = ""
    descriptor: str | None = None
    expected_kind: str | None = None
    field_name: str | None = None
    source_offset: int | None = None
    active_parameter: int | None = None
    partial_ast: Any | None = None
    symbols: Any | None = None
    type_environment: Any | None = None
    expected_type: Any | None = None
    source_path: str | None = None

def complete(context: HintContext, registry: LanguageRegistry, catalogue: AuthoringCatalogue) -> list[LanguageHint]:
    """Return prefix-filtered items for the supplied syntactic context.

    The adapter supplies ``mode`` and, where useful, ``descriptor`` or a
    partial AST/symbol table. Registry order is made deterministic by sorting
    labels; no UETD declarations are guessed from their spelling.
    """
    candidates: list[LanguageHint] = []
    if context.mode in {"declaration", "descriptor"}:
        candidates.extend(_descriptor_hints(catalogue, context.expected_kind))
        if context.mode == "declaration":
            candidates.extend(_keyword_hints(registry))
    if context.mode == "implementation":
        candidates.extend(_implementation_hints(context))
    if context.mode == "implementation_descriptor":
        binding = catalogue.dsl_descriptors.get(context.descriptor or "")
        if binding is not None:
            contract = binding.contract
            detail = contract.display_name if contract else binding.module_id
            candidates.append(LanguageHint(
                "descriptor", binding.descriptor, f"UETD module: {detail}",
                f"Matches the constructor declaration for this implementation. Module: `{binding.module_id}`.",
                binding.descriptor, data={"module_id": binding.module_id, "kind": contract.kind if contract else "registered"},
            ))
    if context.mode == "component_field":
        candidates.extend(_component_field_hints(context, catalogue))
    if context.mode == "builtin_field":
        candidates.extend(_builtin_field_hints(context, registry))
    if context.mode == "type":
        candidates.extend(complete_types(context, context.type_environment))
        candidates.extend(_catalogue_type_hints(context, catalogue))
    if context.mode == "runtime":
        candidates.extend(
            LanguageHint("runtime", name, contract.display_name, f"UETD runtime module {contract.registry_id}.", name)
            for name, contract in catalogue.runtimes.items()
        )
    if context.mode == "import_path":
        candidates.extend(complete_contract_import_paths(context))
    if context.mode == "target":
        candidates.extend(_target_hints(context.partial_ast))
    if context.mode == "expression":
        candidates.extend(builtin_hint(item) for item in registry.builtins.values())
        candidates.extend(_math_hints(catalogue))
        candidates.extend(_local_value_hints(context))
    return _filter_sort(candidates, context.prefix)

def hover(context: HintContext, registry: LanguageRegistry, catalogue: AuthoringCatalogue) -> LanguageHint | None:
    """Resolve the hovered name against language, UETD, type, or local symbols."""
    name = context.field_name or context.prefix
    if not name:
        return None
    binding = catalogue.dsl_descriptors.get(name)
    if binding:
        contract = binding.contract
        if contract is None:
            return LanguageHint("descriptor", name, binding.module_id,
                                f"Registered in UETD, but no authoring contract is available. Source: {binding.source_path}", data={"module_id": binding.module_id})
        status = contract.generation.get("status", "unspecified")
        details = [f"UETD module: {contract.registry_id}", f"Kind: {contract.kind}", f"Generation: {status}", f"Manifest: {contract.source_path}"]
        return LanguageHint("descriptor", name, contract.display_name, "\n".join(details), data={"module_id": contract.registry_id, "kind": contract.kind})
    math = catalogue.math_functions.get(name)
    if math:
        signatures = [item.dsl_signature or f"{name}({item.parameters})" for item in math]
        details = [
            f"- `{item.dsl_signature or item.c_signature}` → `{item.c_name}` · {item.implementation_status}/{item.dsl_binding_status} · `{item.header}`"
            for item in math
        ]
        contexts = sorted({context for item in math for context in item.callable_contexts})
        return LanguageHint("function", name, "\n".join(signatures),
                            "UMATH overloads are selected by argument types.\n\n" + "\n".join(details)
                            + f"\n\nCallable in: {', '.join(contexts)}",
                            data={"headers": sorted({item.header for item in math}), "c_names": [item.c_name for item in math]})
    builtin = registry.builtins.get(name)
    if builtin:
        return builtin_hint(builtin)
    if name in catalogue.units:
        item = catalogue.units[name]
        return LanguageHint("value", name, "registered unit", str(item), data={"unit": name})
    type_hint = _type_hint(name, context.type_environment, catalogue)
    if type_hint:
        return type_hint
    symbol = _lookup_local_symbol(name, context)
    if symbol is not None:
        return LanguageHint("value", name, _symbol_detail(symbol), data={"symbol": name})
    return None

def signature_help(descriptor: str, registry: LanguageRegistry, catalogue: AuthoringCatalogue) -> list[LanguageHint]:
    """Return the declared parameter and port contract for a UETD constructor."""
    if descriptor in registry.builtins:
        builtin = registry.builtins[descriptor]
        return [LanguageHint("builtin", builtin.builtin_id, builtin.syntax, builtin.description,
                             data={"fields": list(builtin.fields), "field_policy": builtin.field_policy})]
    if descriptor in catalogue.math_functions:
        return _math_signature(catalogue.math_functions[descriptor])
    binding = catalogue.dsl_descriptors.get(descriptor)
    if binding is None:
        binding = next((item for item in catalogue.dsl_descriptors.values() if item.module_id == descriptor), None)
    if binding is None:
        return []
    contract = binding.contract
    if contract is None:
        return [LanguageHint("descriptor", binding.descriptor, binding.module_id,
                             f"No authoring contract is registered yet. Manifest: {binding.source_path}")]
    values: list[LanguageHint] = []
    for item in contract.properties:
        name = str(item.get("name", "<unnamed>"))
        required = item.get("required") is True
        type_name = item.get("type", "value")
        values.append(LanguageHint("field", name, f"{type_name}{' (required)' if required else ''}",
                                   str(item.get("description", "")), data=dict(item)))
    for port in (*contract.inputs, *contract.outputs):
        direction = str(port.get("direction", "port"))
        values.append(LanguageHint("field", str(port.get("name", "<unnamed>")),
                                   f"{direction}: {port.get('type', 'unspecified')}",
                                   str(port.get("description", "")), data=dict(port)))
    if not values:
        values.append(LanguageHint("descriptor", binding.descriptor, contract.display_name,
                                   f"UETD module {contract.registry_id}; no parameter or port fields are declared."))
    return values

def complete_types(context: HintContext, type_environment: Any) -> list[LanguageHint]:
    """Offer all types visible in the compiler's merged environment."""
    values: list[LanguageHint] = []
    if type_environment is not None:
        for category in ("language_types", "project_types", "imported_types", "uetd_types_by_dsl_name"):
            mapping = getattr(type_environment, category, {})
            if not isinstance(mapping, dict):
                continue
            for name, resolved in mapping.items():
                identity = getattr(resolved, "identity", None)
                owner = getattr(identity, "owner", category.removesuffix("_types"))
                canonical = getattr(identity, "canonical_name", name)
                c_name = getattr(identity, "c_name", None)
                detail = canonical if not c_name or c_name == canonical else f"{canonical} ({c_name})"
                values.append(LanguageHint("type", name, detail, f"Owner: {owner}", name, data={"owner": owner, "canonical_name": canonical}))
    return _filter_sort(values, context.prefix)

def complete_contract_import_paths(context: HintContext) -> list[LanguageHint]:
    """Complete relative `.uefs` paths under the importing source directory."""
    if not context.source_path:
        return []
    source = Path(context.source_path).expanduser().resolve(strict=False)
    root = source.parent
    raw_prefix = context.prefix.replace("\\", "/")
    prefix_dir, _, partial = raw_prefix.rpartition("/")
    search_root = (root / prefix_dir).resolve(strict=False) if prefix_dir else root
    if not search_root.is_relative_to(root) or not search_root.is_dir():
        return []
    hints: list[LanguageHint] = []
    for item in sorted(search_root.iterdir(), key=lambda path: path.name.casefold()):
        candidate = item.resolve(strict=False)
        if not candidate.is_relative_to(root):
            continue
        if not item.name.casefold().startswith(partial.casefold()):
            continue
        if item.is_dir():
            label = f"{prefix_dir + '/' if prefix_dir else ''}{item.name}/"
            hints.append(LanguageHint("value", label, "project folder", insert_text=label))
        elif item.is_file() and item.suffix.casefold() == ".uefs":
            label = f"{prefix_dir + '/' if prefix_dir else ''}{item.name}"
            hints.append(LanguageHint("value", label, "system contract", insert_text=label))
    return hints

def umath_signature_help(function_name: str, context: HintContext, catalogue: AuthoringCatalogue) -> list[LanguageHint]:
    """Show the registered UMATH spelling and reviewed binding metadata."""
    del context
    contracts = catalogue.math_functions.get(function_name, ())
    return _math_signature(contracts) if contracts else []

def builtin_hint(builtin: LanguageBuiltin) -> LanguageHint:
    """Convert a language-owned builtin record to editor-neutral metadata."""
    return LanguageHint("builtin", builtin.builtin_id, builtin.syntax, builtin.description,
                        insert_text=f"{builtin.builtin_id}($0)",
                        data={"parse_rule": builtin.parse_rule, "lowering_rule": builtin.lowering_rule,
                              "fields": list(builtin.fields), "field_policy": builtin.field_policy})

def _descriptor_hints(catalogue: AuthoringCatalogue, expected_kind: str | None) -> list[LanguageHint]:
    hints = []
    for descriptor, binding in catalogue.dsl_descriptors.items():
        contract = binding.contract
        kind = contract.kind if contract else "registered"
        if expected_kind and contract and kind != expected_kind and expected_kind != "module":
            continue
        detail = contract.display_name if contract else binding.module_id
        readiness = contract.generation.get("status") if contract else "authoring contract missing"
        hints.append(LanguageHint("descriptor", descriptor, f"{kind}: {detail}",
                                  f"UETD module {binding.module_id}; generation {readiness}; {binding.source_path}",
                                  descriptor, data={"module_id": binding.module_id, "kind": kind}))
    return hints

def _keyword_hints(registry: LanguageRegistry) -> list[LanguageHint]:
    return [LanguageHint("keyword", item.construct_id, item.syntax, item.description, item.construct_id)
            for item in registry.constructs.values()]

def _implementation_hints(context: HintContext) -> list[LanguageHint]:
    signatures = getattr(context.symbols, "signatures", {})
    implemented = getattr(context.symbols, "implementations", {})
    if not isinstance(signatures, dict):
        signatures = {}
    if not isinstance(implemented, dict):
        implemented = {}
    return [LanguageHint("value", name, f"implementation body: {getattr(signature, 'kind', 'component')}",
                         insert_text=f"{name} {{\n    $0\n}}", data={"kind": getattr(signature, "kind", None)})
            for name, signature in signatures.items() if name not in implemented]

def _component_field_hints(context: HintContext, catalogue: AuthoringCatalogue) -> list[LanguageHint]:
    binding = catalogue.dsl_descriptors.get(context.descriptor or "")
    if binding is None or binding.contract is None:
        return []
    hints = []
    for item in binding.contract.properties:
        name = item.get("name")
        if isinstance(name, str):
            hints.append(LanguageHint("field", name, str(item.get("type", "value")),
                                      str(item.get("description", "")), f"{name} = $0;", data=dict(item)))
    return hints

def _builtin_field_hints(context: HintContext, registry: LanguageRegistry) -> list[LanguageHint]:
    builtin = registry.builtins.get(context.descriptor or "")
    if builtin is None:
        return []
    return [LanguageHint("field", str(item.get("name", "")), str(item.get("type", "value")),
                         str(item.get("description", "")), data=dict(item)) for item in builtin.fields]

def _math_hints(catalogue: AuthoringCatalogue) -> list[LanguageHint]:
    return [LanguageHint(
        "function", name,
        "\n".join(item.dsl_signature or f"{name}({item.parameters})" for item in contracts),
        f"UMATH overload set ({len(contracts)} candidates); selected by argument types.",
        f"{name}($0)", data={"headers": sorted({item.header for item in contracts}),
                             "c_names": [item.c_name for item in contracts]},
    ) for name, contracts in catalogue.math_functions.items()]

def _math_signature(contracts: Any) -> list[LanguageHint]:
    if not contracts:
        return []
    if not isinstance(contracts, (tuple, list)):
        contracts = (contracts,)
    result: list[LanguageHint] = []
    for contract in contracts:
        state = "ready" if contract.available_for_generation and contract.dsl_binding_status == "ready" else f"{contract.implementation_status}; binding {contract.dsl_binding_status}"
        result.append(LanguageHint("function", contract.dsl_name, contract.dsl_signature or contract.parameters,
                                  f"UETD declaration: {contract.c_name}\nHeader: {contract.header}\nCallable in: {', '.join(contract.callable_contexts)}\nGeneration: {state}",
                                  f"{contract.dsl_name}($0)", data={"c_name": contract.c_name, "header": contract.header,
                                                                   "active_parameter": contract.parameters}))
    return result

def _target_hints(partial_ast: Any) -> list[LanguageHint]:
    """Return targets only when caller provides a registry-backed catalogue."""
    values = getattr(partial_ast, "targets", ())
    result = []
    for item in values if isinstance(values, (list, tuple)) else ():
        if isinstance(item, dict) and isinstance(item.get("id"), str):
            result.append(LanguageHint("target", item["id"], str(item.get("name", "UETD target")), data=dict(item)))
    return result

def _local_value_hints(context: HintContext) -> list[LanguageHint]:
    hints: list[LanguageHint] = []
    for name, symbol in _symbols(context):
        hints.append(LanguageHint("value", name, _symbol_detail(symbol), name, data={"symbol": name}))
    return hints

def _lookup_local_symbol(name: str, context: HintContext) -> Any | None:
    return next((value for key, value in _symbols(context) if key == name), None)

def _symbols(context: HintContext) -> list[tuple[str, Any]]:
    result: list[tuple[str, Any]] = []
    table = context.symbols
    if table is not None:
        for category in ("project_parameters", "signals", "components", "models", "algorithms", "subsystems", "state_machines", "project_routines"):
            value = getattr(table, category, {})
            if isinstance(value, dict):
                result.extend(value.items())
    document = context.partial_ast
    if document is not None:
        result.extend((binding.name, binding) for binding in getattr(document, "parameters", ()))
        for declaration in getattr(document, "declarations", ()):
            if declaration.name and not declaration.prototype:
                result.append((declaration.name, declaration))
    unique: dict[str, Any] = {}
    for name, value in result:
        unique.setdefault(name, value)
    return list(unique.items())

def _type_hint(name: str, environment: Any, catalogue: AuthoringCatalogue) -> LanguageHint | None:
    definitions = catalogue.types.get(name, ())
    if not definitions:
        definitions = tuple(item for values in catalogue.types.values() for item in values if name in {item.dsl_name, item.c_name})
    if definitions:
        first = definitions[0]
        return LanguageHint("type", first.dsl_name or first.c_name, first.kind,
                            f"C type: {first.c_name}\nHeader: {first.header}\nVisibility: {first.language_visibility}\nSource status: {first.source_status}",
                            data={"c_name": first.c_name, "header": first.header, "source_status": first.source_status})
    if environment is not None:
        for category in ("language_types", "project_types", "imported_types", "uetd_types_by_dsl_name"):
            mapping = getattr(environment, category, {})
            if name in mapping:
                resolved = mapping[name]
                identity = getattr(resolved, "identity", None)
                return LanguageHint("type", name, getattr(identity, "canonical_name", name),
                                    f"Owner: {getattr(identity, 'owner', category)}", data={"owner": getattr(identity, "owner", None)})
    return None


def _catalogue_type_hints(context: HintContext, catalogue: AuthoringCatalogue) -> list[LanguageHint]:
    hints: list[LanguageHint] = []
    for name in ("bool", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64", "q15", "q31"):
        hints.append(LanguageHint("type", name, "UEF-PL scalar", insert_text=name))
    for name, definitions in catalogue.types.items():
        for definition in definitions:
            label = definition.dsl_name or definition.c_name
            hints.append(LanguageHint("type", label, definition.kind,
                                      f"{definition.declaration}\nHeader: {definition.header}\nVisibility: {definition.language_visibility}",
                                      label, data={"c_name": definition.c_name, "header": definition.header}))
    document = context.partial_ast
    if document is not None:
        for item in (*getattr(document, "records", ()), *getattr(document, "enums", ()), *getattr(document, "variants", ()), *getattr(document, "flags", ()), *getattr(document, "fixed_point_types", ())):
            hints.append(LanguageHint("type", item.name, "project type", insert_text=item.name))
    return hints

def _symbol_detail(symbol: Any) -> str:
    type_spec = getattr(symbol, "type_spec", None)
    if type_spec is None:
        bindings = getattr(symbol, "bindings", ())
        type_spec = bindings[0].type_spec if bindings else None
    if type_spec is not None:
        return getattr(type_spec, "name", str(type_spec))
    kind = getattr(symbol, "kind", None)
    return str(kind or type(symbol).__name__)

def _filter_sort(hints: list[LanguageHint], prefix: str) -> list[LanguageHint]:
    needle = prefix.casefold()
    unique: dict[tuple[str, str], LanguageHint] = {}
    for item in hints:
        if needle and not item.label.casefold().startswith(needle):
            continue
        unique.setdefault((item.kind, item.label), item)
    return sorted(unique.values(), key=lambda item: (item.label.casefold(), item.kind, item.detail.casefold()))
