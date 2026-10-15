"""Lower validated UEF-PL ASTs into hardware and standalone application models."""

from __future__ import annotations
from contextvars import ContextVar
from pathlib import Path
from dataclasses import fields, is_dataclass, replace
from typing import Any
from uef_gen.chips import load_chip_database
from uef_gen.chips.development_boards import load_development_boards
from uef_gen.language.frontends.project.application_spec import (
    BoundaryRecordSpec, ChannelSpec, ComponentCallSpec, ComponentSpec, CompositionLocalSpec,
    CompositionOutputSpec, ContractImportSpec, CustomAlgorithmSpec, EnumSpec,
    EnumValueSpec, FixedPointSpec, FlagsSpec, LoopSpec, ModelSpec, OperatingSystemSpec,
    PortBindingSpec, PortContractSpec, ProjectRoutineSpec, ProtocolFieldSpec, ProtocolMessageSpec, ProtocolSpec,
    RecordSpec, StandaloneApplicationSpec, StateActionSpec, StateMachineSpec, StateTransitionSpec,
    SubsystemSpec, TaskInvocationSpec,
    TaskSpec, TasksetSpec, TypeInterchangeManifest, VariantSpec,
)
from uef_gen.language.frontends.project.ast import (
    BinaryExpression, BoundedLoop, CastExpression, CompositionLocal, CompositionStage, ConditionalStatement, Identifier,
    Declaration, Equation, FlagsDefinition, FixedPointDefinition, IndexExpression,
    FlagsOperationExpression, ListValue, Literal, MatchExpression, MatrixLiteral, MathCallExpression, ProjectDocument,
    RecordBoundary, RecordValue, Reference, SelectValue, SourceMap, TypeSpec, UnaryExpression,
    UnitConversionExpression, VariantConstructorExpression, VariantDefinition,
)
from uef_gen.language.frontends.project.contract_imports import ImportedSystemContract
from uef_gen.language.frontends.project.registry_contracts import AuthoringCatalogue
from uef_gen.language.frontends.project.type_interchange import ResolvedType, TypeEnvironment, TypedExpressionFacts
from uef_gen.language.frontends.system.ast import SystemRecordSpec
from uef_gen.language.serialization import to_json_value

_ACTIVE_UMATH_BINDINGS: ContextVar[dict[int, Any]] = ContextVar("uef_pl_umath_bindings", default={})
_ACTIVE_UMATH_SHAPES: ContextVar[dict[int, dict[str, int]]] = ContextVar("uef_pl_umath_shapes", default={})

def lower_hardware_configuration(document: ProjectDocument, catalogue: AuthoringCatalogue, source_map: SourceMap,
                                 *, uetd_root: Path | None = None) -> dict[str, Any]:
    """Build the existing uef-gen project-config shape from target declarations."""
    target_value = _target_value(document)
    if not target_value: raise ValueError("A target ID is required before hardware configuration can be lowered.")
    config: dict[str, Any] = {"project": {"name": document.name}, "target": {}, "modules": []}
    if uetd_root is not None: config["uetd"] = {"path": str(uetd_root.resolve())}
    boards: dict[str, dict[str, Any]] = {}
    chip_database = None
    if uetd_root is not None:
        boards = load_development_boards(uetd_root)
        chip_database = load_chip_database(uetd_root)
    board_key = next((key for key in boards if key.casefold() == target_value.casefold()), None)
    board = boards.get(board_key) if board_key is not None else None
    if board is not None:
        config["target"]["development_board"] = board.get("id", board_key)
        config["target"]["chip"] = board.get("chip_id")
    elif chip_database is not None:
        canonical_chip = next((item for item in chip_database.names() if item.casefold() == target_value.casefold()), None)
        if canonical_chip is not None:
            record = chip_database.get(canonical_chip)
            config["target"]["chip"] = record.registry_id or record.name
        else:
            # Keep an exact unresolved name so the common target validator can
            # produce the canonical unknown-target diagnostic.
            config["target"]["chip"] = target_value
    else:
        # Keep an exact unresolved name so the common target validator can
        # produce the canonical unknown-target diagnostic.
        config["target"]["chip"] = target_value
    os_node = _one(document.declarations, "operatingsystem")
    runtime = _property_name(os_node, "runtime") if os_node else None
    if runtime and runtime.casefold() not in {"baremetal", "bare_metal"}:
        config["target"]["rtos"] = runtime
    numeric = _one(document.declarations, "numeric")
    if numeric:
        values = _property_map(numeric)
        arithmetic = {"f32": "float32", "f64": "float64", "q15": "q15", "q31": "q31"}.get(_name(values.get("scalar")))
        if arithmetic: config["target"]["arithmetic"] = arithmetic
        for key, value in values.items():
            if key == "matrix_layout": config.setdefault("numeric", {})[key] = _value(value)
            elif key in {"rounding", "overflow", "non_finite", "fused_multiply_add"}: config.setdefault("numeric", {})[key] = _value(value)
    module_ids: list[str] = []
    implementations = _implementation_bodies(document)
    for signature in _signatures(document):
        if signature.kind == "peripheral": continue
        binding = catalogue.dsl_descriptors.get(signature.registry_key or "")
        if binding and binding.module_id not in module_ids: module_ids.append(binding.module_id)
    if module_ids: config["modules"] = module_ids
    peripheral_requests: list[dict[str, Any]] = []
    for signature in _signatures(document):
        if signature.kind != "peripheral": continue
        body = implementations.get(signature.name or "")
        props = _property_map(body) if body else {}
        physical_instance = _name(props.get("instance"))
        if physical_instance:
            request: dict[str, Any] = {"instance": physical_instance, "type": signature.registry_key}
            pins = _value(props.get("pins"))
            if isinstance(pins, dict): request["pins"] = pins
            for key in ("clock_hz", "dma", "irq_priority", "mode", "prescaler", "period"):
                if key in props: request[key] = _value(props[key])
            peripheral_requests.append(request)
        elif body and body.span:
            source_map.add(f"hardware.peripherals.{signature.name}", body.span)
    if peripheral_requests: config["peripherals"] = peripheral_requests
    source_map.add("project.name", document.span)
    target_decl = _one(document.declarations, "target")
    if target_decl and target_decl.span: source_map.add("target", target_decl.span)
    return config

def lower_application_spec(document: ProjectDocument, catalogue: AuthoringCatalogue, source_map: SourceMap,
                           *, imported_contracts: tuple[ImportedSystemContract, ...] = (),
                           board_id: str | None = None) -> StandaloneApplicationSpec:
    """Create the typed, named-value application model consumed by UETD's backend."""
    bound = bind_signatures_to_implementations(document)
    components: list[ComponentSpec] = []
    models: list[ModelSpec] = []
    algorithms: list[CustomAlgorithmSpec] = []
    hybrids = []
    subsystems: list[SubsystemSpec] = []
    protocols: list[ProtocolSpec] = []
    globals_: dict[str, Any] = {}
    for body in bound:
        if body.kind in {"algorithm", "sensor", "actuator", "protocol", "bus", "peripheral", "module"}:
            components.append(lower_component(body, catalogue, source_map))
            if body.kind == "protocol": protocols.append(_lower_native_protocol(body, catalogue))
        elif body.kind == "model": models.append(lower_model(body, source_map))
        elif body.kind == "custom_algorithm": algorithms.append(lower_custom_algorithm(body, catalogue, source_map))
        elif body.kind == "hybrid_algorithm": hybrids.append(lower_hybrid_algorithm(body, catalogue, source_map))
        elif body.kind == "subsystem": subsystems.append(lower_subsystem(body, source_map))
        elif body.kind == "custom_protocol": protocols.append(_lower_custom_protocol(body, source_map))
    implementation = _one(document.declarations, "implementation_block")
    if implementation:
        for child in implementation.children:
            if child.kind == "signal" and child.bindings:
                binding = child.bindings[0]
                globals_[child.name or ""] = {"type": _type_descriptor(binding.type_spec), "value": _expression(binding.value) if binding.value is not None else None, "source_span": _span(child)}
    os_node = _one(document.declarations, "operatingsystem")
    if os_node is None: raise ValueError("operatingsystem declaration is required for lowering")
    operating_system = lower_operatingsystem(os_node, catalogue, source_map)
    target_id = _target_value(document) or ""
    records = tuple(RecordSpec(item.name, item.fields, item.span) for item in document.records)
    enums: dict[str, EnumSpec] = {}
    for item in document.enums:
        next_value = 0; values: list[EnumValueSpec] = []
        for variant in item.variants:
            value = variant.value if variant.value is not None else next_value
            values.append(EnumValueSpec(variant.name, value, variant.span)); next_value = value + 1
        enums[item.name] = EnumSpec(item.name, item.storage_type, tuple(values), item.span)
    variants = tuple(_lower_variant_spec(item) for item in document.variants)
    flags = tuple(FlagsSpec(item, sum(1 << member.bit for member in item.members), item.span) for item in document.flags)
    fixed = tuple(FixedPointSpec(item, 1, 1 << item.fractional_bits, -(1 << (_width(item.storage_type) - 1)), (1 << (_width(item.storage_type) - 1)) - 1, item.span) for item in document.fixed_point_types)
    boundary_records = tuple(BoundaryRecordSpec(RecordSpec(item.name, item.fields, item.span), item.boundary,
                                                None, item.boundary.alignment, item.span)
                             for item in document.records if item.boundary is not None)
    imports = tuple(lower_contract_imports(imported_contracts))
    project_routines = tuple(
        ProjectRoutineSpec(
            name=item.name or "",
            owner_component=item.registry_key,
            return_type=(item.signature_type or "void()").split("(", 1)[0],
            execution_context=_property_name(item, "context") or "task",
            source_span=item.span,
        )
        for item in document.declarations
        if item.kind == "project_routine" and item.prototype
    )
    for routine in project_routines:
        source_map.add(f"project_routines.{routine.qualified_name}", routine.source_span)
    numeric = _property_map(_one(document.declarations, "numeric"))
    target = _target_value(document)
    app = StandaloneApplicationSpec(
        project_name=document.name, components=tuple(components), models=tuple(models), custom_algorithms=tuple(algorithms),
        subsystems=tuple(subsystems), operating_system=operating_system, target_id=target or "", project_parameters=document.parameters,
        board_id=board_id, numeric_profile={key: _value(value) for key, value in numeric.items()}, records=records,
        hybrid_algorithms=tuple(hybrids), protocols=tuple(protocols), enums=enums, contract_imports=imports,
        variants=variants, flags_types=flags, fixed_point_types=fixed, boundary_records=boundary_records,
        project_routines=project_routines, type_interchange=TypeInterchangeManifest(), globals=globals_, source_span=document.span,
    )
    source_map.add("application", document.span)
    return app

def bind_signatures_to_implementations(document: ProjectDocument) -> tuple[Declaration, ...]:
    """Copy canonical constructor identity from each header onto its body."""
    signatures = {item.name: item for item in _signatures(document) if item.name}
    implementation = _one(document.declarations, "implementation_block")
    if implementation is None: return ()
    result: list[Declaration] = []
    for body in implementation.children:
        if body.kind == "signal": continue
        signature = signatures.get(body.name)
        if signature is None: raise ValueError(f"Implementation {body.name!r} has no constructor signature")
        if signature.kind != body.kind: raise ValueError(f"Implementation role {body.kind!r} does not match signature role {signature.kind!r} for {body.name!r}")
        result.append(_replace_declaration(body, registry_key=signature.registry_key, signature_type=signature.signature_type))
    return tuple(result)

def lower_component(declaration: Declaration, catalogue: AuthoringCatalogue, source_map: SourceMap) -> ComponentSpec:
    """Resolve one UETD-backed declaration and retain authored config/ports."""
    descriptor = declaration.registry_key or ""
    binding = catalogue.dsl_descriptors.get(descriptor)
    registry_id = binding.module_id if binding else descriptor
    kind = binding.contract.kind if binding and binding.contract else declaration.kind
    properties = {item.name: _expression(item.value) for item in declaration.properties}
    inputs: dict[str, Any] = {}
    outputs: dict[str, TypeSpec] = {}
    contract = binding.contract if binding else None
    if contract:
        inputs = {str(item.get("name")): item for item in contract.inputs if isinstance(item.get("name"), str)}
        outputs = {str(item.get("name")): TypeSpec(str(item.get("type", "<unresolved>"))) for item in contract.outputs if isinstance(item.get("name"), str)}
    if declaration.span: source_map.add(f"components.{declaration.name}", declaration.span)
    output_assignments = tuple(CompositionOutputSpec(item.name, _expression(item.expression), item.span) for item in declaration.output_assignments)
    nested = tuple(_declaration_ir(child) for child in declaration.children)
    return ComponentSpec(
        name=declaration.name or "",
        kind=kind,
        registry_id=registry_id,
        properties=properties,
        inputs=inputs,
        outputs=outputs,
        members=tuple(child.name or "" for child in declaration.children if child.kind == "use"),
        source_span=declaration.span,
        bindings=declaration.bindings,
        output_assignments=output_assignments,
        nested_declarations=nested,
        uetd_contract=to_json_value(contract) if contract is not None else {},
    )

def lower_model(declaration: Declaration, source_map: SourceMap) -> ModelSpec:
    """Preserve model declarations, matrices, typed bindings, and equation ASTs."""
    matrix_map = {
        child.kind: {key: _expression(value) for key, value in _property_map(child).items()}
        for child in declaration.children if child.kind in {"linear", "jacobians"}
    }
    properties = _property_map(declaration)
    sample = _quantity_seconds(properties.get("sample_time"))
    inputs = tuple(item for item in declaration.bindings if item.role == "input")
    outputs = tuple(item for item in declaration.bindings if item.role == "output")
    states = tuple(item for item in declaration.bindings if item.role == "state")
    parameters = tuple(item for item in declaration.bindings if item.role == "parameter")
    discretization = {key: _model_property_value(value) if key == "discretize" else _value(value)
                      for key, value in properties.items() if key in {"discretize", "state_order", "input_order", "output_order"}}
    if declaration.span: source_map.add(f"models.{declaration.name}", declaration.span)
    return ModelSpec(declaration.name or "", declaration.signature_type or "continuous", sample, states, inputs, outputs, parameters,
                     matrix_map.get("linear", {}), tuple(item for item in declaration.equations if isinstance(item, Equation)),
                     matrix_map.get("jacobians", {}), discretization, {}, declaration.span, declaration.model_form)

def lower_custom_algorithm(declaration: Declaration, catalogue: AuthoringCatalogue, source_map: SourceMap) -> CustomAlgorithmSpec:
    """Lower custom algorithm fields and ordered bounded equation stages."""
    del catalogue
    bindings = declaration.bindings
    props = _property_map(declaration)
    if declaration.span: source_map.add(f"custom_algorithms.{declaration.name}", declaration.span)
    return CustomAlgorithmSpec(declaration.name or "", declaration.signature_type or "custom", _name(props.get("model")),
                               tuple(item for item in bindings if item.role == "parameter"),
                               tuple(item for item in bindings if item.role == "state"),
                               tuple(item for item in bindings if item.role == "input"),
                               tuple(item for item in bindings if item.role == "output"),
                               tuple(item for item in bindings if item.role == "local"), declaration.stages,
                               {}, declaration.span)

def lower_bounded_loop(statement: BoundedLoop, catalogue: AuthoringCatalogue) -> dict[str, Any]:
    """Represent a statically bounded loop as a typed backend node."""
    del catalogue
    return {"kind": "for", "index": statement.variable, "start": _expression(statement.start), "stop_exclusive": _expression(statement.stop_exclusive),
            "body": [_statement(item) for item in statement.body], "source_span": _span(statement)}

def lower_conditional_statement(statement: ConditionalStatement, catalogue: AuthoringCatalogue) -> dict[str, Any]:
    """Represent a boolean branch with ordered, independently scoped bodies."""
    del catalogue
    return {"kind": "if", "condition": _expression(statement.condition), "then": [_statement(item) for item in statement.then_body],
            "else": [_statement(item) for item in statement.else_body], "source_span": _span(statement)}

def lower_math_expression(expression: Any, catalogue: AuthoringCatalogue) -> dict[str, Any]:
    """Serialize the safe expression AST; retain DSL names, never raw code."""
    if isinstance(expression, MathCallExpression):
        contract = _ACTIVE_UMATH_BINDINGS.get().get(id(expression))
        if contract is None: raise ValueError(f"UMATH call {expression.callee!r} has no type-resolved registry binding")
        return _lower_resolved_math_call(expression, contract)
    return _expression(expression)

def lower_unit_conversion(expression: UnitConversionExpression, catalogue: AuthoringCatalogue) -> dict[str, Any]:
    """Preserve an explicit registered-unit conversion for checked codegen."""
    if expression.target_unit not in catalogue.units and expression.target_unit != "1": raise ValueError(f"Unit {expression.target_unit!r} is not registered")
    return {"kind": "convert_unit", "target_unit": expression.target_unit, "value": _expression(expression.value), "source_span": _span(expression)}

def lower_hybrid_algorithm(declaration: Declaration, catalogue: AuthoringCatalogue, source_map: SourceMap):
    """Lower a language-owned algorithm composition without inventing a UETD ID."""
    members = tuple((child.name or "", child.registry_key or "") for child in declaration.children if child.kind == "use")
    inputs = tuple(_port(item, "input") for item in declaration.bindings if item.role == "input")
    outputs = tuple(_port(item, "output") for item in declaration.bindings if item.role == "output")
    stages = tuple(_composition_stage(item) for item in declaration.composition_stages)
    locals_ = tuple(CompositionLocalSpec(item.name, item.type_spec, _expression(item.expression), item.span) for item in declaration.composition_locals)
    output_expressions = tuple(CompositionOutputSpec(item.name, _expression(item.expression), item.span) for item in declaration.composition_outputs)
    choices = {
        child.name or "": [
            _lower_choice_branch(prop) for prop in child.properties
        ]
        for child in declaration.children if child.kind == "choose"
    }
    properties = _property_map(declaration)
    if declaration.span: source_map.add(f"hybrid_algorithms.{declaration.name}", declaration.span)
    from uef_gen.language.frontends.project.application_spec import HybridAlgorithmSpec
    return HybridAlgorithmSpec(declaration.name or "", inputs, outputs, members, stages, locals_, output_expressions, choices,
                              _value(properties.get("failure_policy")), _quantity_seconds(properties.get("execution_budget")),
                              declaration.span, _composition_execution_body(declaration))

def _lower_choice_branch(branch: Any) -> dict[str, Any]:
    """Keep a hybrid selection's predicate and selected value as separate IR."""
    if branch.name == "when" and isinstance(branch.value, RecordValue):
        values = dict(branch.value.fields)
        return {"kind": "when", "condition": _expression(values.get("condition")),
                "value": _expression(values.get("value"))}
    return {"kind": "otherwise", "value": _expression(branch.value)}

def lower_subsystem(declaration: Declaration, source_map: SourceMap) -> SubsystemSpec:
    """Lower a typed function-like component wrapper and its explicit calls."""
    members = tuple((child.name or "", child.registry_key or "") for child in declaration.children if child.kind == "use")
    inputs = tuple(_port(item, "input") for item in declaration.bindings if item.role == "input")
    outputs = tuple(_port(item, "output") for item in declaration.bindings if item.role == "output")
    stages = tuple(_composition_stage(item) for item in declaration.composition_stages)
    locals_ = tuple(CompositionLocalSpec(item.name, item.type_spec, _expression(item.expression), item.span) for item in declaration.composition_locals)
    outputs_ = tuple(CompositionOutputSpec(item.name, _expression(item.expression), item.span) for item in declaration.composition_outputs)
    if declaration.span: source_map.add(f"subsystems.{declaration.name}", declaration.span)
    return SubsystemSpec(declaration.name or "", members, inputs, outputs, stages, locals_, outputs_,
                         {key: _value(value) for key, value in _property_map(declaration).items()},
                         declaration.span, _composition_execution_body(declaration))

def lower_state_machine(declaration: Declaration, catalogue: AuthoringCatalogue, source_map: SourceMap) -> StateMachineSpec:
    """Lower a single-owner FSM with source-ordered snapshot transitions."""
    del catalogue
    props = _property_map(declaration); initial = _name(props.get("initial")) or ""
    owner = _name(props.get("owner")) or ""
    consumers = _name_list(props.get("consumers"))
    transitions = []
    actions = []
    for child in declaration.children:
        if child.kind == "transition":
            guard = next((prop.value for prop in child.properties if prop.name == "when"), None)
            transitions.append(StateTransitionSpec(child.name or "", child.registry_key or "", _expression(guard), child.span))
        elif child.kind == "state_action":
            operation = next((prop.value for prop in child.properties if prop.name == "operation"), None)
            if isinstance(operation, Reference):
                actions.append(StateActionSpec(child.registry_key or "on_enter", child.name or "", ".".join(operation.path), child.span))
    if declaration.span: source_map.add(f"state_machines.{declaration.name}", declaration.span)
    declared_states = [child.name or "" for child in declaration.children if child.kind == "machine_state"]
    states = tuple(dict.fromkeys(declared_states or [initial, *(item.source_state for item in transitions),
                                                     *(item.destination_state for item in transitions)]))
    transition_capacity = sum(len(states) if item.source_state == "any" else 1 for item in transitions)
    return StateMachineSpec(declaration.name or "", declaration.registry_key or "", initial,
                            states, owner, consumers, tuple(transitions), len(states), transition_capacity, tuple(actions),
                            evaluation_boundary="after_owner_run",
                            max_transitions_per_activation=1,
                            state_delivery="latest_snapshot_per_consumer",
                            consumer_activation="scheduled_independently",
                            source_span=declaration.span,
                            properties={key: _value(value) for key, value in props.items()
                                        if key not in {"initial", "owner", "consumers"}})

def lower_channel(declaration: Declaration) -> ChannelSpec:
    """Lower a validated channel into a typed, explicitly subscribed route."""
    props = _property_map(declaration)
    source = _endpoint_name(props.get("source")) or ""
    destination = _endpoint_name(props.get("destination"))
    producer = _name(props.get("producer")) or ""
    consumers = _name_list(props.get("consumers"))
    # Preserve symbolic, statically typed capacities from project parameters.
    # Collapsing these to zero would make the lowered spec disagree with the
    # validated source and could turn a bounded mailbox into an unusable one.
    capacity = _value(props.get("capacity"))
    overflow = _name(props.get("overflow")) or ""
    ownership = _name(props.get("ownership")) or ""
    memory_region = _name(props.get("memory_region"))
    typed = {"source", "destination", "producer", "consumers", "capacity", "overflow", "ownership",
             "maximum_age", "memory_region"}
    return ChannelSpec(
        declaration.name or "", source, producer, consumers, capacity, overflow, ownership,
        destination=destination,
        maximum_age=_expression(props.get("maximum_age")) if "maximum_age" in props else None,
        memory_region=memory_region,
        properties={key: _value(value) for key, value in props.items() if key not in typed},
        source_span=declaration.span,
    )

def lower_taskset(declaration: Declaration, catalogue: AuthoringCatalogue, source_map: SourceMap) -> TasksetSpec:
    """Lower grouped tasks and runtime-owned resource declarations."""
    tasks = tuple(_lower_scheduled(child) for child in declaration.children if child.kind == "task")
    interrupts = tuple(_declaration_dict(child) for child in declaration.children if child.kind == "interrupt")
    resources = tuple(_declaration_dict(child) for child in declaration.children if child.kind == "resource")
    constraints = tuple(_declaration_dict(child) for child in declaration.children if child.kind == "constraint")
    channels = tuple(lower_channel(child) for child in declaration.children if child.kind == "channel")
    if declaration.span: source_map.add(f"tasksets.{declaration.name}", declaration.span)
    return TasksetSpec(declaration.name or "", {key: _value(value) for key, value in _property_map(declaration).items()}, tasks, interrupts, resources, constraints, declaration.span, channels)

def lower_operatingsystem(declaration: Declaration, catalogue: AuthoringCatalogue, source_map: SourceMap) -> OperatingSystemSpec:
    """Lower runtime policy, direct work, tasksets, and owned state machines."""
    tasksets = tuple(lower_taskset(item, catalogue, source_map) for item in declaration.children if item.kind == "taskset")
    tasks = tuple(_lower_scheduled(item) for item in declaration.children if item.kind == "task")
    loops = tuple(_lower_scheduled(item, loop=True) for item in declaration.children if item.kind == "loop")
    machines = tuple(lower_state_machine(item, catalogue, source_map) for item in declaration.children if item.kind == "state_machine")
    get_dict = lambda kind: tuple(_declaration_dict(item) for item in declaration.children if item.kind == kind)
    channels = tuple(lower_channel(item) for item in declaration.children if item.kind == "channel")
    if declaration.span: source_map.add("operatingsystem", declaration.span)
    return OperatingSystemSpec(declaration.name or "", {key: _value(value) for key, value in _property_map(declaration).items()}, tasksets,
                              tasks, machines, get_dict("interrupt"), channels, get_dict("resource"),
                              get_dict("constraint"), declaration.span, loops)

def lower_project(document: ProjectDocument, catalogue: AuthoringCatalogue, *, uetd_root: Path | None = None,
                  imported_contracts: tuple[ImportedSystemContract, ...] = (),
                  resolved_math_bindings: dict[int, Any] | None = None,
                  resolved_math_shape_bindings: dict[int, dict[str, int]] | None = None) -> tuple[dict[str, Any], StandaloneApplicationSpec, SourceMap]:
    """Produce hardware configuration and application specification after validation."""
    token = _ACTIVE_UMATH_BINDINGS.set(resolved_math_bindings or {})
    shape_token = _ACTIVE_UMATH_SHAPES.set(resolved_math_shape_bindings or {})
    try:
        source_map = SourceMap()
        hardware = lower_hardware_configuration(document, catalogue, source_map, uetd_root=uetd_root)
        app = lower_application_spec(
            document,
            catalogue,
            source_map,
            imported_contracts=imported_contracts,
            board_id=hardware.get("target", {}).get("development_board"),
        )
        manifest = build_type_interchange_manifest(
            app, target_capabilities={}, source_map=source_map, catalogue=catalogue,
            document=document, resolved_math_bindings=resolved_math_bindings,
            resolved_math_shape_bindings=resolved_math_shape_bindings,
        )
        app = replace(app, type_interchange=manifest)
        return hardware, app, source_map
    finally:
        _ACTIVE_UMATH_BINDINGS.reset(token)
        _ACTIVE_UMATH_SHAPES.reset(shape_token)

def lower_contract_imports(imported_contracts: tuple[ImportedSystemContract, ...]) -> tuple[ContractImportSpec, ...]:
    """Copy the selected import view and digest into the application model."""
    values: list[ContractImportSpec] = []
    for item in imported_contracts:
        enums: list[EnumSpec] = []
        for enum in item.enums:
            next_value = 0; cases: list[EnumValueSpec] = []
            for case in enum.variants:
                value = case.value if case.value is not None else next_value
                cases.append(EnumValueSpec(case.name, value, case.span)); next_value = value + 1
            enums.append(EnumSpec(enum.name, enum.storage_type, tuple(cases), enum.span))
        values.append(ContractImportSpec(str(item.source_path), item.system_name, item.content_digest, item.local_node_name,
                                         item.records, tuple(enums), item.protocols, item.variants, item.flags_types,
                                         item.fixed_point_types, dict(item.local_endpoint_facts), item.local_links, item.source_span))
    return tuple(values)

def lower_type_spec(type_spec: TypeSpec, type_environment: TypeEnvironment) -> dict[str, Any]:
    """Encode a resolved type identity rather than emitting a source token as C."""
    resolved, diagnostics = __import__("uef_gen.language.frontends.project.type_interchange", fromlist=["resolve_type_spec"]).resolve_type_spec(type_spec, type_environment)
    errors = [item for item in diagnostics if item.severity == "error"]
    if errors or resolved is None: raise ValueError("Cannot lower unresolved type: " + "; ".join(item.message for item in errors))
    return {"owner": resolved.identity.owner, "canonical_name": resolved.identity.canonical_name, "c_name": resolved.identity.c_name,
            "arguments": [item.identity.canonical_name for item in resolved.arguments], "shape": list(resolved.shape),
            "unit": resolved.unit, "unit_dimensions": dict(resolved.unit_dimensions), "boundary": resolved.boundary,
            "storage_bits": resolved.storage_bits, "signed": resolved.signed, "source_name": type_spec.name,
            "c_declaration_kind": resolved.identity.c_kind if resolved.identity.owner == "uetd" else None,
            "source_header": resolved.identity.source_path if resolved.identity.owner == "uetd" else None,
            "language_contract": resolved.identity.language_contract if resolved.identity.owner == "uetd" else None,
            "readiness": resolved.identity.readiness if resolved.identity.owner == "uetd" else None}

def lower_variant(definition: VariantDefinition, resolved_variant: VariantSpec, source_map: SourceMap) -> dict[str, Any]:
    """Describe a tagged variant layout for a checked backend codec."""
    source_map.add(f"types.{definition.name}", definition.span)
    return {"kind": "variant", "name": definition.name, "tag_type": definition.tag_type,
            "tags": dict(resolved_variant.resolved_tags), "cases": [{"name": case.name, "fields": [_binding(field) for field in case.fields]} for case in definition.cases],
            "maximum_payload_size": resolved_variant.maximum_payload_size, "alignment": resolved_variant.alignment}

def lower_option_result(resolved_type: ResolvedType, source_map: SourceMap) -> dict[str, Any]:
    """Describe the fixed tag/payload representation of Option or Result."""
    if resolved_type.identity.canonical_name not in {"Option", "Result"}: raise ValueError("Expected Option<T> or Result<T,E>")
    return {"kind": resolved_type.identity.canonical_name.lower(), "tag_type": "u8", "constructors": ["None", "Some"] if resolved_type.identity.canonical_name == "Option" else ["Ok", "Err"],
            "payload_types": [item.identity.canonical_name for item in resolved_type.arguments], "checked_payload_access": True}

def lower_match_expression(expression: MatchExpression, typed_facts: TypedExpressionFacts, source_map: SourceMap) -> dict[str, Any]:
    """Lower exhaustive matches to ordered tag dispatch and scoped arms."""
    source_map.add(f"expressions.match@{expression.span.start.offset if expression.span else 0}", expression.span)
    return {"kind": "match", "subject": _expression(expression.subject),
            "arms": [{"case": arm.case_name, "binding": arm.binding_name, "fields": dict(arm.field_bindings), "value": _expression(arm.expression)} for arm in expression.arms],
            "default": _expression(expression.default_expression) if expression.default_expression is not None else None,
            "result_type": typed_facts.resolved_type.identity.canonical_name, "dispatch": "checked_tag"}

def lower_flags(definition: FlagsDefinition, resolved_flags: FlagsSpec, source_map: SourceMap) -> dict[str, Any]:
    """Lower a flags enum with its exact allowed-bit mask."""
    source_map.add(f"types.{definition.name}", definition.span)
    return {"kind": "flags", "name": definition.name, "storage_type": definition.storage_type,
            "members": {item.name: item.bit for item in definition.members}, "valid_mask": resolved_flags.valid_mask}

def lower_fixed_point(definition: FixedPointDefinition, resolved_fixed_type: FixedPointSpec, source_map: SourceMap) -> dict[str, Any]:
    """Lower fixed-point scale/range and arithmetic policy as explicit metadata."""
    source_map.add(f"types.{definition.name}", definition.span)
    return {"kind": "fixed_point", "name": definition.name, "storage_type": definition.storage_type,
            "fractional_bits": definition.fractional_bits, "scale": [resolved_fixed_type.scale_numerator, resolved_fixed_type.scale_denominator],
            "minimum": resolved_fixed_type.minimum_value, "maximum": resolved_fixed_type.maximum_value,
            "unit": definition.unit, "rounding": definition.rounding, "overflow": definition.overflow}

def lower_bounded_buffer(type_spec: TypeSpec, resolved_type: ResolvedType, source_map: SourceMap) -> dict[str, Any]:
    """Lower bounded byte/string storage with explicit length semantics."""
    if resolved_type.maximum_encoded_bytes is None: raise ValueError("Bounded buffer type has no validated byte capacity")
    return {"kind": type_spec.name.lower(), "capacity_bytes": resolved_type.maximum_encoded_bytes,
            "length_field": "byte_length", "terminator": False, "heap_allocation": False}

def lower_record_boundary(record: BoundaryRecordSpec, resolved_boundary: Any, source_map: SourceMap) -> dict[str, Any]:
    """Serialize explicit shared-memory or persistent layout facts."""
    source_map.add(f"boundary_records.{record.record.name}", record.source_span)
    return {"kind": record.boundary.kind, "record": record.record.name, "version": record.boundary.version,
            "schema_id": record.boundary.schema_id, "encoded_size": record.encoded_size, "alignment": record.alignment,
            "layout": resolved_boundary, "integrity": record.boundary.integrity, "migration_policy": record.boundary.migration_policy}

def lower_uetd_value_binding(binding: Any, resolved_contract: Any, source_map: SourceMap) -> dict[str, Any]:
    """Create a named argument binding tied to the owning UETD module contract."""
    name = getattr(binding, "name", "")
    return {"name": name, "type": _type_descriptor(getattr(binding, "type_spec", None)),
            "direction": getattr(binding, "role", "binding"), "contract": resolved_contract,
            "value": _expression(getattr(binding, "value", None)), "source_span": _span(binding)}

def lower_umath_call(expression: MathCallExpression, typed_binding: Any, source_map: SourceMap) -> dict[str, Any]:
    """Lower one UMATH call through a validated hidden C binding."""
    binding = getattr(typed_binding, "dsl_binding_contract", None)
    if not isinstance(binding, dict): raise ValueError(f"UMATH function {expression.callee!r} has no approved DSL binding")
    source_map.add(f"umath.{expression.callee}", expression.span)
    return {"kind": "umath_call", "dsl_name": expression.callee, "binding": binding,
            "arguments": [_expression(item) for item in expression.arguments]}

def lower_protocol_codec(protocol: Any, resolved_types: Any, source_map: SourceMap) -> dict[str, Any]:
    """Produce a bounded protocol codec description from validated fields."""
    return {"protocol": getattr(protocol, "name", ""), "transport": getattr(protocol, "transport", None),
            "messages": resolved_types, "framing": getattr(protocol, "framing", {}), "checksum": getattr(protocol, "checksum", {})}

def lower_shared_memory_layout(record: Any, resolved_layout: Any, source_map: SourceMap) -> dict[str, Any]:
    """Keep shared-memory offsets and publication policy separate from wire layout."""
    return {"record": getattr(record, "name", ""), "boundary": "shared", "layout": resolved_layout}

def lower_persistent_codec(record: Any, resolved_schema: Any, source_map: SourceMap) -> dict[str, Any]:
    """Build a versioned persistent codec descriptor from a validated schema."""
    return {"record": getattr(record, "name", ""), "boundary": "persistent", "schema": resolved_schema}

def build_type_interchange_manifest(application_spec: StandaloneApplicationSpec, *, target_capabilities: dict[str, Any],
                                    source_map: SourceMap, catalogue: AuthoringCatalogue | None = None,
                                    document: ProjectDocument | None = None,
                                    resolved_math_bindings: dict[int, Any] | None = None,
                                    resolved_math_shape_bindings: dict[int, dict[str, int]] | None = None) -> TypeInterchangeManifest:
    """Collect stable type and exact registry-backed call routes for backend validation."""
    ids: dict[str, str] = {}
    spans = dict(source_map.entries)
    for record in application_spec.records: ids[record.name] = f"project:{application_spec.project_name}:{record.name}"
    for name in application_spec.enums: ids[name] = f"project:{application_spec.project_name}:{name}"
    for item in application_spec.variants: ids[item.definition.name] = f"project:{application_spec.project_name}:{item.definition.name}"
    for item in application_spec.flags_types: ids[item.definition.name] = f"project:{application_spec.project_name}:{item.definition.name}"
    for item in application_spec.fixed_point_types: ids[item.definition.name] = f"project:{application_spec.project_name}:{item.definition.name}"
    for imported in application_spec.contract_imports:
        for record in imported.shared_records:
            ids[record.name] = f"system:{imported.system_name}:{record.name}"
        for enum in imported.shared_enums:
            ids[enum.name] = f"system:{imported.system_name}:{enum.name}"
        for variant in imported.shared_variants:
            ids[variant.name] = f"system:{imported.system_name}:{variant.name}"
        for flags in imported.shared_flags:
            ids[flags.name] = f"system:{imported.system_name}:{flags.name}"
        for fixed in imported.shared_fixed_point_types:
            ids[fixed.name] = f"system:{imported.system_name}:{fixed.name}"
        for protocol in imported.shared_protocols:
            ids[protocol.name] = f"system:{imported.system_name}:protocol:{protocol.name}"
    for protocol in application_spec.protocols:
        ids[protocol.name] = f"project:{application_spec.project_name}:protocol:{protocol.name}"
    routes = {name: (target_capabilities[name] if isinstance(target_capabilities.get(name), bool) else None)
              for name in sorted({"module_abi", "umath_abi", "wire", "shared_memory", "persistent"})}
    umath_routes: dict[str, dict[str, Any]] = {}
    resolved_math_bindings = resolved_math_bindings or {}
    resolved_math_shape_bindings = resolved_math_shape_bindings or {}
    uetd_type_bindings: dict[str, dict[str, Any]] = {}
    if catalogue is not None and document is not None:
        def add_type_reference(name: str) -> None:
            direct = list(catalogue.types.get(name, ()))
            matches = direct or [item for entries in catalogue.types.values() for item in entries if item.dsl_name == name]
            if len(matches) != 1:
                return
            item = matches[0]
            if item.language_contract not in {"numeric", "enum", "opaque"} or item.source_status != "public_header":
                return
            uetd_type_bindings[item.c_name] = {
                "c_name": item.c_name,
                "dsl_name": item.dsl_name,
                "declaration_kind": item.kind,
                "header": item.header,
                "language_contract": item.language_contract,
                "source_status": item.source_status,
            }

        for value in _walk_dataclass_values(document):
            if isinstance(value, TypeSpec):
                add_type_reference(value.name)
        for declaration in _walk_dataclass_values(document):
            if not isinstance(declaration, Declaration) or not declaration.registry_key:
                continue
            binding = catalogue.dsl_descriptors.get(declaration.registry_key)
            if binding is None or binding.contract is None:
                continue
            for port in (*binding.contract.inputs, *binding.contract.outputs):
                if isinstance(port.get("type"), str):
                    add_type_reference(port["type"])

        for value in _walk_dataclass_values(document):
            if not isinstance(value, (MathCallExpression, BinaryExpression)):
                continue
            contract = resolved_math_bindings.get(id(value))
            if contract is None:
                continue
            umath_routes[_math_route_key(value)] = {
                "source_span": value.span,
                "source_name": value.callee if isinstance(value, MathCallExpression) else value.operator,
                "source_kind": "function" if isinstance(value, MathCallExpression) else "operator",
                "dsl_name": contract.dsl_name,
                "shape_bindings": resolved_math_shape_bindings.get(id(value), {}),
                "dsl_signature": contract.dsl_signature,
                "c_name": contract.c_name,
                "module_id": contract.module_id,
                "header": contract.header,
                "c_signature": contract.c_signature,
                "parameters": contract.parameters,
                "binding": contract.dsl_binding_contract,
                "declaration_kind": contract.declaration_kind,
                "implementation_status": contract.implementation_status,
                "binding_status": contract.dsl_binding_status,
                "available_for_generation": contract.available_for_generation,
                "callable_contexts": list(contract.callable_contexts),
            }
    return TypeInterchangeManifest(type_ids=ids, target_support=routes,
                                   umath_binding_routes=umath_routes, uetd_type_bindings=uetd_type_bindings,
                                   source_spans=spans)

def _walk_dataclass_values(value: Any):
    """Yield nested dataclass values while skipping non-semantic source metadata."""
    if isinstance(value, (str, bytes, int, float, bool, type(None), Path)):
        return
    if isinstance(value, dict):
        for child in value.values():
            yield from _walk_dataclass_values(child)
        return
    if isinstance(value, (tuple, list)):
        for child in value:
            yield from _walk_dataclass_values(child)
        return
    if not is_dataclass(value):
        return
    yield value
    for item in fields(value):
        if item.name in {"span", "source_path", "path", "start", "end"}:
            continue
        yield from _walk_dataclass_values(getattr(value, item.name))

def _math_route_key(expression: Any) -> str:
    """Use source identity so calls sharing an overloaded DSL name route independently."""
    span = expression.span
    source = str(span.path.resolve()) if span.path is not None else "<memory>"
    return f"{source}:{span.start.offset}-{span.end.offset}"

def _lower_resolved_math_call(expression: MathCallExpression, contract: Any) -> dict[str, Any]:
    """Emit the exact UETD binding selected by semantic overload resolution."""
    return _lower_resolved_math_operation(expression, contract, source_name=expression.callee,
                                          arguments=expression.arguments)

def _lower_resolved_math_operation(expression: Any, contract: Any, *, source_name: str,
                                   arguments: tuple[Any, ...], operator: str | None = None) -> dict[str, Any]:
    """Emit an exact function/operator route with its authored source operands."""
    return {
        "kind": "umath_call",
        "dsl_name": contract.dsl_name,
        "source_name": source_name,
        "operator": operator,
        "c_name": contract.c_name,
        "binding": contract.dsl_binding_contract,
        "shape_bindings": _ACTIVE_UMATH_SHAPES.get().get(id(expression), {}),
        "arguments": [_expression(item) for item in arguments],
        "source_span": _span(expression),
    }

def _lower_scheduled(declaration: Declaration, *, loop: bool = False):
    run = next((child for child in declaration.children if child.kind == "run"), None)
    invocation = TaskInvocationSpec(run.name or "", tuple(PortBindingSpec(item.name, _expression(item.value), item.span) for item in run.properties) if run else (), run.span if run else declaration.span)
    properties = {key: _value(value) for key, value in _property_map(declaration).items()}
    if loop: return LoopSpec(declaration.name or "", invocation, properties, declaration.span)
    return TaskSpec(declaration.name or "", invocation, properties, declaration.span)

def _lower_native_protocol(declaration: Declaration, catalogue: AuthoringCatalogue) -> ProtocolSpec:
    descriptor = catalogue.dsl_descriptors.get(declaration.registry_key or "")
    contract = descriptor.contract if descriptor else None
    return ProtocolSpec(declaration.name or "", descriptor.module_id if descriptor else declaration.registry_key,
                        None, {}, {}, {}, {}, (), {}, {key: _expression(value) for key, value in _property_map(declaration).items()}, declaration.span,
                        sections={"bindings": [_binding(item) for item in declaration.bindings],
                                  "children": [_declaration_ir(item) for item in declaration.children],
                                  "uetd_contract": to_json_value(contract) if contract is not None else {}})

def _lower_custom_protocol(declaration: Declaration, source_map: SourceMap) -> ProtocolSpec:
    children = list(declaration.children)
    categories = {child.kind: _property_map(child) for child in children if child.kind in {"framing", "header", "sequence", "checksum", "receiver", "payload", "behavior"}}
    messages: list[ProtocolMessageSpec] = []
    for message in (child for child in children if child.kind == "message"):
        props = _property_map(message)
        fields = tuple(
            ProtocolFieldSpec(field.name or "", field.bindings[0].type_spec if field.bindings and field.bindings[0].type_spec else TypeSpec("<missing>"),
                              {key: _value(value) for key, value in _property_map(field).items()}, field.span)
            for field in message.children if field.kind == "field"
        )
        fragments = tuple({
            "index": int(fragment.name) if fragment.name and fragment.name.isdecimal() else fragment.name,
            "fields": [{"name": field.name, "type": _type_descriptor(field.bindings[0].type_spec if field.bindings else None),
                        "properties": {key: _value(value) for key, value in _property_map(field).items()}}
                       for field in fragment.children if field.kind == "field"],
            "properties": {key: _value(value) for key, value in _property_map(fragment).items()},
        } for fragment in message.children if fragment.kind == "fragment")
        flags = tuple({"name": child.name, "field": child.registry_key,
                       "bit": int(child.signature_type) if child.signature_type is not None else None,
                       "source_span": _span(child)}
                      for child in message.children if child.kind == "protocol_flag")
        msg_id = _literal_int(props.get("id"))
        messages.append(ProtocolMessageSpec(message.name or "", msg_id, fields, fragments,
                                            {key: _value(value) for key, value in props.items() if key != "id"},
                                            message.span, flags))
    if declaration.span: source_map.add(f"protocols.{declaration.name}", declaration.span)
    props = _property_map(declaration)
    return ProtocolSpec(declaration.name or "", None, _name(props.get("transport")),
                        {key: _value(value) for key, value in categories.get("framing", {}).items()},
                        {key: _value(value) for key, value in categories.get("header", {}).items()},
                        {key: _value(value) for key, value in categories.get("sequence", {}).items()},
                        {key: _value(value) for key, value in categories.get("checksum", {}).items()},
                        tuple(messages), {key: _value(value) for key, value in categories.get("receiver", {}).items()},
                        {key: _value(value) for key, value in props.items()}, declaration.span,
                        {key: _value(value) for key, value in categories.get("payload", {}).items()},
                        {key: _value(value) for key, value in categories.get("behavior", {}).items()},
                        {child.kind + (":" + child.name if child.name else ""): _declaration_ir(child)
                         for child in children if child.kind not in {"message", "framing", "header", "sequence", "checksum", "receiver", "payload", "behavior"}})

def _composition_stage(stage: CompositionStage) -> ComponentCallSpec:
    return ComponentCallSpec(stage.name, stage.member, tuple(PortBindingSpec(name, _expression(value), getattr(value, "span", None)) for name, value in stage.arguments),
                             _expression(stage.guard) if stage.guard else None,
                             _expression(stage.on_failure) if stage.on_failure else None, stage.span)

def _port(binding, direction: str) -> PortContractSpec:
    props = {prop.name: prop.value for prop in binding.properties}
    return PortContractSpec(binding.name, binding.type_spec or TypeSpec("<missing>"), direction,
                            _quantity_hz(props.get("rate")), _quantity_seconds(props.get("max_age")), _name(props.get("clock_domain")), binding.span)

def _statement(statement: Any) -> dict[str, Any]:
    if isinstance(statement, Equation): return {"kind": statement.mode, "target": _expression(statement.target), "value": _expression(statement.expression), "source_span": _span(statement)}
    if isinstance(statement, BoundedLoop): return {"kind": "for", "index": statement.variable, "start": _expression(statement.start), "stop_exclusive": _expression(statement.stop_exclusive), "body": [_statement(item) for item in statement.body], "source_span": _span(statement)}
    if isinstance(statement, ConditionalStatement): return {"kind": "if", "condition": _expression(statement.condition), "then": [_statement(item) for item in statement.then_body], "else": [_statement(item) for item in statement.else_body], "source_span": _span(statement)}
    return {"kind": type(statement).__name__, "source_span": _span(statement)}

def _expression(expression: Any) -> Any:
    if expression is None: return None
    if isinstance(expression, Literal): return {"kind": "literal", "value": expression.value, "unit": expression.unit, "source_span": _span(expression)}
    if isinstance(expression, Identifier): return {"kind": "identifier", "name": expression.value, "source_span": _span(expression)}
    if isinstance(expression, Reference): return {"kind": "reference", "path": list(expression.path), "source_span": _span(expression)}
    if isinstance(expression, BinaryExpression):
        contract = _ACTIVE_UMATH_BINDINGS.get().get(id(expression))
        if contract is not None:
            return _lower_resolved_math_operation(
                expression,
                contract,
                source_name=expression.operator,
                arguments=(expression.left, expression.right),
                operator=expression.operator,
            )
        return {"kind": "binary", "operator": expression.operator, "left": _expression(expression.left), "right": _expression(expression.right), "source_span": _span(expression)}
    if isinstance(expression, UnaryExpression): return {"kind": "unary", "operator": expression.operator, "value": _expression(expression.operand), "source_span": _span(expression)}
    if isinstance(expression, MathCallExpression):
        contract = _ACTIVE_UMATH_BINDINGS.get().get(id(expression))
        if contract is not None:
            return _lower_resolved_math_call(expression, contract)
        return {"kind": "call", "callee": expression.callee, "arguments": [_expression(item) for item in expression.arguments], "source_span": _span(expression)}
    if isinstance(expression, CastExpression): return {"kind": "cast", "type": _type_descriptor(expression.destination), "value": _expression(expression.value), "source_span": _span(expression)}
    if isinstance(expression, UnitConversionExpression): return {"kind": "convert_unit", "unit": expression.target_unit, "value": _expression(expression.value), "source_span": _span(expression)}
    if isinstance(expression, IndexExpression): return {"kind": "index", "value": _expression(expression.value), "indices": [{"start": _expression(sub.start), "end": _expression(sub.end)} for sub in expression.subscripts], "source_span": _span(expression)}
    if isinstance(expression, ListValue): return {"kind": "vector", "items": [_expression(item) for item in expression.items], "source_span": _span(expression)}
    if isinstance(expression, MatrixLiteral): return {"kind": "matrix", "rows": [[_expression(item) for item in row] for row in expression.rows], "source_span": _span(expression)}
    if isinstance(expression, RecordValue): return {"kind": "record", "fields": {name: _expression(item) for name, item in expression.fields}, "source_span": _span(expression)}
    if isinstance(expression, SelectValue): return {"kind": "select", "condition": _expression(expression.condition), "if_true": _expression(expression.if_true), "if_false": _expression(expression.if_false), "source_span": _span(expression)}
    if isinstance(expression, MatchExpression): return {"kind": "match", "subject": _expression(expression.subject), "arms": [{"case": arm.case_name, "binding": arm.binding_name, "fields": dict(arm.field_bindings), "value": _expression(arm.expression), "source_span": _span(arm)} for arm in expression.arms], "default": _expression(expression.default_expression), "source_span": _span(expression)}
    if isinstance(expression, VariantConstructorExpression): return {"kind": "variant_constructor", "constructor": expression.constructor, "value": _expression(expression.positional_value), "fields": {name: _expression(value) for name, value in expression.fields}, "source_span": _span(expression)}
    if isinstance(expression, FlagsOperationExpression): return {"kind": "flags_operation", "operation": expression.operation, "value": _expression(expression.value), "member": expression.member, "source_span": _span(expression)}
    raise TypeError(f"No lowering rule for expression node {type(expression).__name__}")

def _type_descriptor(type_spec: TypeSpec | None) -> dict[str, Any] | None:
    if type_spec is None: return None
    return {"name": type_spec.name, "shape": list(type_spec.shape), "unit": type_spec.unit,
            "arguments": [_type_descriptor(item) for item in type_spec.type_arguments]}

def _binding(binding: Any) -> dict[str, Any]:
    return {"name": binding.name, "type": _type_descriptor(binding.type_spec), "role": binding.role,
            "value": _expression(binding.value), "source_span": _span(binding)}

def _declaration_dict(declaration: Declaration) -> dict[str, Any]:
    return _declaration_ir(declaration)

def _declaration_ir(declaration: Declaration) -> dict[str, Any]:
    """Serialize a nested declaration without discarding bindings or children."""
    return {"kind": declaration.kind, "name": declaration.name, "registry_id": declaration.registry_key,
            "signature_type": declaration.signature_type,
            "properties": {key: _expression(value) for key, value in _property_map(declaration).items()},
            "bindings": [_binding(item) for item in declaration.bindings],
            "output_assignments": [{"name": item.name, "expression": _expression(item.expression), "source_span": item.span} for item in declaration.output_assignments],
            "children": [_declaration_ir(item) for item in declaration.children],
            "source_span": _span(declaration)}

def _composition_execution_body(declaration: Declaration) -> tuple[dict[str, Any], ...]:
    """Preserve the authored interleaving of local values, calls, and outputs."""
    entries: list[tuple[int, int, dict[str, Any]]] = []
    ordinal = 0
    for item in (*declaration.composition_locals, *declaration.composition_stages, *declaration.composition_outputs):
        span = getattr(item, "span", None)
        # Stable fallback order is only relevant for synthetic AST nodes. Parsed
        # nodes always carry source spans, which define the actual sequence.
        offset = span.start.offset if span is not None else 2**63 + ordinal
        if isinstance(item, CompositionLocal):
            value = {"kind": "let", "name": item.name, "type": _type_descriptor(item.type_spec),
                     "expression": _expression(item.expression), "source_span": span}
        elif isinstance(item, CompositionStage):
            value = {"kind": "stage", **_composition_stage_ir(item)}
        else:
            value = {"kind": "output", "name": item.name, "expression": _expression(item.expression), "source_span": span}
        entries.append((offset, ordinal, value))
        ordinal += 1
    return tuple(value for _, _, value in sorted(entries, key=lambda entry: (entry[0], entry[1])))

def _composition_stage_ir(stage: CompositionStage) -> dict[str, Any]:
    return {"name": stage.name, "component": stage.member,
            "arguments": [{"name": name, "expression": _expression(value)} for name, value in stage.arguments],
            "guard": _expression(stage.guard) if stage.guard is not None else None,
            "on_failure": _expression(stage.on_failure) if stage.on_failure is not None else None,
            "source_span": stage.span}

def _lower_variant_spec(definition: VariantDefinition) -> VariantSpec:
    next_tag = 0
    tags: list[tuple[str, int]] = []
    for case in definition.cases:
        tag = case.tag if case.tag is not None else next_tag
        tags.append((case.name, tag))
        next_tag = tag + 1
    return VariantSpec(definition, tuple(tags), None, None, definition.span)

def _model_property_value(value: Any) -> Any:
    """Normalize the structured method/period form into backend model data."""
    if isinstance(value, RecordValue):
        fields = dict(value.fields)
        method = fields.get("method")
        period = fields.get("period")
        return {"method": ".".join(method.path) if isinstance(method, Reference) else _value(method),
                "period": _value(period), "source_span": _span(value)}
    return _value(value)

def _property_map(declaration: Declaration | None) -> dict[str, Any]:
    return {item.name: item.value for item in declaration.properties} if declaration else {}

def _property_name(declaration: Declaration | None, name: str) -> str | None:
    return _name(_property_map(declaration).get(name))

def _value(value: Any) -> Any:
    expression = _expression(value)
    if isinstance(expression, dict) and expression.get("kind") == "literal": return expression["value"]
    if isinstance(expression, dict) and expression.get("kind") == "reference" and len(expression["path"]) == 1: return expression["path"][0]
    return expression

def _name(value: Any) -> str | None:
    if isinstance(value, str): return value
    if isinstance(value, Identifier): return value.value
    if isinstance(value, Literal) and isinstance(value.value, str): return value.value
    if isinstance(value, Reference) and len(value.path) == 1: return value.path[0]
    return None

def _endpoint_name(value: Any) -> str | None:
    if isinstance(value, Reference): return ".".join(value.path)
    return _name(value)

def _name_list(value: Any) -> tuple[str, ...]:
    if not isinstance(value, ListValue): return ()
    names: list[str] = []
    for item in value.items:
        name = _name(item)
        if name is None: return ()
        names.append(name)
    return tuple(names)

def _target_value(document: ProjectDocument) -> str | None:
    declaration = _one(document.declarations, "target")
    prop = next((item for item in declaration.properties if item.name == "value"), None) if declaration else None
    return _name(prop.value) if prop else None

def _one(declarations: tuple[Declaration, ...], kind: str) -> Declaration | None:
    return next((item for item in declarations if item.kind == kind), None)

def _signatures(document: ProjectDocument) -> tuple[Declaration, ...]:
    return tuple(item for item in document.declarations if item.prototype and item.kind != "project_routine")

def _implementation_bodies(document: ProjectDocument) -> dict[str, Declaration]:
    block = _one(document.declarations, "implementation_block")
    return {item.name: item for item in block.children if item.name} if block else {}

def _replace_declaration(declaration: Declaration, *, registry_key: str | None, signature_type: str | None) -> Declaration:
    from dataclasses import replace
    return replace(declaration, registry_key=registry_key, signature_type=signature_type)

def _span(value: Any):
    return getattr(value, "span", None)

def _literal_int(value: Any) -> int | None:
    return value.value if isinstance(value, Literal) and isinstance(value.value, int) and not isinstance(value.value, bool) else None

def _width(name: str) -> int:
    try: return int(name.lstrip("ui"))
    except (AttributeError, ValueError): return 0

def _quantity_seconds(value: Any) -> float | None:
    if not isinstance(value, Literal) or not isinstance(value.value, (int, float)): return None
    unit = value.unit or "s"; factor = {"s": 1.0, "ms": .001, "us": .000001, "ns": .000000001}.get(unit)
    return float(value.value) * factor if factor is not None else None

def _quantity_hz(value: Any) -> float | None:
    if not isinstance(value, Literal) or not isinstance(value.value, (int, float)): return None
    return float(value.value) if value.unit in (None, "Hz", "hz") else None
