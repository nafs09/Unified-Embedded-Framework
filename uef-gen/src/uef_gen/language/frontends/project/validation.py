"""Semantic/type validation for standalone UETD project declarations."""

from __future__ import annotations
import math
import re
from dataclasses import fields, is_dataclass
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any
from uef_gen.language.registry.loader import LanguageRegistry, load_language_registry
from uef_gen.language.frontends.project.ast import (
    BinaryExpression,
    BoundedLoop,
    ConditionalStatement,
    Declaration,
    Equation,
    EnumDefinition,
    FlagsDefinition,
    FixedPointDefinition,
    Identifier,
    IndexExpression,
    Literal,
    ListValue,
    MathCallExpression,
    ProjectDocument,
    RecordValue,
    Reference,
    RecordDefinition,
    SourceMap,
    TypeSpec,
    UnaryExpression,
    VariantDefinition,
)
from uef_gen.language.frontends.project.contract_imports import ImportedSystemContract, build_contract_type_scope
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic, source_error, source_warning
from uef_gen.language.frontends.project.registry_contracts import AuthoringCatalogue
from uef_gen.language.frontends.project.type_interchange import (
    ResolvedType,
    TypeIdentity,
    TypeEnvironment,
    TypedExpressionFacts,
    build_type_environment,
    check_assignable,
    check_explicit_cast,
    check_fixed_point_operation,
    check_flags_operation,
    check_unit_conversion,
    check_variant_match,
    derive_time_rate_type,
    infer_expression_type,
    resolve_type_spec,
    validate_type_boundaries,
)

@dataclass
class ProjectSymbolTable:
    """Header signatures and implementation bodies indexed before resolution."""

    signatures: dict[str, Declaration] = field(default_factory=dict)
    implementations: dict[str, Declaration] = field(default_factory=dict)
    project_routines: dict[str, Declaration] = field(default_factory=dict)
    algorithms: dict[str, Declaration] = field(default_factory=dict)
    components: dict[str, Declaration] = field(default_factory=dict)
    custom_algorithms: dict[str, Declaration] = field(default_factory=dict)
    hybrid_algorithms: dict[str, Declaration] = field(default_factory=dict)
    protocols: dict[str, Declaration] = field(default_factory=dict)
    custom_protocols: dict[str, Declaration] = field(default_factory=dict)
    state_machines: dict[str, Declaration] = field(default_factory=dict)
    models: dict[str, Declaration] = field(default_factory=dict)
    sensors: dict[str, Declaration] = field(default_factory=dict)
    actuators: dict[str, Declaration] = field(default_factory=dict)
    subsystems: dict[str, Declaration] = field(default_factory=dict)
    signals: dict[str, Declaration] = field(default_factory=dict)
    project_parameters: dict[str, Declaration] = field(default_factory=dict)
    records: dict[str, RecordDefinition] = field(default_factory=dict)
    enums: dict[str, EnumDefinition] = field(default_factory=dict)
    variants: dict[str, VariantDefinition] = field(default_factory=dict)
    flags: dict[str, FlagsDefinition] = field(default_factory=dict)
    fixed_point_types: dict[str, FixedPointDefinition] = field(default_factory=dict)
    imported_types: dict[str, object] = field(default_factory=dict)
    diagnostics: list[LanguageDiagnostic] = field(default_factory=list)

@dataclass(frozen=True)
class TypeValidationResult:
    """Merged type environment and expression facts for later compiler passes."""

    environment: TypeEnvironment
    expression_facts: tuple[TypedExpressionFacts, ...]
    diagnostics: tuple[LanguageDiagnostic, ...]

def validate_type_interchange(document: ProjectDocument, catalogue: AuthoringCatalogue, language_registry: LanguageRegistry,
                              symbols: ProjectSymbolTable, imported_contracts: tuple[ImportedSystemContract, ...]) -> TypeValidationResult:
    """Build project and declaration-local scopes, then type-check expressions.

    Component inputs, locals, and composition stage names are lexical values:
    they exist only inside the component that declares them. Keeping them out
    of the project scope prevents two unrelated components that both use a
    name such as ``attitude`` or ``output`` from overwriting one another.
    """
    environment, diagnostics = build_type_environment(document, language_registry, catalogue, imported_contracts)
    for expression, context in _expressions_with_context(document):
        for value in _walk_values(expression):
            if isinstance(value, (MathCallExpression, BinaryExpression)):
                environment.math_call_contexts[id(value)] = context
    symbols.imported_types.update(environment.imported_types)
    diagnostics.extend(validate_project_type_declarations(document, catalogue, symbols, environment))
    project_scope: dict[str, Any] = {}
    project_scope.update(environment.language_types)
    project_scope.update(environment.project_types)
    project_scope.update(environment.imported_types)
    for name, declaration in symbols.signals.items():
        binding = declaration.bindings[0] if declaration.bindings else None
        if binding and binding.type_spec:
            resolved, errors = resolve_type_spec(binding.type_spec, environment)
            diagnostics.extend(errors)
            if resolved: project_scope[name] = resolved
    for binding in document.parameters:
        if binding.type_spec and binding.name not in project_scope:
            resolved, errors = resolve_type_spec(binding.type_spec, environment)
            diagnostics.extend(errors)
            if resolved: project_scope[binding.name] = resolved
    _add_component_input_scope(symbols, catalogue, environment, project_scope, diagnostics)
    _add_component_output_scope(symbols, catalogue, environment, project_scope, diagnostics)
    _add_state_machine_state_scope(document, project_scope, environment)
    _add_protocol_message_scope(document, catalogue, environment, symbols, imported_contracts, project_scope, diagnostics)
    _add_operating_system_channel_scope(document, project_scope, environment)
    _add_operating_system_runtime_scope(document, environment, project_scope)
    facts: list[TypedExpressionFacts] = []
    expected_by_expression: dict[int, Any] = {}
    scope_by_expression: dict[int, dict[str, Any]] = {}
    non_value_property_ids: set[int] = set()
    discretization_periods: dict[int, Any] = {}
    deferred_record_arguments: set[int] = set()
    _add_protocol_send_expected_types(
        document, catalogue, environment, symbols, imported_contracts,
        expected_by_expression, diagnostics,
    )

    def record_expected(expression: Any, type_spec: Any) -> None:
        if expression is None or type_spec is None:
            return
        resolved, errors = resolve_type_spec(type_spec, environment)
        diagnostics.extend(errors)
        if resolved is not None:
            expected_by_expression[id(expression)] = resolved

    def record_equation_target(statement: Any, scope: dict[str, Any]) -> None:
        if not isinstance(statement, Equation):
            return
        target_result, target_errors = infer_expression_type(statement.target, scope, environment)
        diagnostics.extend(target_errors)
        if target_result is not None:
            expected_type = target_result.resolved_type
            if statement.mode == "derivative":
                expected_type = derive_time_rate_type(expected_type, environment)
                if expected_type is None:
                    diagnostics.append(source_error(
                        "derivative_target_not_numeric",
                        "A model derivative target must have a numeric scalar or fixed-shape vector/matrix type.",
                        statement.target.span,
                    ))
                    return
            expected_by_expression[id(statement.expression)] = expected_type

    for binding in document.parameters:
        record_expected(binding.value, binding.type_spec)
    for declaration in _walk_declarations(document.declarations):
        if declaration.kind == "choose":
            continue
        metadata_properties = ({"owner", "consumers"} if declaration.kind == "state_machine" else
                              {"producer", "consumer", "consumers", "destination", "overflow", "ownership",
                               "memory_region", "cache_policy", "barrier_policy", "notification", "clock_policy",
                               "alignment"} if declaration.kind == "channel" else set())
        non_value_property_ids.update(id(prop.value) for prop in declaration.properties if prop.name in metadata_properties)
        scope = dict(project_scope)
        for binding in declaration.bindings:
            if binding.type_spec:
                resolved, errors = resolve_type_spec(binding.type_spec, environment)
                diagnostics.extend(errors)
                if resolved is not None:
                    scope[binding.name] = resolved
        for stage in declaration.stages:
            for binding in stage.parameters:
                if binding.type_spec:
                    resolved, errors = resolve_type_spec(binding.type_spec, environment)
                    diagnostics.extend(errors)
                    if resolved is not None:
                        scope[binding.name] = resolved
        for local in declaration.composition_locals:
            resolved, errors = resolve_type_spec(local.type_spec, environment)
            diagnostics.extend(errors)
            if resolved is not None:
                scope[local.name] = resolved
        _add_composition_value_scope(declaration, symbols, catalogue, environment, scope, diagnostics)
        for value in _walk_values((declaration.equations, declaration.stages)):
            if isinstance(value, BoundedLoop):
                scope.setdefault(value.variable, environment.language_types["i32"])
        for expression in _declaration_expressions(declaration):
            if id(expression) in non_value_property_ids:
                continue
            scope_by_expression[id(expression)] = scope
        for prop in declaration.properties:
            if prop.name in {"when", "guard", "condition", "feed_only_when", "valid_when"}:
                expected_by_expression[id(prop.value)] = environment.language_types["bool"]
            if declaration.kind == "channel" and prop.name == "capacity":
                record_expected(prop.value, TypeSpec("u16", span=prop.span))
            if declaration.kind == "model" and prop.name == "discretize" and isinstance(prop.value, RecordValue):
                # `discretize with METHOD at PERIOD` is represented as a small
                # internal record for lowering, not as a user-authored record
                # constructor. Type-check only the period here; the method is
                # resolved by model validation/backend registry lookup.
                period = dict(prop.value.fields).get("period")
                if period is not None:
                    discretization_periods[id(period)] = prop.span
        for binding in declaration.bindings:
            record_expected(binding.value, binding.type_spec)
        if declaration.output_assignments:
            signature = symbols.signatures.get(declaration.name or "")
            body_bindings = declaration.bindings or (signature.bindings if signature else ())
            output_types = {binding.name: binding.type_spec for binding in body_bindings
                            if binding.role == "output" and binding.type_spec is not None}
            if signature is not None:
                contract_binding = catalogue.dsl_descriptors.get(signature.registry_key or "")
                if contract_binding and contract_binding.contract:
                    for port in contract_binding.contract.outputs:
                        if isinstance(port.get("name"), str) and isinstance(port.get("type"), str):
                            output_types.setdefault(port["name"], TypeSpec(port["type"]))
            for output in declaration.output_assignments:
                record_expected(output.expression, output_types.get(output.name))
        for local in declaration.composition_locals:
            record_expected(local.expression, local.type_spec)
        for stage in declaration.stages:
            for statement in stage.body:
                if isinstance(statement, ConditionalStatement):
                    expected_by_expression[id(statement.condition)] = environment.language_types["bool"]
                record_equation_target(statement, scope)
        for statement in declaration.equations:
            if isinstance(statement, ConditionalStatement):
                expected_by_expression[id(statement.condition)] = environment.language_types["bool"]
            record_equation_target(statement, scope)
        for stage in declaration.composition_stages:
            member_alias = stage.member.split(".", 1)[0]
            member_target = next((child.registry_key for child in declaration.children
                                  if child.kind == "use" and child.name == member_alias), None)
            input_types = _composition_member_ports(member_target, symbols, catalogue, "input")
            if input_types is not None:
                for argument, expression in stage.arguments:
                    record_expected(expression, input_types.get(argument))
            else:
                target_signature = symbols.signatures.get((member_target or "").split(".", 1)[0])
                target_descriptor = (catalogue.dsl_descriptors.get(target_signature.registry_key or "")
                                     if target_signature is not None else None)
                has_unavailable_uetd_contract = (
                    target_signature is not None
                    and bool(target_signature.registry_key)
                    and (target_descriptor is None or target_descriptor.contract is None)
                )
                # A record-valued call argument needs a nominal expected type.
                # When its UETD component has no authoring contract, the
                # contract-missing diagnostic is the actionable result; do not
                # report a misleading map_requires_expected_record error.
                if has_unavailable_uetd_contract:
                    deferred_record_arguments.update(
                        id(expression) for _, expression in stage.arguments
                        if isinstance(expression, RecordValue)
                    )
            if stage.guard is not None:
                expected_by_expression[id(stage.guard)] = environment.language_types["bool"]
        for invocation in (child for child in declaration.children if child.kind == "run"):
            input_types = _composition_member_ports(invocation.name, symbols, catalogue, "input")
            if input_types is not None:
                for argument in invocation.properties:
                    record_expected(argument.value, input_types.get(argument.name))
        if declaration.composition_outputs:
            signature = symbols.signatures.get(declaration.name or "")
            body_bindings = declaration.bindings or (signature.bindings if signature else ())
            output_types = {
                binding.name: binding.type_spec
                for binding in body_bindings
                if binding.role == "output" and binding.type_spec is not None
            }
            for output in declaration.composition_outputs:
                record_expected(output.expression, output_types.get(output.name))
        choose_output_types = {
            binding.name: binding.type_spec
            for binding in declaration.bindings
            if binding.role == "output" and binding.type_spec is not None
        }
        signature = symbols.signatures.get(declaration.name or "")
        if signature is not None:
            for binding in signature.bindings:
                if binding.role == "output" and binding.type_spec is not None:
                    choose_output_types.setdefault(binding.name, binding.type_spec)
        for child in declaration.children:
            if child.kind != "choose":
                continue
            output_type = choose_output_types.get(child.name or "")
            for branch in child.properties:
                if branch.name == "when" and isinstance(branch.value, RecordValue):
                    branch_values = dict(branch.value.fields)
                    condition = branch_values.get("condition")
                    selected = branch_values.get("value")
                    if condition is not None:
                        expected_by_expression[id(condition)] = environment.language_types["bool"]
                    record_expected(selected, output_type)
                else:
                    record_expected(branch.value, output_type)
    for expression in document.parameters:
        if expression.value is None:
            continue
        expected = expected_by_expression.get(id(expression.value))
        if expected is None and isinstance(expression.value, (Reference, Identifier)):
            continue
        result, errors = infer_expression_type(expression.value, project_scope, environment, expected_type=expected)
        diagnostics.extend(errors)
        if result: facts.append(result)
    for declaration in _walk_declarations(document.declarations):
        if declaration.kind == "choose":
            continue
        for expression in _declaration_expressions(declaration):
            if id(expression) in non_value_property_ids:
                continue
            if id(expression) in deferred_record_arguments:
                continue
            expected = expected_by_expression.get(id(expression))
            if expected is None and isinstance(expression, (Reference, Identifier)):
                # Bare identifiers in configuration properties denote registry or
                # enum literals; their owning contract validators resolve them.
                continue
            result, errors = infer_expression_type(expression, scope_by_expression.get(id(expression), project_scope), environment, expected_type=expected)
            diagnostics.extend(_defer_unknown_native_members(expression, errors, document, symbols, catalogue))
            if result:
                facts.append(result)
                if id(expression) in discretization_periods:
                    seconds, unit_errors = resolve_type_spec(TypeSpec("f32", unit="s", span=discretization_periods[id(expression)]), environment)
                    diagnostics.extend(unit_errors)
                    if seconds is not None and result.resolved_type.unit_dimensions != seconds.unit_dimensions:
                        diagnostics.append(source_error(
                            "model_discretization_period_not_time",
                            "A model discretization period must have units of time.",
                            discretization_periods[id(expression)],
                        ))
    diagnostics.extend(validate_type_boundaries(document, environment))
    return TypeValidationResult(environment, tuple(facts), tuple(_deduplicate_diagnostics(diagnostics)))

def build_project_symbol_table(document: ProjectDocument, imported_contracts: tuple[ImportedSystemContract, ...] = ()) -> ProjectSymbolTable:
    """Index project signatures, bodies, symbols, and imported nominal types."""
    table = ProjectSymbolTable()
    table.project_parameters = {binding.name: Declaration("parameter", binding.name, None, bindings=(binding,), span=binding.span) for binding in document.parameters}
    table.records = {item.name: item for item in document.records}
    table.enums = {item.name: item for item in document.enums}
    table.variants = {item.name: item for item in document.variants}
    table.flags = {item.name: item for item in document.flags}
    table.fixed_point_types = {item.name: item for item in document.fixed_point_types}
    types_seen: dict[str, Any] = {}
    for collection in (table.records, table.enums, table.variants, table.flags, table.fixed_point_types):
        for name, item in collection.items():
            if name in types_seen:
                table.diagnostics.append(source_error("duplicate_project_type", f"Project type {name!r} is declared more than once.", getattr(item, "span", None)))
            types_seen[name] = item
    for contract in imported_contracts:
        for name in (*[item.name for item in contract.records], *[item.name for item in contract.enums], *[item.name for item in contract.variants], *[item.name for item in contract.flags_types], *[item.name for item in contract.fixed_point_types]):
            if name in types_seen:
                table.diagnostics.append(source_error("imported_type_collision", f"Imported type {name!r} conflicts with a project type.", getattr(types_seen[name], "span", None)))
            else:
                table.imported_types[name] = contract
                types_seen[name] = contract
    implementation = next((item for item in document.declarations if item.kind == "implementation_block"), None)
    for declaration in document.declarations:
        if declaration.prototype and declaration.name:
            if declaration.kind == "project_routine":
                qualified_name = f"{declaration.registry_key}.{declaration.name}" if declaration.registry_key else declaration.name
                if qualified_name in table.project_routines:
                    table.diagnostics.append(source_error("duplicate_project_routine", f"Project routine {qualified_name!r} is duplicated.", declaration.span))
                table.project_routines[qualified_name] = declaration
                continue
            if declaration.name in table.signatures:
                table.diagnostics.append(source_error("duplicate_component_signature", f"Component {declaration.name!r} has multiple signatures.", declaration.span))
            table.signatures[declaration.name] = declaration
            table.components[declaration.name] = declaration
            target = {"algorithm": table.algorithms, "custom_algorithm": table.custom_algorithms, "hybrid_algorithm": table.hybrid_algorithms,
                      "subsystem": table.subsystems, "model": table.models, "sensor": table.sensors, "actuator": table.actuators,
                      "protocol": table.protocols, "custom_protocol": table.custom_protocols}.get(declaration.kind)
            if target is not None: target[declaration.name] = declaration
        if declaration.kind == "operatingsystem":
            for child in declaration.children:
                if child.kind == "state_machine" and child.name:
                    if child.name in table.state_machines: table.diagnostics.append(source_error("duplicate_state_machine", f"State machine {child.name!r} is duplicated.", child.span))
                    table.state_machines[child.name] = child
    if implementation:
        for declaration in implementation.children:
            if declaration.kind == "signal" and declaration.name:
                if declaration.name in table.signals or declaration.name in table.signatures:
                    table.diagnostics.append(source_error("duplicate_project_symbol", f"Signal {declaration.name!r} conflicts with another project symbol.", declaration.span))
                table.signals[declaration.name] = declaration
            elif declaration.name:
                if declaration.name in table.implementations:
                    table.diagnostics.append(source_error("duplicate_implementation_body", f"Component {declaration.name!r} has multiple implementation bodies.", declaration.span))
                table.implementations[declaration.name] = declaration
                signature = table.signatures.get(declaration.name)
                if signature is None:
                    table.diagnostics.append(source_error("implementation_without_signature", f"Implementation {declaration.name!r} has no constructor signature.", declaration.span))
                elif signature.kind != declaration.kind:
                    table.diagnostics.append(source_error("implementation_role_mismatch", f"Implementation role {declaration.kind!r} does not match signature role {signature.kind!r}.", declaration.span))
    for name, signature in table.signatures.items():
        # A direct UETD module declaration may be fully configured in its
        # signature (or require no configuration at all). Whether it needs a
        # detailed body is decided from that module's authoring contract below.
        if name not in table.implementations and signature.kind != "module":
            table.diagnostics.append(source_error("missing_implementation", f"Component {name!r} has a signature but no implementation body.", signature.span))
    table.project_parameters.update({name: declaration for name, declaration in table.signals.items() if name not in table.project_parameters})
    return table

def validate_project_routines(symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Check that component-qualified project routines have a declared owner."""
    diagnostics: list[LanguageDiagnostic] = []
    for qualified_name, routine in symbols.project_routines.items():
        context = _value_reference_name(_property_map(routine).get("context")) or "task"
        if context not in {"task", "isr_signal_only"}:
            diagnostics.append(source_error(
                "project_routine_context_invalid",
                f"Project routine {qualified_name!r} has unsupported execution context {context!r}; use task or isr_signal_only.",
                routine.span,
            ))
        owner = routine.registry_key
        if owner is not None and owner not in symbols.signatures:
            diagnostics.append(source_error(
                "project_routine_owner_unknown",
                f"Project routine {qualified_name!r} is attached to undeclared component {owner!r}.",
                routine.span,
            ))
    return diagnostics

def validate_project(document: ProjectDocument, catalogue: AuthoringCatalogue, *, uetd_root: Path,
                    imported_contracts: tuple[ImportedSystemContract, ...] = ()) -> tuple[list[LanguageDiagnostic], SourceMap, dict[int, Any], dict[int, dict[str, int]]]:
    """Run the same deterministic semantic passes for CLI and editor requests."""
    diagnostics = validate_declaration_layout(document)
    symbols = build_project_symbol_table(document, imported_contracts)
    diagnostics.extend(symbols.diagnostics)
    diagnostics.extend(validate_contract_imports(document, imported_contracts))
    diagnostics.extend(validate_enums(document))
    diagnostics.extend(validate_project_parameters(document, symbols))
    diagnostics.extend(validate_component_bindings(document, catalogue, symbols))
    diagnostics.extend(validate_registered_algorithm_names(document, catalogue, symbols))
    diagnostics.extend(validate_dsl_builtin_expressions(document, catalogue, symbols))
    diagnostics.extend(validate_algorithm_control_flow(document, catalogue, symbols))
    diagnostics.extend(validate_models(document, catalogue, symbols))
    diagnostics.extend(validate_custom_algorithms(document, catalogue, symbols))
    diagnostics.extend(validate_project_routines(symbols))
    diagnostics.extend(validate_hybrid_algorithms(document, catalogue, symbols))
    diagnostics.extend(validate_subsystems(document, catalogue, symbols))
    diagnostics.extend(validate_state_machines(document, catalogue, symbols))
    diagnostics.extend(validate_protocols(document, catalogue, symbols))
    diagnostics.extend(validate_custom_protocols(document, catalogue, symbols))
    diagnostics.extend(validate_tasksets(document, catalogue, uetd_root=uetd_root))
    diagnostics.extend(validate_operatingsystem(document, catalogue, symbols, uetd_root=uetd_root))
    diagnostics.extend(validate_umath_calls(document, catalogue, symbols))
    diagnostics.extend(validate_numerics(document, catalogue))
    resolved_math_bindings: dict[int, Any] = {}
    resolved_math_shape_bindings: dict[int, dict[str, int]] = {}
    try:
        registry = load_language_registry()
        type_result = validate_type_interchange(document, catalogue, registry, symbols, imported_contracts)
        diagnostics.extend(type_result.diagnostics)
        resolved_math_bindings = dict(type_result.environment.resolved_math_bindings)
        resolved_math_shape_bindings = dict(type_result.environment.resolved_math_shape_bindings)
    except (OSError, ValueError, KeyError) as exc:
        diagnostics.append(source_error("type_environment_error", str(exc), document.span))
    return _deduplicate_diagnostics(diagnostics), SourceMap(), resolved_math_bindings, resolved_math_shape_bindings

def validate_contract_imports(document: ProjectDocument, imported_contracts: tuple[ImportedSystemContract, ...]) -> list[LanguageDiagnostic]:
    """Check import cardinality, selected node identity, and nominal collisions."""
    diagnostics: list[LanguageDiagnostic] = []
    if len(document.contract_imports) > 1:
        diagnostics.extend(source_error("too_many_contract_imports", "A project may import at most one .uefs contract.", item.span) for item in document.contract_imports[1:])
    if len(imported_contracts) > 1:
        diagnostics.append(source_error("ambiguous_contract_import", "Only one imported system contract can be active in a project.", document.span))
    visible: dict[str, Any] = {}
    for imported in imported_contracts:
        declarations = [*imported.records, *imported.enums, *imported.variants, *imported.flags_types, *imported.fixed_point_types]
        for item in declarations:
            if item.name in visible: diagnostics.append(source_error("duplicate_imported_type", f"Imported type {item.name!r} is duplicated.", getattr(item, "span", None)))
            visible[item.name] = item
        # Link facts are descriptive contracts. They never add callable remote members.
        for link in imported.local_links:
            if not (link.source_endpoint or link.destination_endpoint):
                diagnostics.append(source_error("invalid_imported_link", "Imported link has no local endpoint identity.", link.source_span))
    return diagnostics

def validate_project_type_declarations(document: ProjectDocument, catalogue: AuthoringCatalogue,
                                        symbols: ProjectSymbolTable, environment: TypeEnvironment) -> list[LanguageDiagnostic]:
    """Validate nominal declarations, fixed widths, fields, and local cycles."""
    diagnostics: list[LanguageDiagnostic] = []
    definitions: dict[str, Any] = {**symbols.records, **symbols.enums, **symbols.variants, **symbols.flags, **symbols.fixed_point_types}
    for record in document.records:
        field_names = [field.name for field in record.fields]
        if len(field_names) != len(set(field_names)): diagnostics.append(source_error("duplicate_record_field", f"Record {record.name!r} has duplicate fields.", record.span))
        for field in record.fields:
            if field.type_spec is None: diagnostics.append(source_error("record_field_type_required", f"Record field {field.name!r} needs a type.", field.span))
            elif field.type_spec.name == record.name: diagnostics.append(source_error("recursive_record", f"Record {record.name!r} cannot contain itself by value.", field.span))
    for enum in document.enums:
        diagnostics.extend(_validate_integer_values(enum.name, enum.storage_type, [(case.name, case.value, case.span) for case in enum.variants], enum.span))
    for variant in document.variants:
        tags = [(case.name, case.tag, case.span) for case in variant.cases]
        if not variant.cases: diagnostics.append(source_error("empty_variant", f"Variant {variant.name!r} must declare at least one case.", variant.span))
        if variant.tag_type not in {"u8", "u16", "u32", "u64"}: diagnostics.append(source_error("variant_tag_must_be_unsigned", "Variant tags use unsigned fixed-width storage.", variant.span))
        diagnostics.extend(_validate_integer_values(variant.name, variant.tag_type, tags, variant.span))
        for case in variant.cases:
            names = [field.name for field in case.fields]
            if len(names) != len(set(names)): diagnostics.append(source_error("duplicate_variant_payload_field", f"Variant case {case.name!r} repeats a payload field.", case.span))
            for field in case.fields:
                if field.type_spec and field.type_spec.name == variant.name: diagnostics.append(source_error("recursive_variant", "Recursive by-value variant payloads are not supported.", field.span))
    for flags in document.flags:
        width = _integer_width(flags.storage_type)
        used_bits: set[int] = set()
        for member in flags.members:
            if member.bit < 0 or member.bit >= width: diagnostics.append(source_error("flags_bit_out_of_range", f"Bit {member.bit} does not fit {flags.storage_type}.", member.span))
            if member.bit in used_bits: diagnostics.append(source_error("duplicate_flags_bit", f"Bit {member.bit} is assigned more than once.", member.span))
            used_bits.add(member.bit)
    for item in document.fixed_point_types:
        width = _integer_width(item.storage_type)
        if item.storage_type not in {"i8", "i16", "i32", "i64"}: diagnostics.append(source_error("fixed_point_storage_signed", "Fixed-point storage must be signed.", item.span))
        if item.fractional_bits < 0 or item.fractional_bits >= width: diagnostics.append(source_error("fixed_point_fraction_bits", "fractional_bits must be between zero and storage width minus one.", item.span))
        if item.rounding not in {"nearest_even", "toward_zero", "floor", "ceil"}: diagnostics.append(source_error("fixed_point_rounding", f"Unsupported rounding rule {item.rounding!r}.", item.span))
        if item.overflow not in {"reject", "saturate"}: diagnostics.append(source_error("fixed_point_overflow", f"Unsupported overflow policy {item.overflow!r}.", item.span))
    diagnostics.extend(validate_type_boundaries(document, environment))
    return diagnostics

def validate_declaration_layout(document: ProjectDocument) -> list[LanguageDiagnostic]:
    """Require a single target, runtime root, and trailing implementation section."""
    errors: list[LanguageDiagnostic] = []
    targets = [item for item in document.declarations if item.kind == "target"]
    numerics = [item for item in document.declarations if item.kind == "numeric"]
    systems = [item for item in document.declarations if item.kind == "operatingsystem"]
    implementations = [item for item in document.declarations if item.kind == "implementation_block"]
    if len(targets) != 1: errors.append(source_error("target_cardinality", "A project requires exactly one target declaration.", document.span))
    if len(numerics) > 1: errors.append(source_error("numeric_cardinality", "A project may declare at most one numeric profile.", numerics[1].span))
    if len(systems) != 1: errors.append(source_error("operatingsystem_cardinality", "A project requires exactly one operatingsystem declaration.", document.span))
    if len(implementations) != 1: errors.append(source_error("implementation_cardinality", "A project requires exactly one implementation block.", document.span))
    if systems and implementations and document.declarations.index(systems[0]) > document.declarations.index(implementations[0]):
        errors.append(source_error("implementation_order", "implementation must follow operatingsystem.", implementations[0].span))
    if implementations and document.declarations[-1] is not implementations[0]: errors.append(source_error("declaration_after_implementation", "implementation must be the final project section.", implementations[0].span))
    return errors

def validate_project_parameters(document: ProjectDocument, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Check project parameter declarations and ensure defaults are constant data."""
    errors: list[LanguageDiagnostic] = []
    reserved = {"project", "target", "numeric", "implementation", "operatingsystem", "map", "select", "match", "cast", "convert_unit"}
    seen: set[str] = set()
    for binding in document.parameters:
        if binding.name in reserved: errors.append(source_error("reserved_parameter_name", f"{binding.name!r} is a reserved UEF-PL identifier.", binding.span))
        if binding.name in seen: errors.append(source_error("duplicate_parameter", f"Project parameter {binding.name!r} is declared more than once.", binding.span))
        seen.add(binding.name)
        if binding.type_spec is None: errors.append(source_error("parameter_type_required", "Project parameters require an explicit type.", binding.span))
        if binding.value is not None and not _is_constant_expression(binding.value): errors.append(source_error("parameter_default_not_constant", "Project parameter defaults must be compile-time constant expressions.", binding.span))
    return errors

def validate_enums(document: ProjectDocument) -> list[LanguageDiagnostic]:
    """Resolve omitted enum values deterministically and check storage ranges."""
    diagnostics: list[LanguageDiagnostic] = []
    for enum in document.enums:
        diagnostics.extend(_validate_integer_values(enum.name, enum.storage_type, [(item.name, item.value, item.span) for item in enum.variants], enum.span))
    return diagnostics

def validate_component_bindings(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Resolve registry descriptors and check authored properties against their UETD contracts."""
    diagnostics: list[LanguageDiagnostic] = []
    for name, signature in symbols.signatures.items():
        if signature.kind in {"peripheral", "custom_algorithm", "custom_protocol", "hybrid_algorithm", "subsystem", "model"}:
            continue
        descriptor = signature.registry_key or ""
        binding = catalogue.dsl_descriptors.get(descriptor)
        if binding is None:
            # Peripheral resource names are target-owned in UETD; the selected
            # target catalogue is checked by the hardware validator.
            if signature.kind == "peripheral":
                diagnostics.append(source_warning("target_resource_resolution_deferred", f"Peripheral resource {descriptor!r} is checked against the selected target profile.", signature.span))
            else:
                diagnostics.append(source_error("unknown_uetd_descriptor", f"UETD does not register DSL descriptor {descriptor!r}.", signature.span))
            continue
        contract = binding.contract
        if contract is None:
            diagnostics.append(source_warning("uetd_descriptor_contract_missing", f"{descriptor!r} is registered as {binding.module_id!r}, but its authoring contract is incomplete ({binding.source_path}).", signature.span))
            continue
        if signature.kind != "module" and contract.kind != signature.kind:
            diagnostics.append(source_error("uetd_descriptor_kind_mismatch", f"UETD descriptor {descriptor!r} has kind {contract.kind!r}; source role is {signature.kind!r}.", signature.span))
        body = symbols.implementations.get(name)
        if body is None:
            required_properties = [item for item in contract.properties if item.get("required") is True]
            if required_properties:
                diagnostics.append(source_error(
                    "module_configuration_required",
                    f"Module {name!r} requires an implementation block because its UETD contract has required configuration properties.",
                    signature.span,
                ))
            continue
        allowed = {str(item.get("name")) for item in contract.properties if isinstance(item.get("name"), str)}
        declared = {prop.name for prop in body.properties}
        _validate_output_assignments(body, signature, contract, diagnostics)
        for prop in body.properties:
            if allowed and prop.name not in allowed:
                diagnostics.append(source_error("unknown_component_property", f"{descriptor!r} has no property {prop.name!r} in its UETD authoring contract.", prop.span))
        for item in contract.properties:
            prop_name = item.get("name")
            if item.get("required") is True and isinstance(prop_name, str) and prop_name not in declared:
                diagnostics.append(source_error("missing_component_property", f"Component {name!r} requires property {prop_name!r}.", body.span))
        status = contract.generation.get("status")
        if status not in {"ready", "implemented", "available"}:
            diagnostics.append(source_warning("uef_generation_route_incomplete", f"UETD descriptor {descriptor!r} is registered but its generation route is {status or 'unspecified'!r}.", signature.span))
    return diagnostics

def _validate_output_assignments(body: Declaration, signature: Declaration, contract: Any,
                                 diagnostics: list[LanguageDiagnostic]) -> None:
    """Require output expressions to bind one declared output port exactly once."""
    if not body.output_assignments:
        return
    output_names = {binding.name for binding in body.bindings if binding.role == "output"}
    if not output_names:
        output_names = {binding.name for binding in signature.bindings if binding.role == "output"}
    if contract is not None:
        output_names.update(str(item["name"]) for item in contract.outputs
                            if isinstance(item.get("name"), str))
    assigned: set[str] = set()
    for output in body.output_assignments:
        if output.name not in output_names:
            diagnostics.append(source_error("assignment_to_undeclared_output",
                                            f"{body.kind} {body.name!r} has no declared output {output.name!r}.",
                                            output.span))
        if output.name in assigned:
            diagnostics.append(source_error("duplicate_output_assignment",
                                            f"Output {output.name!r} is assigned more than once.", output.span))
        assigned.add(output.name)

def validate_registered_algorithm_names(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Apply the exact UETD descriptor and algorithm-kind rules."""
    diagnostics: list[LanguageDiagnostic] = []
    for signature in symbols.signatures.values():
        if signature.kind != "algorithm": continue
        descriptor = signature.registry_key or ""
        binding = catalogue.dsl_descriptors.get(descriptor)
        if binding is None: diagnostics.append(source_error("unknown_algorithm", f"UETD has no algorithm descriptor {descriptor!r}.", signature.span)); continue
        if binding.contract is None:
            diagnostics.append(source_warning("algorithm_contract_incomplete", f"Algorithm {descriptor!r} is registered but has no complete authoring contract ({binding.source_path}).", signature.span)); continue
        if binding.contract.kind != "algorithm": diagnostics.append(source_error("algorithm_kind_mismatch", f"Descriptor {descriptor!r} is registered as {binding.contract.kind!r}, not algorithm.", signature.span))
    return diagnostics

def validate_dsl_builtin_expressions(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Check language builtin names remain distinct from UETD descriptors."""
    diagnostics: list[LanguageDiagnostic] = []
    for reserved in ("map", "select", "match", "cast", "convert_unit", "has", "set", "clear"):
        if reserved in catalogue.dsl_descriptors:
            diagnostics.append(source_error("builtin_descriptor_collision", f"UETD descriptor {reserved!r} collides with a reserved UEF-PL builtin.", document.span))
    seen: set[int] = set()
    for expression in _project_expressions(document):
        for node in _walk_values(expression):
            if id(node) in seen:
                continue
            seen.add(id(node))
            if isinstance(node, MathCallExpression) and node.callee in {"map", "select"}:
                diagnostics.append(source_error("builtin_call_representation", f"{node.callee} must use its DSL-native AST form.", node.span))
    return diagnostics

def validate_algorithm_control_flow(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Reject unbounded and malformed equation control flow."""
    diagnostics: list[LanguageDiagnostic] = []
    def visit(statement: Any, multiplier: int = 1) -> None:
        if isinstance(statement, BoundedLoop):
            low = _constant_number(statement.start); high = _constant_number(statement.stop_exclusive)
            if low is None or high is None:
                diagnostics.append(source_error("nonconstant_loop_bound", "Loop bounds must be compile-time integer literals or parameters with constant defaults.", statement.span))
                count = 1
            else:
                count = high - low
                if count < 0: diagnostics.append(source_error("invalid_loop_range", "A bounded loop's exclusive stop must not be below its start.", statement.span)); count = 0
                if count > 4096: diagnostics.append(source_error("loop_bound_exceeded", "Loop iteration count exceeds the language's v1 static bound of 4096.", statement.span))
            for child in statement.body: visit(child, multiplier * count)
        elif isinstance(statement, ConditionalStatement):
            for child in (*statement.then_body, *statement.else_body): visit(child, multiplier)
    for declaration in _walk_declarations(document.declarations):
        for stage in declaration.stages:
            for statement in stage.body: visit(statement)
    return diagnostics

def validate_models(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Check model form and required dynamic-system declarations."""
    diagnostics: list[LanguageDiagnostic] = []
    for declaration in symbols.models.values():
        body = symbols.implementations.get(declaration.name or "")
        if body is None: continue
        if body.model_form not in {"linear", "equations"}:
            diagnostics.append(source_error("model_form_cardinality", "A model implementation must contain exactly one linear block or equations block.", body.span))
        if len([child for child in body.children if child.kind == "linear"]) > 1:
            diagnostics.append(source_error("duplicate_linear_model_block", "A model may contain at most one linear matrix block.", body.span))
        if len([child for child in body.children if child.kind == "jacobians"]) > 1:
            diagnostics.append(source_error("duplicate_model_jacobians", "A model may contain at most one jacobians block.", body.span))
        if declaration.signature_type == "discrete" and not any(prop.name == "sample_time" for prop in body.properties):
            diagnostics.append(source_error("model_sample_time_required", "A discrete model requires sample_time.", body.span))
        if declaration.signature_type == "continuous" and not any(prop.name == "discretize" for prop in body.properties):
            diagnostics.append(source_error("continuous_model_discretization_required", "A continuous model must name a registered discretization method and sample period before embedded execution.", body.span))
        if body.model_form == "linear":
            linear = next((child for child in body.children if child.kind == "linear"), None)
            names = set(_property_map(linear))
            required = {"A", "B"}
            if any(binding.role == "output" for binding in body.bindings):
                required.update({"C", "D"})
            missing = sorted(required - names)
            if missing:
                diagnostics.append(source_error("linear_model_matrix_missing", f"Linear model is missing required matrix/matrices: {', '.join(missing)}.", linear.span if linear else body.span))
        elif body.model_form == "equations":
            equations = [item for item in body.equations if isinstance(item, Equation)]
            if not equations:
                diagnostics.append(source_error("model_equations_empty", "An equation-based model must define at least one executable equation.", body.span))
            state_names = {binding.name for binding in body.bindings if binding.role == "state"}
            time_mode = declaration.signature_type or "continuous"
            expected_mode = "next" if time_mode == "discrete" else "derivative"
            target_names = {
                _equation_target_name(item)
                for item in equations if item.mode == expected_mode
            }
            # Record-state models commonly expose one typed derivative record
            # (`output derivative : RateRecord`) and assign its members in the
            # equation block. In that shape the record's field-level state
            # mapping is type-checked separately; require at least one member
            # assignment instead of pretending the state record is scalar.
            derivative_record = any(binding.role == "output" and binding.name == "derivative"
                                    for binding in body.bindings)
            has_derivative_record_assignment = any(_equation_target_root(item) == "derivative" for item in equations)
            missing_states = state_names - target_names
            if derivative_record and has_derivative_record_assignment:
                missing_states = set()
            if state_names and missing_states:
                diagnostics.append(source_error("model_state_equation_missing",
                                                f"{time_mode.capitalize()} model does not define {expected_mode} equations for state(s): {', '.join(sorted(missing_states))}.",
                                                body.span))
    return diagnostics

def validate_custom_algorithms(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Check custom algorithm declarations and deterministic lifecycle stages."""
    diagnostics: list[LanguageDiagnostic] = []
    permitted = {"initialize", "predict", "update", "step"}
    for signature in symbols.custom_algorithms.values():
        body = symbols.implementations.get(signature.name or "")
        if body is None: continue
        names = [stage.name for stage in body.stages]
        if len(names) != len(set(names)): diagnostics.append(source_error("duplicate_algorithm_stage", "Custom algorithm stage names must be unique.", body.span))
        unknown = set(names) - permitted
        if unknown: diagnostics.append(source_error("unknown_algorithm_stage", f"Unsupported lifecycle stage(s): {', '.join(sorted(unknown))}.", body.span))
        if not any(name in {"step", "update", "predict"} for name in names): diagnostics.append(source_error("algorithm_stage_required", "A custom algorithm needs at least one executable step, predict, or update stage.", body.span))
        binding_names = [binding.name for binding in body.bindings]
        if len(binding_names) != len(set(binding_names)): diagnostics.append(source_error("duplicate_algorithm_binding", "Algorithm input/state/output/local names must be unique.", body.span))
    return diagnostics

def validate_hybrid_algorithms(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Validate declared members, ordered stages, choice branches, and outputs."""
    return _validate_composition_symbols(symbols.hybrid_algorithms, symbols, catalogue, "hybrid_algorithm")

def validate_subsystems(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Validate subsystem member contracts and explicit output assignments."""
    return _validate_composition_symbols(symbols.subsystems, symbols, catalogue, "subsystem")

def validate_state_machines(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Check each FSM's state set, owner declaration, transitions, and callbacks."""
    diagnostics: list[LanguageDiagnostic] = []
    for machine in symbols.state_machines.values():
        allowed_properties = {"initial", "owner", "consumers"}
        property_names = [prop.name for prop in machine.properties]
        for name in sorted(set(property_names)):
            if property_names.count(name) > 1:
                diagnostics.append(source_error(
                    "duplicate_state_machine_property",
                    f"State machine {machine.name!r} repeats property {name!r}.",
                    machine.span,
                ))
        for prop in machine.properties:
            if prop.name not in allowed_properties:
                diagnostics.append(source_error(
                    "unknown_state_machine_property",
                    f"State machine {machine.name!r} does not define property {prop.name!r}; use owner and consumers for scheduling and state publication.",
                    prop.span,
                ))
        owner_properties = [prop for prop in machine.properties if prop.name == "owner"]
        if len(owner_properties) != 1 or _single_reference_name(owner_properties[0].value) is None:
            diagnostics.append(source_error(
                "state_machine_owner_required",
                f"State machine {machine.name!r} requires exactly one owner = TASK; property naming its sole scheduled evaluator.",
                machine.span,
            ))
        consumer_properties = [prop for prop in machine.properties if prop.name == "consumers"]
        if len(consumer_properties) != 1 or _reference_list_names(consumer_properties[0].value) is None:
            diagnostics.append(source_error(
                "state_machine_consumers_invalid",
                f"State machine {machine.name!r} requires exactly one consumers = [task_a, task_b]; list, or consumers = []; when it has no external readers.",
                consumer_properties[0].span if consumer_properties else machine.span,
            ))
        declared_states = [child for child in machine.children if child.kind == "machine_state"]
        if not declared_states:
            diagnostics.append(source_error(
                "state_machine_states_required",
                f"State machine {machine.name!r} must declare its states with 'state name;' before 'initial'.",
                machine.span,
            ))
            continue
        # UETD stores both counts in u8 fields and accepts capacities through
        # 255. The emitter will select shared CMake capacities from lowered
        # requirements; source models must stay within the API's hard ceiling.
        if len(declared_states) > 255:
            diagnostics.append(source_error(
                "state_machine_state_capacity_exceeded",
                f"State machine {machine.name!r} has {len(declared_states)} states; UETD supports at most 255 per machine.",
                machine.span,
            ))
        transition_rows = sum(
            len(declared_states) if child.kind == "transition" and child.name == "any"
            else 1 if child.kind == "transition" else 0
            for child in machine.children
        )
        if transition_rows > 255:
            diagnostics.append(source_error(
                "state_machine_transition_capacity_exceeded",
                f"State machine {machine.name!r} lowers to {transition_rows} transition rows after expanding 'any'; UETD supports at most 255 per machine.",
                machine.span,
            ))
        enum = _enum_definition(machine.registry_key or "", symbols)
        if enum is None:
            diagnostics.append(source_error("state_machine_type_missing", f"State machine {machine.name!r} has no generated state type {machine.registry_key!r}.", machine.span)); continue
        states = {item.name for item in enum.variants}
        initial_properties = [prop for prop in machine.properties if prop.name == "initial"]
        if len(initial_properties) != 1:
            diagnostics.append(source_error("state_machine_initial_count", f"State machine {machine.name!r} requires exactly one initial state.", machine.span))
        initial = initial_properties[0].value if initial_properties else None
        initial_name = _value_reference_name(initial)
        if initial_name and initial_name.rsplit(".", 1)[-1] not in states: diagnostics.append(source_error("invalid_initial_state", f"Initial state {initial_name!r} is not in {enum.name}.", machine.span))
        for transition in machine.children:
            if transition.kind == "transition":
                if transition.name != "any" and transition.name not in states: diagnostics.append(source_error("unknown_transition_source", f"Unknown source state {transition.name!r}.", transition.span))
                if transition.registry_key not in states: diagnostics.append(source_error("unknown_transition_destination", f"Unknown destination state {transition.registry_key!r}.", transition.span))
                if transition.name == "any":
                    diagnostics.append(source_warning(
                        "state_machine_any_transition_reentry",
                        "An 'any' transition also matches its destination state. If its guard remains true there, each evaluation exits and re-enters that state; exclude the destination in the guard when repeated entry actions are not intended.",
                        transition.span,
                    ))
            elif transition.kind == "state_action":
                if transition.name not in states:
                    diagnostics.append(source_error("unknown_state_action_state", f"State action refers to unknown state {transition.name!r}.", transition.span))
                    continue
                operation = next((prop.value for prop in transition.properties if prop.name == "operation"), None)
                if not isinstance(operation, Reference) or len(operation.path) != 2:
                    diagnostics.append(source_error("state_action_operation_invalid", "State actions use exactly one component.operation path.", transition.span))
                    continue
                owner_name, operation_name = operation.path
                owner = symbols.signatures.get(owner_name)
                descriptor = catalogue.dsl_descriptors.get(owner.registry_key or "") if owner else None
                contract = descriptor.contract if descriptor is not None else None
                if owner is None:
                    diagnostics.append(source_error("state_action_component_unknown", f"State action names undeclared component {owner_name!r}.", operation.span))
                elif f"{owner_name}.{operation_name}" in symbols.project_routines:
                    if contract is not None and operation_name in {str(item.get("id")) for item in contract.operations}:
                        diagnostics.append(source_error(
                            "project_routine_conflicts_with_uetd_operation",
                            f"{owner_name}.{operation_name} is declared both as a project routine and a UETD operation; keep one owner for this call.",
                            operation.span,
                        ))
                    else:
                        routine = symbols.project_routines[f"{owner_name}.{operation_name}"]
                        if (routine.signature_type or "void()").split("(", 1)[0] != "status":
                            diagnostics.append(source_error(
                                "state_action_project_routine_status_required",
                                f"Project routine {owner_name}.{operation_name} is an FSM callback and must be declared with 'returns status' so callback failures latch the machine fault.",
                                operation.span,
                            ))
                        routine_context = _value_reference_name(_property_map(routine).get("context")) or "task"
                        if routine_context != "task":
                            diagnostics.append(source_error(
                                "state_action_project_routine_context_invalid",
                                f"FSM callback {owner_name}.{operation_name} must run in task context; ISR callbacks may only signal deferred work.",
                                operation.span,
                            ))
                elif contract is None:
                    diagnostics.append(source_warning("state_action_contract_missing", f"UETD has no complete operation contract for {owner_name!r}; declare 'project routine {owner_name}.{operation_name}();' if this is project-owned.", operation.span))
                elif operation_name not in {str(item.get("id")) for item in contract.operations}:
                    if not contract.operations and contract.generation.get("status") not in {"ready", "implemented", "available"}:
                        diagnostics.append(source_warning("state_action_contract_missing", f"UETD has not published operations for draft component {owner_name!r}; declare 'project routine {owner_name}.{operation_name}();' if this is project-owned.", operation.span))
                    else:
                        diagnostics.append(source_error("state_action_operation_unknown", f"Component {owner_name!r} has no registered operation {operation_name!r}.", operation.span))
                else:
                    operation_contract = next(item for item in contract.operations if str(item.get("id")) == operation_name)
                    if operation_contract.get("result") != "status":
                        diagnostics.append(source_error(
                            "state_action_operation_status_required",
                            f"UETD operation {owner_name}.{operation_name} must publish result = status to be used as a state hook.",
                            operation.span,
                        ))
                    if operation_contract.get("arguments"):
                        diagnostics.append(source_error(
                            "state_action_operation_arguments_unsupported",
                            f"FSM state actions currently call zero-argument operations, but {owner_name}.{operation_name} requires operation arguments.",
                            operation.span,
                        ))
                    contexts = set(contract.scheduling.get("contexts", ()))
                    if "task" not in contexts:
                        diagnostics.append(source_error(
                            "state_action_operation_task_context_unsupported",
                            f"UETD component {owner_name!r} does not publish task-context support for operation {operation_name!r}.",
                            operation.span,
                        ))
    return diagnostics

def validate_protocols(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Validate native protocol descriptors and then custom wire declarations."""
    diagnostics: list[LanguageDiagnostic] = []
    for signature in symbols.protocols.values():
        binding = catalogue.dsl_descriptors.get(signature.registry_key or "")
        if binding is None: diagnostics.append(source_error("unknown_protocol_descriptor", f"UETD has no protocol descriptor {signature.registry_key!r}.", signature.span))
        elif binding.contract is None: diagnostics.append(source_warning("protocol_contract_incomplete", f"Protocol descriptor {binding.descriptor!r} has no complete authoring contract.", signature.span))
        elif binding.contract.kind != "protocol": diagnostics.append(source_error("protocol_kind_mismatch", f"Descriptor {binding.descriptor!r} is not a protocol module.", signature.span))
    return diagnostics

def validate_custom_protocols(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Prove custom message IDs, payload extents, field encodings, and mappings."""
    diagnostics: list[LanguageDiagnostic] = []
    for signature in symbols.custom_protocols.values():
        body = symbols.implementations.get(signature.name or "")
        if body is None: continue
        messages = [child for child in body.children if child.kind == "message"]
        names = [item.name for item in messages]
        if len(names) != len(set(names)): diagnostics.append(source_error("duplicate_protocol_message", "Protocol message names must be unique.", body.span))
        if not messages: diagnostics.append(source_error("protocol_message_required", "A custom protocol must declare at least one message.", body.span))
        transport = _value_reference_name(_property_map(body).get("transport"))
        if transport and transport.split(".", 1)[0] not in symbols.signatures:
            diagnostics.append(source_error("unknown_protocol_transport", f"Custom protocol {signature.name!r} names undeclared transport {transport!r}.", body.span))
        ids_seen: set[int] = set()
        for message in messages:
            props = _property_map(message)
            message_id = _literal_int(props.get("id"))
            if message_id is None:
                diagnostics.append(source_error("protocol_message_id_required", f"Message {message.name!r} requires an explicit integer ID.", message.span))
            elif not 0 <= message_id <= 0xFFFF:
                diagnostics.append(source_error("protocol_message_id_range", f"Message ID {message_id} must fit u16.", message.span))
            elif message_id in ids_seen:
                diagnostics.append(source_error("duplicate_message_id", f"Protocol {signature.name!r} repeats message ID {message_id}.", message.span))
            else:
                ids_seen.add(message_id)
            payload_size = _protocol_quantity(props.get("payload_bytes"), "B")
            if payload_size is None or payload_size <= 0 or int(payload_size) != payload_size:
                diagnostics.append(source_error("protocol_payload_size_required", f"Message {message.name!r} requires a positive integral payload_bytes value.", message.span))
            logical_type = _value_reference_name(props.get("logical_type"))
            raw_payload = isinstance(props.get("raw_payload"), Literal) and props["raw_payload"].value is True
            if bool(logical_type) == raw_payload:
                diagnostics.append(source_error("protocol_payload_contract_required", f"Message {message.name!r} must declare exactly one of logical_type or raw_payload = true.", message.span))
            if logical_type and _record_definition(logical_type, symbols) is None:
                diagnostics.append(source_error("protocol_logical_type_unknown", f"Message logical_type {logical_type!r} must name a project record.", message.span))
            direct_fields = [child for child in message.children if child.kind == "field"]
            fragments = [child for child in message.children if child.kind == "fragment"]
            if direct_fields and fragments:
                diagnostics.append(source_error("protocol_layout_mixed", f"Message {message.name!r} cannot mix flat fields with fragments.", message.span))
            all_fields = [*direct_fields, *(field for fragment in fragments for field in fragment.children if field.kind == "field")]
            fields_names = [child.name for child in all_fields]
            if len(fields_names) != len(set(fields_names)): diagnostics.append(source_error("duplicate_protocol_field", "Protocol field names must be unique within a message.", message.span))
            if direct_fields:
                diagnostics.extend(_validate_protocol_fields(direct_fields, message.name or "", logical_type,
                                                             payload_size, symbols))
            for fragment in fragments:
                fragment_fields = [field for field in fragment.children if field.kind == "field"]
                fragment_size = _protocol_quantity(_property_map(fragment).get("payload_bytes"), "B")
                bound = fragment_size if fragment_size is not None else None
                diagnostics.extend(_validate_protocol_fields(fragment_fields, message.name or "",
                                                             logical_type, bound, symbols,
                                                             layout=f"fragment {fragment.name}"))
            protocol_flags = [child for child in message.children if child.kind == "protocol_flag"]
            flag_names = [child.name for child in protocol_flags]
            if len(flag_names) != len(set(flag_names)):
                diagnostics.append(source_error("duplicate_protocol_flag", f"Message {message.name!r} repeats a derived flag name.", message.span))
            fields_by_name = {field.name: field for field in all_fields}
            for flag in protocol_flags:
                source_field = fields_by_name.get(flag.registry_key or "")
                if source_field is None:
                    diagnostics.append(source_error("protocol_flag_field_unknown", f"Derived flag {flag.name!r} names unknown field {flag.registry_key!r}.", flag.span))
                    continue
                bit = int(flag.signature_type or "-1")
                width = _protocol_field_width_bits(source_field, _property_map(source_field), symbols)
                if width is None or bit < 0 or bit >= width:
                    diagnostics.append(source_error("protocol_flag_bit_range", f"Derived flag {flag.name!r} bit {bit} is outside field {source_field.name!r}.", flag.span))
            if raw_payload and all_fields:
                diagnostics.append(source_error("raw_protocol_has_typed_fields", f"Raw-payload message {message.name!r} cannot also declare typed fields.", message.span))
        for state_machine in (child for child in body.children if child.kind == "state_machine"):
            states = [child.name for child in state_machine.children if child.kind == "protocol_state"]
            if not states:
                diagnostics.append(source_error("protocol_state_required", "A protocol parser state machine must declare at least one state.", state_machine.span))
            if len(states) != len(set(states)):
                diagnostics.append(source_error("duplicate_protocol_state", "Protocol parser state names must be unique.", state_machine.span))
            state_set = set(states)
            for transition in (child for child in state_machine.children if child.kind == "protocol_transition"):
                if transition.registry_key not in state_set:
                    diagnostics.append(source_error("protocol_transition_source_unknown", f"Transition {transition.name!r} starts from unknown state {transition.registry_key!r}.", transition.span))
                if transition.signature_type not in state_set:
                    diagnostics.append(source_error("protocol_transition_destination_unknown", f"Transition {transition.name!r} ends at unknown state {transition.signature_type!r}.", transition.span))
    seen_umath: set[int] = set()
    for expression, context in _expressions_with_context(document):
        if context != "protocol":
            continue
        for node in _walk_values(expression):
            if isinstance(node, MathCallExpression) and id(node) not in seen_umath:
                seen_umath.add(id(node))
                diagnostics.append(source_error("umath_forbidden_in_protocol", "UMATH calls are not allowed in protocol layout contracts.", node.span))
    return diagnostics

def _validate_protocol_fields(fields: list[Declaration], message_name: str, logical_type: str | None,
                              payload_size: int | float | None, symbols: ProjectSymbolTable,
                              *, layout: str = "message") -> list[LanguageDiagnostic]:
    diagnostics: list[LanguageDiagnostic] = []
    occupied: list[tuple[int, int, str]] = []
    for field in fields:
        field_props = _property_map(field)
        offset = _protocol_quantity(field_props.get("offset"), "B")
        bit = _protocol_bit(field_props.get("bit"))
        if offset is None or offset < 0 or int(offset) != offset:
            diagnostics.append(source_error("protocol_field_offset_required", f"Field {field.name!r} needs an explicit nonnegative integral offset in bytes.", field.span))
            continue
        if bit is not None and (bit < 0 or int(bit) != bit or bit >= 8):
            diagnostics.append(source_error("protocol_field_bit_range", f"Field {field.name!r} bit must be in [0, 7].", field.span))
            continue
        width_bits = _protocol_field_width_bits(field, field_props, symbols)
        if width_bits is None or width_bits <= 0:
            diagnostics.append(source_error("protocol_field_width_unresolved", f"Field {field.name!r} needs a fixed-width encoding the compiler can size.", field.span))
            continue
        start = int(offset) * 8 + int(bit or 0)
        end = start + width_bits
        if payload_size is not None and end > int(payload_size) * 8:
            diagnostics.append(source_error("protocol_field_out_of_bounds", f"Field {field.name!r} extends beyond its {int(payload_size)}-byte {layout} in message {message_name!r}.", field.span))
        for old_start, old_end, old_name in occupied:
            if start < old_end and old_start < end:
                diagnostics.append(source_error("protocol_field_overlap", f"Fields {old_name!r} and {field.name!r} overlap within {layout} of message {message_name!r}.", field.span))
        occupied.append((start, end, field.name or "<unnamed>"))
        encoding = _value_reference_name(field_props.get("encoding") or field_props.get("wire_encoding"))
        if not encoding:
            diagnostics.append(source_error("protocol_field_encoding_required", f"Field {field.name!r} needs an explicit encoding or wire_encoding.", field.span))
        application_field = _value_reference_name(field_props.get("application_field"))
        protocol_internal = isinstance(field_props.get("protocol_internal"), Literal) and field_props["protocol_internal"].value is True
        if not application_field and not protocol_internal:
            diagnostics.append(source_error("protocol_field_mapping_required", f"Field {field.name!r} must map to an application_field or declare protocol_internal = true.", field.span))
        if logical_type and application_field and not protocol_internal:
            app_parts = application_field.split(".")
            if len(app_parts) < 2 or app_parts[0] != logical_type:
                diagnostics.append(source_error("protocol_field_mapping_owner", f"Field {field.name!r} must map into logical_type {logical_type!r}.", field.span))
            elif logical_type and (record_definition := _record_definition(logical_type, symbols)) is not None:
                record_fields = ({item.name for item in record_definition.fields}
                                 if logical_type in symbols.records
                                 else {item[0] for item in record_definition.fields})
                if app_parts[1] not in record_fields:
                    diagnostics.append(source_error("protocol_field_mapping_unknown", f"Application field {application_field!r} is not declared by record {logical_type!r}.", field.span))
        if _encoding_is_integer(encoding) and ("scale" in field_props or "offset_value" in field_props):
            if "rounding" not in field_props or "overflow" not in field_props:
                diagnostics.append(source_error("protocol_quantization_policy_required", f"Scaled integer field {field.name!r} must declare rounding and overflow policies.", field.span))
    return diagnostics

def _literal_int(value: Any) -> int | None:
    if isinstance(value, Literal) and isinstance(value.value, int) and not isinstance(value.value, bool):
        return value.value
    return None

def _protocol_quantity(value: Any, expected_unit: str) -> int | float | None:
    if not isinstance(value, Literal) or not isinstance(value.value, (int, float)) or isinstance(value.value, bool):
        return None
    if value.unit != expected_unit:
        return None
    return value.value

def _protocol_bit(value: Any) -> int | None:
    if not isinstance(value, Literal) or not isinstance(value.value, int) or isinstance(value.value, bool):
        return None
    return value.value if value.unit in {None, "bit"} else None

def _protocol_field_width_bits(field: Declaration, props: dict[str, Any], symbols: ProjectSymbolTable) -> int | None:
    width = props.get("width")
    if isinstance(width, Literal) and isinstance(width.value, (int, float)) and not isinstance(width.value, bool):
        if width.unit == "bit" and int(width.value) == width.value:
            return int(width.value)
        if width.unit == "B" and int(width.value) == width.value:
            return int(width.value) * 8
    encoding = _value_reference_name(props.get("encoding") or props.get("wire_encoding")) or ""
    encoding_lower = encoding.lower()
    if "bool_u8" in encoding_lower:
        scalar_bits = 8
    elif "binary64" in encoding_lower:
        scalar_bits = 64
    elif "binary32" in encoding_lower:
        scalar_bits = 32
    else:
        match = re.search(r"(?:^|_)(?:signed_|unsigned_)?(?:i|u)(8|16|32|64)(?:_|$)", encoding_lower)
        if match:
            scalar_bits = int(match.group(1))
        else:
            type_spec = field.bindings[0].type_spec if field.bindings else None
            if type_spec is None:
                return None
            name = type_spec.name
            if name == "bool": scalar_bits = 8
            elif name in {"f32", "f64", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64"}:
                scalar_bits = int(name[1:])
            elif (enum_definition := _enum_definition(name, symbols)) is not None:
                scalar_bits = _integer_width(enum_definition.storage_type)
            elif (flags_definition := _flags_definition(name, symbols)) is not None:
                scalar_bits = _integer_width(flags_definition.storage_type)
            else:
                return None
    type_spec = field.bindings[0].type_spec if field.bindings else None
    if type_spec is None:
        return scalar_bits
    count = 1
    for dimension in type_spec.shape:
        if not isinstance(dimension, int) or dimension <= 0:
            return None
        count *= dimension
    return scalar_bits * count

def _imported_definition(name: str, symbols: ProjectSymbolTable, collection_name: str) -> Any | None:
    """Return a shared declaration from the one imported .uefs contract."""
    contract = symbols.imported_types.get(name)
    if contract is None:
        return None
    return next((item for item in getattr(contract, collection_name, ()) if getattr(item, "name", None) == name), None)

def _record_definition(name: str, symbols: ProjectSymbolTable) -> Any | None:
    return symbols.records.get(name) or _imported_definition(name, symbols, "records")

def _enum_definition(name: str, symbols: ProjectSymbolTable) -> Any | None:
    return symbols.enums.get(name) or _imported_definition(name, symbols, "enums")

def _flags_definition(name: str, symbols: ProjectSymbolTable) -> Any | None:
    return symbols.flags.get(name) or _imported_definition(name, symbols, "flags_types")

def _encoding_is_integer(encoding: str | None) -> bool:
    if not encoding:
        return False
    name = encoding.lower()
    return "binary32" not in name and "binary64" not in name and any(token in name for token in ("i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "integer"))

def validate_tasksets(document: ProjectDocument, catalogue: AuthoringCatalogue, *, uetd_root: Path) -> list[LanguageDiagnostic]:
    """Check scheduled declarations have bounded timing and coherent grouping."""
    del catalogue, uetd_root
    diagnostics: list[LanguageDiagnostic] = []
    os_node = next((item for item in document.declarations if item.kind == "operatingsystem"), None)
    if os_node is None: return diagnostics
    for group in [child for child in os_node.children if child.kind == "taskset"]:
        names = [child.name for child in group.children if child.kind == "task"]
        if len(names) != len(set(names)): diagnostics.append(source_error("duplicate_task", f"Task names in taskset {group.name!r} must be unique.", group.span))
        props = {prop.name for prop in group.properties}
        if not props.intersection({"core", "scheduler", "memory", "channel_policy", "resource_owner", "interrupt_owner"}):
            diagnostics.append(source_warning("taskset_has_no_grouping_policy", f"Taskset {group.name!r} does not declare a shared scheduling, resource, or memory policy.", group.span))
    return diagnostics

def validate_operatingsystem(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable, *, uetd_root: Path) -> list[LanguageDiagnostic]:
    """Resolve schedules, single-owner FSM execution, and explicit channel subscribers."""
    del uetd_root
    diagnostics: list[LanguageDiagnostic] = []
    roots = [item for item in document.declarations if item.kind == "operatingsystem"]
    if len(roots) != 1: return diagnostics
    root = roots[0]
    runtime_properties = [p for p in root.properties if p.name == "runtime"]
    runtime = _single_reference_name(runtime_properties[0].value) if len(runtime_properties) == 1 else None
    if runtime is None:
        diagnostics.append(source_error("runtime_required", "operatingsystem requires exactly one runtime = RuntimeName; property.", runtime_properties[0].span if runtime_properties else root.span))
    elif runtime not in catalogue.dsl_descriptors and runtime not in catalogue.runtimes:
        diagnostics.append(source_error("unknown_runtime", f"UETD has no registered runtime descriptor {runtime!r}.", root.span))
    failure_policy_properties = [p for p in root.properties if p.name == "component_failure_policy"]
    if len(failure_policy_properties) != 1 or _single_reference_name(failure_policy_properties[0].value) is None:
        diagnostics.append(source_error(
            "component_failure_policy_required",
            "operatingsystem requires exactly one component_failure_policy = POLICY; so task, loop, and FSM callback failures have an explicit system response.",
            failure_policy_properties[0].span if failure_policy_properties else root.span,
        ))
    tasks = [child for child in root.children if child.kind == "task"]
    loops = [child for child in root.children if child.kind == "loop"]
    tasksets = [child for child in root.children if child.kind == "taskset"]
    runtime_name = runtime or ""
    bare = runtime_name.lower() in {"baremetal", "bare_metal"}
    scheduled = [item for item in _walk_declarations(root.children) if item.kind in {"task", "loop"}]
    scheduled_by_name: dict[str, Declaration] = {}
    for item in scheduled:
        if not item.name:
            continue
        if item.name in scheduled_by_name:
            diagnostics.append(source_error(
                "duplicate_scheduled_context",
                f"Scheduled task/loop name {item.name!r} must be unique across the operating system and all tasksets.",
                item.span,
            ))
        scheduled_by_name[item.name] = item
    task_names = {item.name for item in scheduled if item.kind == "task" and item.name}
    loop_names = {item.name for item in scheduled if item.kind == "loop" and item.name}
    channel_declarations = [item for item in _walk_declarations(root.children) if item.kind == "channel"]
    channel_consumers: dict[str, tuple[str, ...]] = {}
    channel_inputs: dict[str, dict[str, set[str]]] = {}
    channel_names: set[str] = set()
    channel_properties = {
        "source", "destination", "producer", "consumer", "consumers", "capacity", "overflow",
        "maximum_age", "ownership", "memory_region", "publish_when", "cache_policy",
        "barrier_policy", "notification", "clock_policy", "alignment",
    }
    for channel in channel_declarations:
        if channel.name in channel_names:
            diagnostics.append(source_error("duplicate_channel", f"Channel name {channel.name!r} must be unique within the operating system.", channel.span))
        if channel.name:
            channel_names.add(channel.name)
        properties = _property_map(channel)
        property_names = [prop.name for prop in channel.properties]
        for name in sorted(set(property_names)):
            if property_names.count(name) > 1:
                diagnostics.append(source_error("duplicate_channel_property", f"Channel {channel.name!r} repeats property {name!r}.", channel.span))
        for prop in channel.properties:
            if prop.name not in channel_properties:
                diagnostics.append(source_error("unknown_channel_property", f"Channel {channel.name!r} has no declared property {prop.name!r}.", prop.span))
        if "consumer" in properties:
            diagnostics.append(source_error(
                "channel_consumer_is_list",
                f"Channel {channel.name!r} must declare consumers = [task_a, task_b]; so delivery targets are explicit and can fan out safely.",
                next((prop.span for prop in channel.properties if prop.name == "consumer"), channel.span),
            ))
        source = properties.get("source")
        if not isinstance(source, Reference) or len(source.path) < 2:
            diagnostics.append(source_error("channel_source_required", f"Channel {channel.name!r} requires a component/endpoint output source path.", channel.span))
        producer = _single_reference_name(properties.get("producer"))
        if producer is None:
            diagnostics.append(source_error("channel_producer_required", f"Channel {channel.name!r} requires producer = TASK;.", channel.span))
        elif producer not in scheduled_by_name:
            diagnostics.append(source_error("channel_producer_unknown", f"Channel {channel.name!r} names undeclared producer task/loop {producer!r}.", channel.span))
        consumers = _reference_list_names(properties.get("consumers"))
        if consumers is None or not consumers:
            diagnostics.append(source_error("channel_consumers_required", f"Channel {channel.name!r} requires a non-empty consumers = [TASK, ...]; list.", channel.span))
            consumers = ()
        elif len(consumers) != len(set(consumers)):
            diagnostics.append(source_error("channel_consumers_duplicate", f"Channel {channel.name!r} repeats a subscriber; list each receiving task once.", channel.span))
        for consumer in consumers:
            if consumer not in scheduled_by_name:
                diagnostics.append(source_error("channel_consumer_unknown", f"Channel {channel.name!r} names undeclared consumer task/loop {consumer!r}.", channel.span))
        channel_consumers[channel.name or ""] = tuple(consumers)
        if "capacity" not in properties or _statically_nonpositive_integer(properties.get("capacity")):
            diagnostics.append(source_error("channel_capacity_invalid", f"Channel {channel.name!r} requires a positive bounded capacity.", channel.span))
        for required in ("overflow", "ownership"):
            if required not in properties or _single_reference_name(properties.get(required)) is None:
                diagnostics.append(source_error("channel_policy_required", f"Channel {channel.name!r} requires {required} = POLICY;.", channel.span))
        destination = _value_reference_name(properties.get("destination"))
        if destination and "." in destination:
            destination_parts = destination.split(".")
            if len(destination_parts) == 2 and destination_parts[1] == "trigger":
                if destination_parts[0] not in consumers:
                    diagnostics.append(source_error("channel_trigger_consumer_required", f"Channel {channel.name!r} trigger destination task must be listed in consumers.", channel.span))
            elif len(destination_parts) == 2:
                component, port = destination_parts
                matching = []
                for consumer in consumers:
                    scheduled_consumer = scheduled_by_name.get(consumer)
                    run = next((child for child in scheduled_consumer.children if child.kind == "run"), None) if scheduled_consumer else None
                    if run is not None and (run.name or "").split(".", 1)[0] == component:
                        matching.append(consumer)
                if len(matching) != 1:
                    diagnostics.append(source_error(
                        "channel_component_destination_consumer",
                        f"Channel {channel.name!r} component input destination must match exactly one subscribed task that runs {component!r}; other subscribers can read channel.value independently.",
                        channel.span,
                    ))
                elif matching:
                    channel_inputs.setdefault(matching[0], {}).setdefault(component, set()).add(port)
            elif len(destination_parts) == 3 and destination_parts[1] == "trigger":
                if destination_parts[0] not in consumers:
                    diagnostics.append(source_error("channel_trigger_consumer_required", f"Channel {channel.name!r} trigger destination task must be listed in consumers.", channel.span))
            elif len(destination_parts) < 3:
                diagnostics.append(source_error("channel_destination_invalid", f"Channel {channel.name!r} destination must name a component input, task trigger, or endpoint send message.", channel.span))
    if bare and (len(loops) != 1 or tasks or tasksets):
        diagnostics.append(source_error("baremetal_schedule_shape", "BareMetal requires exactly one loop and cannot contain RTOS tasks or tasksets.", root.span))
    if not bare and loops:
        diagnostics.append(source_error("rtos_loop_forbidden", "RTOS runtimes use tasks; loop declarations are reserved for BareMetal.", loops[0].span))
    if not bare and not tasks and not tasksets:
        diagnostics.append(source_error("rtos_task_required", "An RTOS operating system must declare at least one task.", root.span))
    machines = [child for child in root.children if child.kind == "state_machine"]
    machine_owners: dict[str, str] = {}
    machine_consumers: dict[str, tuple[str, ...]] = {}
    for machine in machines:
        properties = _property_map(machine)
        owner = _single_reference_name(properties.get("owner"))
        consumers = _reference_list_names(properties.get("consumers")) or ()
        if machine.name:
            machine_owners[machine.name] = owner or ""
            machine_consumers[machine.name] = tuple(consumers)
        owner_context = scheduled_by_name.get(owner or "")
        if owner_context is None:
            diagnostics.append(source_error("state_machine_owner_unknown", f"State machine {machine.name!r} owner {owner!r} must name one declared task/loop.", machine.span))
        elif bare and owner_context.kind != "loop":
            diagnostics.append(source_error("baremetal_state_machine_owner", f"BareMetal state machine {machine.name!r} must be owned by the sole main loop.", machine.span))
        elif not bare and owner_context.kind != "task":
            diagnostics.append(source_error("rtos_state_machine_owner", f"RTOS state machine {machine.name!r} must be owned by a task, not a loop.", machine.span))
        if owner_context is not None:
            owner_properties = _property_map(owner_context)
            if "when" in owner_properties:
                diagnostics.append(source_error("state_machine_owner_may_not_be_gated", f"Owner {owner!r} cannot be conditional; the FSM must receive every scheduled tick until shutdown or explicit fault handling.", owner_context.span))
            if owner_context.kind == "task" and not any(key in owner_properties for key in ("rate", "period", "trigger")):
                diagnostics.append(source_error("state_machine_owner_schedule_required", f"FSM owner task {owner!r} requires a rate/period or trigger so the runtime defines its evaluation boundary.", owner_context.span))
        if owner in consumers:
            diagnostics.append(source_error("state_machine_owner_is_implicit_consumer", f"State machine {machine.name!r} owner {owner!r} already reads its own state and must not be repeated in consumers.", machine.span))
        if bare and consumers:
            diagnostics.append(source_error("baremetal_state_machine_consumers_forbidden", f"BareMetal FSM {machine.name!r} has one loop context and cannot subscribe other tasks; omit consumers.", machine.span))
        for consumer in consumers:
            if consumer not in task_names:
                diagnostics.append(source_error("state_machine_consumer_unknown", f"State machine {machine.name!r} consumer {consumer!r} must name a declared RTOS task.", machine.span))
    for item in scheduled:
        runs = [child for child in item.children if child.kind == "run"]
        if len(runs) != 1:
            diagnostics.append(source_error("scheduled_work_run_count", f"{item.kind.capitalize()} {item.name!r} requires exactly one run = invocation(...); clause.", item.span)); continue
        invocation = runs[0]
        target = invocation.name or ""
        if target not in symbols.signatures and target.split(".", 1)[0] not in symbols.signatures and target not in symbols.signals:
            diagnostics.append(source_error("unknown_run_target", f"Scheduled work refers to undeclared component {target!r}.", invocation.span))
        diagnostics.extend(_validate_invocation_arguments(
            invocation,
            symbols,
            catalogue,
            channel_inputs.get(item.name or "", {}).get(target.split(".", 1)[0], set()),
        ))
        if item.kind == "loop" and not any(prop.name in {"rate", "period", "free_running"} for prop in item.properties):
            diagnostics.append(source_error("loop_timing_required", "A BareMetal loop must declare a rate, period, or explicit free_running policy.", item.span))
        for reference in _references(item):
            if len(reference.path) < 2:
                continue
            root_name, member = reference.path[:2]
            if root_name in machine_owners and member == "state":
                owner = machine_owners[root_name]
                if item.name != owner and item.name not in machine_consumers.get(root_name, ()):
                    diagnostics.append(source_error("state_machine_consumer_not_declared", f"Task/loop {item.name!r} reads {root_name}.state but is not its owner or a declared FSM consumer.", reference.span))
            if root_name in channel_consumers and member in {"value", "not_empty", "dropped_count"}:
                if item.name not in channel_consumers[root_name]:
                    diagnostics.append(source_error("channel_consumer_not_declared", f"Task/loop {item.name!r} reads channel {root_name!r} but is not listed in consumers.", reference.span))
    for machine in machines:
        owner = machine_owners.get(machine.name or "", "")
        for transition in (child for child in machine.children if child.kind == "transition"):
            guard = next((prop.value for prop in transition.properties if prop.name == "when"), None)
            for reference in _references(guard):
                if len(reference.path) < 2:
                    continue
                root_name, member = reference.path[:2]
                if root_name in machine_owners and member == "state":
                    other_owner = machine_owners[root_name]
                    if root_name != machine.name and owner != other_owner and owner not in machine_consumers.get(root_name, ()):
                        diagnostics.append(source_error("state_machine_consumer_not_declared", f"FSM owner {owner!r} reads {root_name}.state but is not its owner or a declared consumer.", reference.span))
                if root_name in channel_consumers and member in {"value", "not_empty", "dropped_count"}:
                    if owner not in channel_consumers[root_name]:
                        diagnostics.append(source_error("channel_consumer_not_declared", f"FSM owner {owner!r} reads channel {root_name!r} but is not listed in consumers.", reference.span))
    for interrupt in _walk_declarations(root.children):
        if interrupt.kind != "interrupt":
            continue
        interrupt_properties = _property_map(interrupt)
        handler = interrupt_properties.get("handler")
        interrupt_context = _value_reference_name(interrupt_properties.get("context"))
        owner_name: str | None = None
        operation_name: str | None = None
        project_routine_key: str | None = None
        if isinstance(handler, Reference) and len(handler.path) == 2:
            owner_name, operation_name = handler.path
            project_routine_key = f"{owner_name}.{operation_name}"
        elif isinstance(handler, Reference) and len(handler.path) == 1:
            project_routine_key = handler.path[0]
            if any(item.kind in {"task", "loop"} and item.name == project_routine_key for item in _walk_declarations(root.children)):
                if interrupt_context != "isr_signal_only":
                    diagnostics.append(source_error("interrupt_deferred_handler_context", "A task or loop used as an interrupt handler requires context = isr_signal_only so the ISR only signals deferred work.", handler.span))
                continue
        elif isinstance(handler, Identifier):
            project_routine_key = handler.value
            if any(item.kind in {"task", "loop"} and item.name == handler.value for item in _walk_declarations(root.children)):
                if interrupt_context != "isr_signal_only":
                    diagnostics.append(source_error("interrupt_deferred_handler_context", "A task or loop used as an interrupt handler requires context = isr_signal_only so the ISR only signals deferred work.", handler.span))
                continue
        if project_routine_key is None:
            diagnostics.append(source_error(
                "interrupt_handler_invalid",
                "An interrupt handler must name a declared component.operation, project routine, or task/loop deferred target.",
                interrupt.span,
            ))
            continue
        project_routine = symbols.project_routines.get(project_routine_key)
        if project_routine is not None:
            owner_binding = (catalogue.dsl_descriptors.get(symbols.signatures[owner_name].registry_key or "")
                             if owner_name in symbols.signatures else None)
            owner_contract = owner_binding.contract if owner_binding is not None else None
            if (operation_name is not None and owner_contract is not None
                    and operation_name in {str(item.get("id")) for item in owner_contract.operations}):
                diagnostics.append(source_error(
                    "project_routine_conflicts_with_uetd_operation",
                    f"{project_routine_key} is declared both as a project routine and a UETD operation; keep one owner for this call.",
                    handler.span,
                ))
                continue
            routine_context = _value_reference_name(_property_map(project_routine).get("context")) or "task"
            if interrupt_context != "isr_signal_only" or routine_context != "isr_signal_only":
                diagnostics.append(source_error(
                    "interrupt_project_routine_context",
                    f"Project routine {project_routine_key!r} must declare context = isr_signal_only and the interrupt must use the same context.",
                    handler.span,
                ))
            continue
        if owner_name is None or operation_name is None:
            diagnostics.append(source_error("interrupt_handler_unknown", f"Interrupt handler {project_routine_key!r} is not a declared project routine or task/loop.", handler.span))
            continue
        owner = symbols.signatures.get(owner_name)
        if owner is None:
            diagnostics.append(source_error("interrupt_handler_component_unknown", f"Interrupt handler names undeclared component {owner_name!r}.", handler.span))
            continue
        binding = catalogue.dsl_descriptors.get(owner.registry_key or "")
        contract = binding.contract if binding is not None else None
        operation_registered = contract is not None and operation_name in {str(item.get("id")) for item in contract.operations}
        if operation_registered:
            available_contexts = set(contract.scheduling.get("contexts", ())) if contract is not None else set()
            if interrupt_context != "isr_signal_only":
                diagnostics.append(source_error("interrupt_context_required", "A component operation invoked by an interrupt must declare context = isr_signal_only.", interrupt.span))
            elif "isr" not in available_contexts:
                diagnostics.append(source_error(
                    "interrupt_operation_context_unsupported",
                    f"UETD component {owner_name!r} does not declare ISR execution support for operation {operation_name!r}.",
                    handler.span,
                ))
        elif contract is None:
            diagnostics.append(source_warning(
                "interrupt_handler_contract_missing",
                f"UETD has no complete operation contract for {owner_name!r}; declare 'project routine {owner_name}.{operation_name}();' if this is project-owned.",
                handler.span,
            ))
        elif not contract.operations and contract.generation.get("status") not in {"ready", "implemented", "available"}:
            diagnostics.append(source_warning(
                "interrupt_handler_contract_missing",
                f"UETD has not published operations for draft component {owner_name!r}; declare 'project routine {owner_name}.{operation_name}();' if this is project-owned.",
                handler.span,
            ))
        else:
            diagnostics.append(source_error(
                "interrupt_handler_operation_unknown",
                f"Component {owner_name!r} has no registered operation {operation_name!r} for interrupt handling.",
                handler.span,
            ))
    diagnostics.extend(validate_state_machines(document, catalogue, symbols))
    return diagnostics

def validate_expression_types(document: ProjectDocument, environment: TypeEnvironment, symbols: ProjectSymbolTable) -> tuple[list[TypedExpressionFacts], list[LanguageDiagnostic]]:
    """Type-check expressions with the shared type engine and project symbol scope."""
    scope = dict(environment.language_types)
    scope.update(environment.project_types); scope.update(environment.imported_types); scope.update(environment.uetd_types_by_dsl_name)
    diagnostics: list[LanguageDiagnostic] = []; facts: list[TypedExpressionFacts] = []
    for name, declaration in symbols.signals.items():
        if declaration.bindings and declaration.bindings[0].type_spec:
            resolved, errors = resolve_type_spec(declaration.bindings[0].type_spec, environment); diagnostics.extend(errors)
            if resolved: scope[name] = resolved
    for parameter in symbols.project_parameters.values():
        if parameter.bindings and parameter.bindings[0].type_spec:
            resolved, errors = resolve_type_spec(parameter.bindings[0].type_spec, environment); diagnostics.extend(errors)
            if resolved: scope[parameter.name or ""] = resolved
    for declaration in _walk_declarations(document.declarations):
        for binding in declaration.bindings:
            if binding.type_spec and binding.name not in scope:
                resolved, errors = resolve_type_spec(binding.type_spec, environment); diagnostics.extend(errors)
                if resolved: scope[binding.name] = resolved
        for stage in declaration.stages:
            for binding in stage.parameters:
                if binding.type_spec and binding.name not in scope:
                    resolved, errors = resolve_type_spec(binding.type_spec, environment); diagnostics.extend(errors)
                    if resolved: scope[binding.name] = resolved
    for expression in _project_expressions(document):
        result, errors = infer_expression_type(expression, scope, environment)
        diagnostics.extend(errors)
        if result: facts.append(result)
    return facts, diagnostics

def _add_composition_value_scope(declaration: Declaration, symbols: ProjectSymbolTable, catalogue: AuthoringCatalogue, environment: TypeEnvironment,
                                scope: dict[str, Any], diagnostics: list[LanguageDiagnostic]) -> None:
    """Add output types for named stages in hybrids and subsystems."""
    members = {child.name: child.registry_key for child in declaration.children if child.kind == "use" and child.name}
    for stage in declaration.composition_stages:
        component_name = members.get(stage.member)
        signature = symbols.signatures.get((component_name or "").split(".", 1)[0])
        outputs: list[tuple[str, TypeSpec]] = []
        if signature is not None:
            body = symbols.implementations.get(signature.name or "")
            if body:
                outputs.extend((item.name, item.type_spec) for item in body.bindings if item.role == "output" and item.type_spec)
            binding = catalogue.dsl_descriptors.get(signature.registry_key or "")
            if binding and binding.contract:
                declared = {name for name, _ in outputs}
                outputs.extend(
                    (str(item["name"]), _contract_port_type_spec(item, body))
                    for item in binding.contract.outputs
                    if isinstance(item.get("name"), str)
                    and _contract_port_type_spec(item, body) is not None
                    and item["name"] not in declared
                )
        for output_name, type_spec in outputs:
            resolved, errors = resolve_type_spec(type_spec, environment)
            diagnostics.extend(errors)
            if resolved:
                scope[f"{stage.name}.{output_name}"] = resolved
        if len(outputs) == 1:
            resolved = scope.get(f"{stage.name}.{outputs[0][0]}")
            if resolved:
                scope.setdefault(stage.name, resolved)

def _add_component_output_scope(symbols: ProjectSymbolTable, catalogue: AuthoringCatalogue, environment: TypeEnvironment,
                                scope: dict[str, Any], diagnostics: list[LanguageDiagnostic]) -> None:
    """Resolve component output names used by OS policies and expressions.

    User-authored output bindings are authoritative for locally implemented
    components. For UETD-native modules, output types come from the UETD
    authoring contract. Qualified names are always added; an unqualified
    component name is added only when it has exactly one output and does not
    collide with another known value.
    """
    for name, signature in symbols.signatures.items():
        outputs: dict[str, TypeSpec] = {}
        body = symbols.implementations.get(name)
        if body is not None:
            outputs.update(
                (binding.name, binding.type_spec)
                for binding in body.bindings
                if binding.role == "output" and binding.type_spec is not None
            )

        binding = catalogue.dsl_descriptors.get(signature.registry_key or "")
        contract = binding.contract if binding is not None else None
        if contract is not None:
            for item in contract.outputs:
                output_name = item.get("name")
                output_type = _contract_port_type_spec(item, body)
                if isinstance(output_name, str) and output_type is not None:
                    outputs.setdefault(output_name, output_type)

        resolved_outputs: list[tuple[str, Any]] = []
        for output_name, type_spec in outputs.items():
            resolved, errors = resolve_type_spec(type_spec, environment)
            diagnostics.extend(errors)
            if resolved is not None:
                scope[f"{name}.{output_name}"] = resolved
                resolved_outputs.append((output_name, resolved))

        if len(resolved_outputs) == 1 and name not in scope:
            scope[name] = resolved_outputs[0][1]


def _add_component_input_scope(symbols: ProjectSymbolTable, catalogue: AuthoringCatalogue, environment: TypeEnvironment,
                               scope: dict[str, Any], diagnostics: list[LanguageDiagnostic]) -> None:
    """Expose each component instance's typed input ports to OS expressions.

    Channels deliver data to a component input independently of a task's
    ``run = component(...)`` argument list. The qualified ``component.port``
    path is also available to state-machine guards and other OS policies.
    """
    for name, signature in symbols.signatures.items():
        body = symbols.implementations.get(name)
        bindings = body.bindings if body is not None and body.bindings else signature.bindings
        inputs = [binding for binding in bindings if binding.role == "input" and binding.type_spec is not None]
        if not inputs:
            continue
        for binding in inputs:
            resolved, errors = resolve_type_spec(binding.type_spec, environment)
            diagnostics.extend(errors)
            if resolved is not None:
                scope[f"{name}.{binding.name}"] = resolved


def _defer_unknown_native_members(expression: Any, errors: list[LanguageDiagnostic], document: ProjectDocument,
                                  symbols: ProjectSymbolTable, catalogue: AuthoringCatalogue) -> list[LanguageDiagnostic]:
    """Downgrade member errors whose registered native module has no contract.

    A missing UETD authoring contract means the frontend cannot know which
    public fields a native component exposes. The module's existing
    ``uetd_descriptor_contract_missing`` diagnostic is authoritative; report
    member access as unresolved instead of falsely claiming the field does
    not exist. Project-authored and contracted component members stay strict.
    """
    if not any(item.code == "unknown_value" for item in errors):
        return errors
    unresolved_roots: dict[str, str] = {}
    for name, signature in symbols.signatures.items():
        descriptor = catalogue.dsl_descriptors.get(signature.registry_key or "")
        if signature.registry_key and (descriptor is None or descriptor.contract is None):
            unresolved_roots[name] = descriptor.module_id if descriptor is not None else signature.registry_key
    for declaration in _walk_declarations(document.declarations):
        aliases = {child.name: child.registry_key for child in declaration.children
                   if child.kind == "use" and child.name}
        for stage in declaration.composition_stages:
            component_name = aliases.get(stage.member.split(".", 1)[0])
            signature = symbols.signatures.get((component_name or "").split(".", 1)[0])
            if signature is None:
                continue
            descriptor = catalogue.dsl_descriptors.get(signature.registry_key or "")
            if signature.registry_key and (descriptor is None or descriptor.contract is None):
                unresolved_roots[stage.name] = descriptor.module_id if descriptor is not None else signature.registry_key
    unresolved: dict[str, tuple[Any, str]] = {}
    references = {".".join(item.path): item for item in _references(expression) if item.path}
    for item in errors:
        if item.code != "unknown_value":
            continue
        for value_path, reference in references.items():
            if value_path not in item.message:
                continue
            module_id = unresolved_roots.get(reference.path[0])
            if module_id:
                unresolved[value_path] = (item.span or reference.span, module_id)
    if not unresolved:
        return errors
    output = [item for item in errors if item.code != "unknown_value"]
    for value_path, (span, module_id) in unresolved.items():
        output.append(source_warning(
            "unresolved_native_member",
            f"Cannot verify member {value_path!r} until UETD provides an authoring contract for {module_id!r}.",
            span,
        ))
    return output

def _add_state_machine_state_scope(document: ProjectDocument, scope: dict[str, Any], environment: TypeEnvironment) -> None:
    """Expose each state-machine instance's typed `.state` value."""
    for declaration in _walk_declarations(document.declarations):
        if declaration.kind != "state_machine" or not declaration.name:
            continue
        state_type = environment.project_types.get(declaration.registry_key or "") or environment.imported_types.get(declaration.registry_key or "")
        if state_type is not None:
            scope[f"{declaration.name}.state"] = state_type

def _add_protocol_message_scope(document: ProjectDocument, catalogue: AuthoringCatalogue, environment: TypeEnvironment,
                                symbols: ProjectSymbolTable, imported_contracts: tuple[ImportedSystemContract, ...],
                                scope: dict[str, Any], diagnostics: list[LanguageDiagnostic]) -> None:
    """Add the typed views declared by custom protocols and endpoint contracts.

    Message payload types come from a custom protocol's `logical_type`, or
    from the application-field roots in an imported `.uefs` protocol. FPGA
    endpoint member names come from UETD's `protocol_endpoint_binding`.
    """
    for name, signature in symbols.custom_protocols.items():
        body = symbols.implementations.get(name)
        if body is None:
            continue
        messages: list[tuple[str, str | None]] = []
        for child in body.children:
            if child.kind != "message" or not child.name:
                continue
            properties = _property_map(child)
            payload_name = _value_reference_name(properties.get("logical_type"))
            messages.append((child.name, payload_name))
        _add_protocol_instance_members(
            name,
            messages,
            receive_namespace="receive",
            freshness_namespace="fresh",
            send_namespace="transmit",
            environment=environment,
            scope=scope,
            diagnostics=diagnostics,
        )

    imported_protocols = {
        protocol.name: protocol
        for contract in imported_contracts
        for protocol in contract.protocols
    }
    for name, signature in symbols.signatures.items():
        binding = catalogue.dsl_descriptors.get(signature.registry_key or "")
        endpoint_contract = binding.contract.protocol_endpoint_binding if binding and binding.contract else None
        if not isinstance(endpoint_contract, dict):
            continue
        body = symbols.implementations.get(name)
        protocol_name = _value_reference_name(_property_map(body).get("protocol"))
        protocol = imported_protocols.get(protocol_name or "")
        if protocol is None:
            continue
        messages = [
            (message.get("name"), _system_protocol_payload_name(message))
            for message in protocol.messages
            if isinstance(message.get("name"), str)
        ]
        _add_protocol_instance_members(
            name,
            messages,
            receive_namespace=str(endpoint_contract.get("receive_namespace", "receive")),
            freshness_namespace=str(endpoint_contract.get("freshness_namespace", "fresh")),
            send_namespace=str(endpoint_contract.get("send_namespace", "send")),
            environment=environment,
            scope=scope,
            diagnostics=diagnostics,
        )

def _add_protocol_instance_members(instance_name: str, messages: list[tuple[str, str | None]], *,
                                   receive_namespace: str, freshness_namespace: str, send_namespace: str,
                                   environment: TypeEnvironment, scope: dict[str, Any],
                                   diagnostics: list[LanguageDiagnostic]) -> None:
    """Publish the contract-defined payload, freshness, and send completion values."""
    received_identity = TypeIdentity("language", "Received", "Received", "generic")
    for message_name, payload_name in messages:
        if not isinstance(message_name, str):
            continue
        scope[f"{instance_name}.{freshness_namespace}.{message_name}"] = environment.language_types["bool"]
        scope[f"{instance_name}.{send_namespace}.{message_name}.complete"] = environment.language_types["bool"]
        if not payload_name:
            continue
        payload_type, errors = resolve_type_spec(TypeSpec(payload_name), environment)
        diagnostics.extend(errors)
        if payload_type is not None:
            scope[f"{instance_name}.{receive_namespace}.{message_name}"] = ResolvedType(
                received_identity,
                arguments=(payload_type,),
                boundary="value",
            )

def _system_protocol_payload_name(message: dict[str, Any]) -> str | None:
    """Find a message's logical record from its explicit or field-map contract."""
    properties = message.get("properties", {})
    logical_name = _value_reference_name(properties.get("logical_type")) if isinstance(properties, dict) else None
    if logical_name:
        return logical_name.split(".", 1)[0]
    roots: set[str] = set()
    fields_to_visit = list(message.get("fields", ()))
    for fragment in message.get("fragments", ()):
        if isinstance(fragment, dict):
            fields_to_visit.extend(fragment.get("fields", ()))
    for field in fields_to_visit:
        if not isinstance(field, dict):
            continue
        field_properties = field.get("properties", {})
        if not isinstance(field_properties, dict):
            continue
        application_field = _value_reference_name(field_properties.get("application_field"))
        if application_field and "." in application_field:
            roots.add(application_field.split(".", 1)[0])
    return next(iter(roots)) if len(roots) == 1 else None


def _add_protocol_send_expected_types(document: ProjectDocument, catalogue: AuthoringCatalogue,
                                     environment: TypeEnvironment, symbols: ProjectSymbolTable,
                                     imported_contracts: tuple[ImportedSystemContract, ...],
                                     expected_by_expression: dict[int, Any],
                                     diagnostics: list[LanguageDiagnostic]) -> None:
    """Type outbound endpoint payload assignments from the imported protocol.

    A record-valued ``send.message = map(...)`` has no local annotation. Its
    expected nominal record comes from the message declaration in the
    project's imported system contract, so use that declaration here rather
    than asking the user to repeat the protocol type in the implementation.
    """
    imported_protocols = {
        protocol.name: protocol
        for contract in imported_contracts
        for protocol in contract.protocols
    }
    for name, signature in symbols.signatures.items():
        descriptor = catalogue.dsl_descriptors.get(signature.registry_key or "")
        contract = descriptor.contract if descriptor is not None else None
        endpoint_binding = contract.protocol_endpoint_binding if contract is not None else None
        if not isinstance(endpoint_binding, dict):
            continue
        body = symbols.implementations.get(name)
        if body is None:
            continue
        protocol_name = _value_reference_name(_property_map(body).get("protocol"))
        protocol = imported_protocols.get(protocol_name or "")
        if protocol is None:
            continue
        namespace = str(endpoint_binding.get("send_namespace", "send"))
        messages = {message.get("name"): message for message in protocol.messages
                    if isinstance(message.get("name"), str)}
        for prop in body.properties:
            parts = prop.name.split(".")
            if len(parts) != 2 or parts[0] != namespace:
                continue
            payload_name = _system_protocol_payload_name(messages.get(parts[1], {}))
            if not payload_name:
                continue
            expected, errors = resolve_type_spec(TypeSpec(payload_name, span=prop.span), environment)
            diagnostics.extend(errors)
            if expected is not None:
                expected_by_expression[id(prop.value)] = expected

def _add_operating_system_channel_scope(document: ProjectDocument, scope: dict[str, Any], environment: TypeEnvironment) -> None:
    """Expose OS channels as typed receive envelopes in project expressions.

    A channel's source is a component output (for example
    ``receiver.input``). Consumers read the channel as ``channel.value``;
    wrapping the payload in ``Received<T>`` lets the common type resolver
    check that path and all of the record fields below it.
    """
    received_identity = TypeIdentity("language", "Received", "Received", "generic")
    for declaration in _walk_declarations(document.declarations):
        if declaration.kind != "channel" or not declaration.name:
            continue
        scope[f"{declaration.name}.not_empty"] = environment.language_types["bool"]
        overflow = _value_reference_name(_property_map(declaration).get("overflow")) or ""
        if overflow.endswith("_and_count"):
            scope[f"{declaration.name}.dropped_count"] = environment.language_types["u32"]
        source_name = _value_reference_name(_property_map(declaration).get("source"))
        source_type = scope.get(source_name or "")
        if source_type is None:
            continue
        # A forwarded receive envelope keeps its original payload type; it is
        # not wrapped recursively by each latest-value channel.
        payload_type = (source_type.arguments[0]
                        if source_type.identity.kind == "generic"
                        and source_type.identity.canonical_name == "Received"
                        and source_type.arguments else source_type)
        scope[declaration.name] = ResolvedType(
            received_identity,
            arguments=(payload_type,),
            boundary="value",
        )

def _add_operating_system_runtime_scope(document: ProjectDocument, environment: TypeEnvironment, scope: dict[str, Any]) -> None:
    """Expose the small set of runtime values defined by UEF-PL constructs."""
    task_reference_type = ResolvedType(TypeIdentity("language", "TaskReference", "TaskReference", "symbol"))
    for declaration in _walk_declarations(document.declarations):
        if declaration.kind in {"task", "loop"} and declaration.name:
            scope.setdefault(declaration.name, task_reference_type)
        elif declaration.kind == "interrupt" and declaration.name:
            # Interrupt event counters are maintained by the runtime binding;
            # they are not fields on the user's hardware driver.
            scope[f"{declaration.name}.count"] = environment.language_types["u32"]
        elif declaration.kind == "peripheral" and declaration.name:
            direction = _value_reference_name(_property_map(declaration).get("direction"))
            if direction == "input":
                scope[f"{declaration.name}.active"] = environment.language_types["bool"]

def validate_umath_calls(document: ProjectDocument, catalogue: AuthoringCatalogue, symbols: ProjectSymbolTable) -> list[LanguageDiagnostic]:
    """Allow UMATH only in custom algorithm and nonlinear model equations."""
    diagnostics: list[LanguageDiagnostic] = []
    del catalogue, symbols
    seen: set[int] = set()
    for expression, context in _expressions_with_context(document):
        for declaration in _walk_values(expression):
            if not isinstance(declaration, MathCallExpression) or id(declaration) in seen:
                continue
            seen.add(id(declaration))
            if context not in {"custom_algorithm", "model_equations"}:
                diagnostics.append(source_error("umath_context_forbidden", f"UMATH calls are not allowed in {context} declarations.", declaration.span))
    return diagnostics

def validate_numerics(document: ProjectDocument, catalogue: AuthoringCatalogue) -> list[LanguageDiagnostic]:
    """Reject non-finite values and malformed numeric-profile settings."""
    diagnostics: list[LanguageDiagnostic] = []
    allowed_numeric = {"scalar", "accumulator", "rounding", "overflow", "non_finite", "fused_multiply_add", "matrix_layout"}
    numeric = next((item for item in document.declarations if item.kind == "numeric"), None)
    if numeric:
        for prop in numeric.properties:
            if prop.name not in allowed_numeric: diagnostics.append(source_error("unknown_numeric_policy", f"Unknown numeric policy {prop.name!r}.", prop.span))
    for expression in _project_expressions(document):
        for node in _walk_values(expression):
            if isinstance(node, Literal) and isinstance(node.value, float) and not math.isfinite(node.value):
                diagnostics.append(source_error("non_finite_literal", "Numeric literals must be finite.", node.span))
    return diagnostics

def _validate_integer_values(name: str, storage_type: str, items: list[tuple[str, int | None, Any]], span: Any) -> list[LanguageDiagnostic]:
    diagnostics: list[LanguageDiagnostic] = []
    width = _integer_width(storage_type)
    signed = storage_type.startswith("i")
    minimum = -(1 << (width - 1)) if signed else 0
    maximum = (1 << (width - (1 if signed else 0))) - 1
    used: set[int] = set()
    next_value = 0
    names: set[str] = set()
    for case_name, explicit, case_span in items:
        if case_name in names: diagnostics.append(source_error("duplicate_enum_case", f"Case {case_name!r} is duplicated in {name!r}.", case_span or span))
        names.add(case_name)
        value = next_value if explicit is None else explicit
        if value < minimum or value > maximum: diagnostics.append(source_error("enum_value_out_of_range", f"Value {value} does not fit {storage_type} in {name!r}.", case_span or span))
        if value in used: diagnostics.append(source_error("duplicate_enum_value", f"Value {value} is assigned more than once in {name!r}.", case_span or span))
        used.add(value); next_value = value + 1
    return diagnostics

def _integer_width(storage_type: str) -> int:
    try: return int(storage_type.lstrip("ui"))
    except (TypeError, ValueError): return 0

def _is_constant_expression(value: Any) -> bool:
    if isinstance(value, Literal): return isinstance(value.value, (bool, int, float, str)) and (not isinstance(value.value, float) or math.isfinite(value.value))
    if isinstance(value, UnaryExpression): return _is_constant_expression(value.operand)
    if isinstance(value, BinaryExpression): return _is_constant_expression(value.left) and _is_constant_expression(value.right)
    return False

def _constant_number(value: Any) -> int | float | None:
    if isinstance(value, Literal) and isinstance(value.value, (int, float)) and not isinstance(value.value, bool): return value.value
    if isinstance(value, UnaryExpression):
        operand = _constant_number(value.operand)
        if operand is None: return None
        if value.operator == "-": return -operand
        if value.operator == "+": return +operand
    if isinstance(value, BinaryExpression):
        left = _constant_number(value.left); right = _constant_number(value.right)
        if left is None or right is None: return None
        try:
            if value.operator == "+": return left + right
            if value.operator == "-": return left - right
            if value.operator == "*": return left * right
            if value.operator == "/" and right != 0: return left / right
            if value.operator == "**": return left ** right
        except (OverflowError, ZeroDivisionError): return None
    return None

def _walk_declarations(declarations: tuple[Declaration, ...] | list[Declaration]):
    for declaration in declarations:
        yield declaration
        yield from _walk_declarations(declaration.children)

def _project_expressions(document: ProjectDocument):
    """Yield expression roots once, preserving their enclosing AST structure."""
    for declaration in _walk_declarations(document.declarations):
        yield from _declaration_expressions(declaration)

def _declaration_expressions(declaration: Declaration):
    """Yield expressions owned by one lexical declaration scope."""
    if declaration.kind == "choose":
        for prop in declaration.properties:
            if prop.name == "when" and isinstance(prop.value, RecordValue):
                for _, nested in prop.value.fields:
                    yield nested
            else:
                yield prop.value
    else:
        for prop in declaration.properties:
            if declaration.kind == "model" and prop.name == "discretize" and isinstance(prop.value, RecordValue):
                fields = dict(prop.value.fields)
                # Avoid treating the parser's internal discretization tuple as
                # an ordinary record literal (which would demand a project
                # record annotation and report map_requires_expected_record).
                for key in ("method", "period"):
                    if key in fields:
                        yield fields[key]
            else:
                yield prop.value
    for binding in declaration.bindings:
        if binding.value is not None:
            yield binding.value
    for stage in declaration.stages:
        for statement in stage.body:
            yield from _statement_expressions(statement)
    for statement in declaration.equations:
        yield from _statement_expressions(statement)
    for stage in declaration.composition_stages:
        for _, value in stage.arguments:
            yield value
        if stage.guard is not None:
            yield stage.guard
        if stage.on_failure is not None:
            yield stage.on_failure
    for local in declaration.composition_locals:
        yield local.expression
    for output in declaration.composition_outputs:
        yield output.expression
    for output in declaration.output_assignments:
        yield output.expression
    # A choose block is syntactically nested in its owning hybrid/subsystem;
    # its conditions and values share that parent's stage/input scope.
    for child in declaration.children:
        if child.kind == "choose":
            yield from _declaration_expressions(child)

def _statement_expressions(statement: Any):
    if hasattr(statement, "expression") and getattr(statement, "expression") is not None: yield statement.expression
    if isinstance(statement, BoundedLoop):
        yield statement.start; yield statement.stop_exclusive
        for nested in statement.body: yield from _statement_expressions(nested)
    elif isinstance(statement, ConditionalStatement):
        yield statement.condition
        for nested in (*statement.then_body, *statement.else_body): yield from _statement_expressions(nested)

def _walk_values(value: Any):
    """Yield an expression node and its nested expression children."""
    if isinstance(value, (str, bytes, int, float, bool, type(None))): return
    if isinstance(value, (tuple, list)):
        for item in value: yield from _walk_values(item)
        return
    if not is_dataclass(value): return
    if hasattr(value, "span") and type(value).__module__.endswith("language.frontends.project.ast"):
        yield value
    for item in fields(value):
        if item.name in {"span", "path", "start", "end"}: continue
        yield from _walk_values(getattr(value, item.name))

def _references(value: Any):
    for item in _walk_values(value):
        if isinstance(item, Reference): yield item

def _expressions_with_context(document: ProjectDocument):
    def visit(declaration: Declaration, inherited_context: str = "configuration"):
        context = inherited_context
        if declaration.kind == "custom_algorithm": context = "custom_algorithm"
        elif declaration.kind == "model": context = "model_equations"
        elif declaration.kind in {"protocol", "custom_protocol"}: context = "protocol"
        elif declaration.kind in {"operatingsystem", "task", "loop", "taskset", "state_machine"}: context = "runtime"
        for prop in declaration.properties:
            yield prop.value, context
        for binding in declaration.bindings:
            if binding.value is not None: yield binding.value, context
        for stage in declaration.stages:
            for statement in stage.body:
                for expression in _statement_expressions(statement):
                    yield expression, "custom_algorithm" if context == "custom_algorithm" else context
        for statement in declaration.equations:
            for expression in _statement_expressions(statement):
                yield expression, "model_equations" if context == "model_equations" else context
        for stage in declaration.composition_stages:
            for _, value in stage.arguments: yield value, context
            if stage.guard is not None: yield stage.guard, context
            if stage.on_failure is not None: yield stage.on_failure, context
        for local in declaration.composition_locals: yield local.expression, context
        for output in declaration.composition_outputs: yield output.expression, context
        for output in declaration.output_assignments: yield output.expression, context
        for child in declaration.children:
            yield from visit(child, context)

    for declaration in document.declarations:
        yield from visit(declaration)

def _validate_composition_symbols(signatures: dict[str, Declaration], symbols: ProjectSymbolTable,
                                  catalogue: AuthoringCatalogue, expected_kind: str) -> list[LanguageDiagnostic]:
    diagnostics: list[LanguageDiagnostic] = []
    for signature in signatures.values():
        body = symbols.implementations.get(signature.name or "")
        if body is None: continue
        alias_items = [child for child in body.children if child.kind == "use"]
        aliases = {child.name: child.registry_key for child in alias_items}
        if len(aliases) != len(alias_items):
            diagnostics.append(source_error("duplicate_composition_alias", f"{expected_kind} member aliases must be unique.", body.span))
        for alias, component in aliases.items():
            if component not in symbols.signatures and (component or "").split(".", 1)[0] not in symbols.signatures:
                if component not in catalogue.dsl_descriptors:
                    diagnostics.append(source_error("unknown_composition_member", f"{expected_kind} member {alias!r} refers to undeclared component {component!r}.", body.span))
        stage_indexes = {stage.name: index for index, stage in enumerate(body.composition_stages)}
        stage_offsets = {stage.name: stage.span.start.offset for stage in body.composition_stages
                         if stage.span is not None}
        local_names = {item.name: item.span.start.offset for item in body.composition_locals if item.span is not None}
        body_names = [*stage_indexes, *local_names]
        if len(body_names) != len(set(body_names)):
            diagnostics.append(source_error("duplicate_composition_value", "Stage results and local values share one namespace and must have unique names.", body.span))
        for index, stage in enumerate(body.composition_stages):
            member_alias = stage.member.split(".", 1)[0]
            if member_alias not in aliases:
                diagnostics.append(source_error("undeclared_composition_alias", f"Stage {stage.name!r} uses undeclared alias {stage.member!r}.", stage.span))
                continue
            argument_names = [name for name, _ in stage.arguments]
            if len(argument_names) != len(set(argument_names)):
                diagnostics.append(source_error("duplicate_stage_argument", f"Stage {stage.name!r} repeats a named input.", stage.span))
            member_target = aliases.get(member_alias)
            input_types = _composition_member_ports(member_target, symbols, catalogue, "input")
            if input_types is not None:
                supplied = set(argument_names)
                unknown = supplied - set(input_types)
                required_inputs = _composition_member_required_inputs(member_target, symbols, catalogue, input_types)
                missing = required_inputs - supplied
                if unknown:
                    diagnostics.append(source_error("unknown_stage_input", f"Stage {stage.name!r} supplies undeclared input(s): {', '.join(sorted(unknown))}.", stage.span))
                if missing:
                    diagnostics.append(source_error("missing_stage_input", f"Stage {stage.name!r} omits required input(s): {', '.join(sorted(missing))}.", stage.span))
            expressions = [value for _, value in stage.arguments]
            if stage.guard is not None: expressions.append(stage.guard)
            if stage.on_failure is not None: expressions.append(stage.on_failure)
            for expression in expressions:
                for reference in _references(expression):
                    root = reference.path[0] if reference.path else ""
                    if root in stage_indexes and stage_indexes[root] >= index:
                        diagnostics.append(source_error("stage_order_reference", f"Stage {stage.name!r} may only read results from earlier stages; {root!r} is not available yet.", reference.span))
                    local_offset = local_names.get(root)
                    if local_offset is not None and stage.span is not None and local_offset >= stage.span.start.offset:
                        diagnostics.append(source_error("local_order_reference", f"Stage {stage.name!r} reads local {root!r} before that local is declared.", reference.span))
        for local in body.composition_locals:
            if local.span is None:
                continue
            for reference in _references(local.expression):
                root = reference.path[0] if reference.path else ""
                if root in stage_offsets and stage_offsets[root] >= local.span.start.offset:
                    diagnostics.append(source_error("local_forward_stage_reference", f"Local {local.name!r} reads stage result {root!r} before that stage is declared.", reference.span))
                local_offset = local_names.get(root)
                if local_offset is not None and local_offset >= local.span.start.offset:
                    diagnostics.append(source_error("local_order_reference", f"Local {local.name!r} reads local {root!r} before that local is declared.", reference.span))

        output_bindings = body.bindings or signature.bindings
        declared_outputs = {binding.name for binding in output_bindings if binding.role == "output"}
        assigned_output_paths: list[tuple[tuple[str, ...], Any]] = []
        assigned_outputs: set[str] = set()
        assignments = [(output.name, output.expression, output.span, "output")
                       for output in body.composition_outputs]
        for choice in (child for child in body.children if child.kind == "choose"):
            assignments.append((choice.name or "", None, choice.span, "choose"))
            branches = choice.properties
            when_branches = [item for item in branches if item.name == "when"]
            otherwise_branches = [item for item in branches if item.name == "otherwise"]
            if not when_branches:
                diagnostics.append(source_error("choose_requires_when", f"Choice for output {choice.name!r} needs at least one 'when' branch.", choice.span))
            if len(otherwise_branches) > 1:
                diagnostics.append(source_error("duplicate_choose_otherwise", f"Choice for output {choice.name!r} has more than one 'otherwise' branch.", choice.span))
            if otherwise_branches and branches[-1] is not otherwise_branches[0]:
                diagnostics.append(source_error("choose_otherwise_must_be_last", "The 'otherwise' branch must be last in a choice.", otherwise_branches[0].span))
            for branch in branches:
                if branch.name == "when" and isinstance(branch.value, RecordValue):
                    values = dict(branch.value.fields)
                    branch_expressions = [values.get("condition"), values.get("value")]
                else:
                    branch_expressions = [branch.value]
                for expression in branch_expressions:
                    if expression is None:
                        continue
                    for reference in _references(expression):
                        root = reference.path[0] if reference.path else ""
                        if root in stage_offsets and choice.span is not None and stage_offsets[root] >= choice.span.start.offset:
                            diagnostics.append(source_error("choice_forward_stage_reference", f"Choice for output {choice.name!r} reads stage result {root!r} before that stage is declared.", reference.span))
                        local_offset = local_names.get(root)
                        if local_offset is not None and choice.span is not None and local_offset >= choice.span.start.offset:
                            diagnostics.append(source_error("choice_local_order_reference", f"Choice for output {choice.name!r} reads local {root!r} before it is declared.", reference.span))

        for output_name, expression, span, source_kind in assignments:
            output_path = tuple(part for part in output_name.split(".") if part)
            root_name = output_path[0] if output_path else ""
            if root_name not in declared_outputs:
                diagnostics.append(source_error("assignment_to_undeclared_output", f"{expected_kind} {body.name!r} has no declared output {root_name!r}.", span))
            if any(output_path[:len(old_path)] == old_path or old_path[:len(output_path)] == output_path
                   for old_path, _ in assigned_output_paths):
                diagnostics.append(source_error("duplicate_output_assignment", f"Output path {output_name!r} overlaps another assignment in {expected_kind} {body.name!r}.", span))
            else:
                assigned_output_paths.append((output_path, span))
            assigned_outputs.add(root_name)
            if expression is not None and span is not None:
                for reference in _references(expression):
                    root = reference.path[0] if reference.path else ""
                    if root in stage_offsets and stage_offsets[root] >= span.start.offset:
                        diagnostics.append(source_error("output_forward_stage_reference", f"Output {root_name!r} reads stage result {root!r} before that stage is declared.", reference.span))
                    local_offset = local_names.get(root)
                    if local_offset is not None and local_offset >= span.start.offset:
                        diagnostics.append(source_error("output_local_order_reference", f"Output {root_name!r} reads local {root!r} before it is declared.", reference.span))
        missing_outputs = declared_outputs - assigned_outputs
        if missing_outputs:
            diagnostics.append(source_error("composition_output_unassigned", f"{expected_kind} {body.name!r} does not assign declared output(s): {', '.join(sorted(missing_outputs))}.", body.span))
        if expected_kind == "hybrid_algorithm" and not body.composition_stages:
            diagnostics.append(source_error("hybrid_stage_required", "A hybrid algorithm must define at least one ordered stage.", body.span))
    return diagnostics

def _validate_invocation_arguments(invocation: Declaration, symbols: ProjectSymbolTable,
                                   catalogue: AuthoringCatalogue,
                                   channel_inputs: set[str] | None = None) -> list[LanguageDiagnostic]:
    """Check scheduled invocation names and required arguments against the call contract."""
    diagnostics: list[LanguageDiagnostic] = []
    names = [prop.name for prop in invocation.properties]
    if len(names) != len(set(names)):
        diagnostics.append(source_error("duplicate_invocation_argument", f"Invocation of {invocation.name!r} repeats a named argument.", invocation.span))
    ports = _composition_member_ports(invocation.name, symbols, catalogue, "input")
    if ports is None:
        return diagnostics
    supplied = set(names)
    unknown = supplied - set(ports)
    missing = (_composition_member_required_inputs(invocation.name, symbols, catalogue, ports)
               - supplied - (channel_inputs or set()))
    if unknown:
        diagnostics.append(source_error("unknown_invocation_argument", f"Invocation of {invocation.name!r} supplies unknown input(s): {', '.join(sorted(unknown))}.", invocation.span))
    if missing:
        diagnostics.append(source_error("missing_invocation_argument", f"Invocation of {invocation.name!r} omits required input(s): {', '.join(sorted(missing))}.", invocation.span))
    return diagnostics

def _composition_member_ports(member: str | None, symbols: ProjectSymbolTable,
                              catalogue: AuthoringCatalogue, direction: str) -> dict[str, TypeSpec | None] | None:
    """Return a member's authored or registry-owned typed call boundary."""
    if not member:
        return None
    signature = symbols.signatures.get(member) or symbols.signatures.get(member.split(".", 1)[0])
    if signature is None:
        descriptor = catalogue.dsl_descriptors.get(member)
        contract = descriptor.contract if descriptor is not None else None
        if contract is None:
            return None
        body = None
        values = contract.inputs if direction == "input" else contract.outputs
        if not values and contract.generation.get("status") not in {"ready", "implemented", "available"}:
            return None
        return {str(item["name"]): _contract_port_type_spec(item, body) for item in values
                if isinstance(item.get("name"), str)}
    body = symbols.implementations.get(signature.name or "")
    bindings = body.bindings if body is not None and body.bindings else signature.bindings
    authored = {binding.name: binding.type_spec for binding in bindings
                if binding.role == direction and binding.type_spec is not None}
    if authored:
        return authored
    descriptor = catalogue.dsl_descriptors.get(signature.registry_key or "")
    contract = descriptor.contract if descriptor is not None else None
    if contract is None:
        return None
    values = contract.inputs if direction == "input" else contract.outputs
    if not values and contract.generation.get("status") not in {"ready", "implemented", "available"}:
        return None
    return {str(item["name"]): _contract_port_type_spec(item, body) for item in values
            if isinstance(item.get("name"), str)}

def _composition_member_required_inputs(member: str | None, symbols: ProjectSymbolTable,
                                        catalogue: AuthoringCatalogue,
                                        ports: dict[str, TypeSpec | None]) -> set[str]:
    """Return required call inputs, respecting UETD's optional-port metadata."""
    if not member:
        return set()
    signature = symbols.signatures.get(member) or symbols.signatures.get(member.split(".", 1)[0])
    if signature is not None:
        body = symbols.implementations.get(signature.name or "")
        bindings = body.bindings if body is not None and body.bindings else signature.bindings
        authored = [binding for binding in bindings if binding.role == "input"]
        if authored:
            return {binding.name for binding in authored
                    if not any(prop.name == "optional" and isinstance(prop.value, Literal) and prop.value.value is True
                               for prop in binding.properties)}
        descriptor = catalogue.dsl_descriptors.get(signature.registry_key or "")
    else:
        descriptor = catalogue.dsl_descriptors.get(member)
    contract = descriptor.contract if descriptor is not None else None
    if contract is None:
        return set()
    return {str(item["name"]) for item in contract.inputs
            if isinstance(item.get("name"), str) and item.get("required", True) is True}

def _port_type_spec(name: Any) -> TypeSpec | None:
    """Convert only concrete registry port types into value type expectations."""
    if not isinstance(name, str) or name in {"scalar", "signal-defined", "output-defined", "vector", "opaque"}:
        return None
    return TypeSpec(name)

def _contract_port_type_spec(port: dict[str, Any], instance: Declaration | None = None) -> TypeSpec | None:
    """Resolve a UETD port's declared type and any contract-defined unit slot.

    UETD uses ``output-defined`` for ports such as PID output: the physical
    unit follows the configured output limits. Read that unit from the native
    module instance's own ``output_min``/``output_max`` settings so expression
    checking retains the unit written by the project author.
    """
    if port.get("units") == "signal-defined":
        return None
    result = _port_type_spec(port.get("type"))
    if result is None:
        # ``scalar`` is a real port type whose representation is selected by
        # the project's numeric profile, unlike an abstract ``vector`` slot.
        if port.get("type") == "scalar":
            result = TypeSpec("scalar")
        else:
            return None
    unit = port.get("unit")
    if port.get("units") == "output-defined":
        unit = _configured_output_unit(instance)
    if isinstance(unit, str) and unit:
        return TypeSpec(result.name, shape=result.shape, unit=unit, span=result.span,
                        type_arguments=result.type_arguments)
    return result

def _configured_output_unit(instance: Declaration | None) -> str | None:
    """Return one consistent unit from a native module's output-limit pair."""
    if instance is None:
        return None
    properties = _property_map(instance)
    units: list[str | None] = []
    for name in ("output_min", "output_max"):
        value = properties.get(name)
        while isinstance(value, UnaryExpression) and value.operator in {"+", "-"}:
            value = value.operand
        if isinstance(value, Literal):
            units.append(value.unit)
    if not units or len(set(units)) != 1:
        return None
    return units[0]

def _value_reference_name(value: Any) -> str | None:
    if isinstance(value, str): return value
    if isinstance(value, Reference): return ".".join(value.path)
    if isinstance(value, Identifier): return value.value
    if isinstance(value, Literal) and isinstance(value.value, str): return value.value
    return None

def _single_reference_name(value: Any) -> str | None:
    """Return one identifier-like name, rejecting dotted runtime paths."""
    if isinstance(value, Reference) and len(value.path) == 1:
        return value.path[0]
    if isinstance(value, Identifier):
        return value.value
    if isinstance(value, str):
        return value
    return None

def _reference_list_names(value: Any) -> tuple[str, ...] | None:
    """Read a list whose entries each name one scheduled context."""
    if not isinstance(value, ListValue):
        return None
    names: list[str] = []
    for item in value.items:
        name = _single_reference_name(item)
        if name is None:
            return None
        names.append(name)
    return tuple(names)

def _statically_nonpositive_integer(value: Any) -> bool:
    """Reject literal invalid capacities while leaving typed parameters to type checking."""
    constant = _constant_number(value)
    return constant is not None and (not isinstance(constant, int) or constant <= 0)

def _property_map(declaration: Declaration | None) -> dict[str, Any]:
    return {item.name: item.value for item in declaration.properties} if declaration is not None else {}

def _equation_target_root(statement: Equation) -> str | None:
    target = statement.target.value if hasattr(statement.target, "value") and isinstance(statement.target, IndexExpression) else statement.target
    if isinstance(target, Reference) and target.path:
        return target.path[0]
    return None

def _equation_target_name(statement: Equation) -> str | None:
    target = statement.target.value if isinstance(statement.target, IndexExpression) else statement.target
    if isinstance(target, Reference) and len(target.path) == 1:
        return target.path[0]
    return None

def _deduplicate_diagnostics(diagnostics: list[LanguageDiagnostic]) -> list[LanguageDiagnostic]:
    output: list[LanguageDiagnostic] = []
    seen: set[tuple[Any, ...]] = set()
    for diagnostic in diagnostics:
        span = diagnostic.span
        key = (diagnostic.severity, diagnostic.code, diagnostic.message,
               str(span.path) if span else None, span.start.offset if span else None)
        if key not in seen:
            seen.add(key); output.append(diagnostic)
    output.sort(key=lambda item: (str(item.span.path) if item.span and item.span.path else "", item.span.start.offset if item.span else -1, item.code))
    return output
