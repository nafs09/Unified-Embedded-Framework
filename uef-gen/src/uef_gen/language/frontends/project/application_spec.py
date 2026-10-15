"""Private standalone application model produced from `.uef` source.

This normalized application model contains named component instances,
configuration bindings, model matrices, equation stages, subsystem membership,
and runtime tasks.
"""

from __future__ import annotations
from dataclasses import dataclass, field
from typing import Any
from uef_gen.language.frontends.project.ast import (
    AlgorithmStage,
    Equation,
    FlagsDefinition,
    FixedPointDefinition,
    RecordBoundary,
    SourceSpan,
    TypedBinding,
    TypeSpec,
    VariantDefinition,
)
from uef_gen.language.frontends.system.ast import SystemLinkSpec, SystemProtocolSpec, SystemRecordSpec

@dataclass(frozen=True)
class RecordSpec:
    """Fixed-shape user record used as a port, protocol, or channel contract."""

    name: str
    fields: tuple[TypedBinding, ...]
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class EnumValueSpec:
    """One enum case with its resolved, deterministic integer value."""

    name: str
    value: int
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class EnumSpec:
    """Fixed-width project or imported enum retained by the generator."""

    name: str
    storage_type: str
    variants: tuple[EnumValueSpec, ...]
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class ContractImportSpec:
    """Resolved `.uefs` contract provenance available to one node project."""

    source_path: str
    system_name: str
    contract_digest: str
    local_node_name: str
    shared_records: tuple[SystemRecordSpec, ...] = ()
    shared_enums: tuple[EnumSpec, ...] = ()
    shared_protocols: tuple[SystemProtocolSpec, ...] = ()
    shared_variants: tuple[VariantDefinition, ...] = ()
    shared_flags: tuple[FlagsDefinition, ...] = ()
    shared_fixed_point_types: tuple[FixedPointDefinition, ...] = ()
    local_endpoint_facts: dict[str, Any] = field(default_factory=dict)
    local_links: tuple[SystemLinkSpec, ...] = ()
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class VariantSpec:
    """Validated tagged variant with deterministic tags and fixed payloads."""

    definition: VariantDefinition
    resolved_tags: tuple[tuple[str, int], ...]
    # Target-specific size and alignment remain unresolved until the backend
    # has a verified ABI route. Placeholder values would imply known layout.
    maximum_payload_size: int | None
    alignment: int | None
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class FlagsSpec:
    """Resolved nominal bitset and its valid-bit mask."""

    definition: FlagsDefinition
    valid_mask: int
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class FixedPointSpec:
    """Validated fixed-point scale, representable range, and arithmetic policy."""

    definition: FixedPointDefinition
    scale_numerator: int
    scale_denominator: int
    minimum_value: int
    maximum_value: int
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class BoundaryRecordSpec:
    """Validated shared-memory or persistent layout, separate from wire ABI."""

    record: RecordSpec
    boundary: RecordBoundary
    # These remain unset until the selected backend ABI/layout policy resolves
    # target alignment and field padding. A placeholder zero would look like a
    # verified empty record and could corrupt persistence/shared-memory plans.
    encoded_size: int | None
    alignment: int | None
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class TypeInterchangeManifest:
    """Canonical type IDs and boundary routes used by generated-project checks."""

    type_ids: dict[str, str] = field(default_factory=dict)
    uetd_abi_routes: dict[str, str] = field(default_factory=dict)
    # Exact API-linked UETD data for every callable used by the application.
    # Keyed by source-file path and call span so overloads retain distinct,
    # already-selected UETD C bindings in backend input.
    umath_binding_routes: dict[str, dict[str, Any]] = field(default_factory=dict)
    # Exact UETD type references used by source declarations or UETD component
    # ports. These point to public C typedefs; they are not copied layouts.
    uetd_type_bindings: dict[str, dict[str, Any]] = field(default_factory=dict)
    protocol_codecs: dict[str, str] = field(default_factory=dict)
    shared_memory_layouts: dict[str, str] = field(default_factory=dict)
    persistent_codecs: dict[str, str] = field(default_factory=dict)
    target_support: dict[str, bool | None] = field(default_factory=dict)
    source_spans: dict[str, SourceSpan] = field(default_factory=dict)

@dataclass(frozen=True)
class PortContractSpec:
    """Typed application boundary with update, age, and clock semantics."""

    name: str
    type_spec: TypeSpec
    direction: str
    rate_hz: float | None = None
    max_age_s: float | None = None
    clock_domain: str | None = None
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class PortBindingSpec:
    """One explicit source-to-member or member-to-public-port binding."""

    destination: str
    expression: Any
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class ComponentSpec:
    """One named configured component backed by a UETD registry contract."""

    name: str
    kind: str
    registry_id: str
    properties: dict[str, Any] = field(default_factory=dict)
    inputs: dict[str, Any] = field(default_factory=dict)
    outputs: dict[str, TypeSpec] = field(default_factory=dict)
    members: tuple[str, ...] = ()
    source_span: SourceSpan | None = None
    # Keep the authored body alongside the UETD contract. Port annotations,
    # configured values, output expressions, and nested declarations are all
    # consumed by code generation and must survive lowering.
    bindings: tuple[TypedBinding, ...] = ()
    output_assignments: tuple[CompositionOutputSpec, ...] = ()
    nested_declarations: tuple[dict[str, Any], ...] = ()
    uetd_contract: dict[str, Any] = field(default_factory=dict)

@dataclass(frozen=True)
class ComponentCallSpec:
    """Ordered, typed member invocation in a hybrid or subsystem."""

    name: str
    component: str
    arguments: tuple[PortBindingSpec, ...]
    guard: Any | None = None
    on_failure: Any | None = None
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class CompositionLocalSpec:
    """Typed immutable value evaluated at its source position."""

    name: str
    type_spec: TypeSpec
    expression: Any
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class CompositionOutputSpec:
    """Explicit public output expression; never inferred from member names."""

    name: str
    expression: Any
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class TaskInvocationSpec:
    """One component/subsystem call made on each task activation."""

    component: str
    arguments: tuple[PortBindingSpec, ...] = ()
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class ModelSpec:
    """Project-local plant/system dynamics in linear or equation form."""

    name: str
    time_domain: str
    sample_time_s: float | None
    state: tuple[TypedBinding, ...]
    inputs: tuple[TypedBinding, ...]
    outputs: tuple[TypedBinding, ...]
    parameters: tuple[TypedBinding, ...] = ()
    matrices: dict[str, Any] = field(default_factory=dict)
    equations: tuple[Equation, ...] = ()
    jacobians: dict[str, Any] = field(default_factory=dict)
    discretization: dict[str, Any] = field(default_factory=dict)
    constraints: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None
    dynamics_form: str | None = None

@dataclass(frozen=True)
class CustomAlgorithmSpec:
    """Project-defined algorithm equations and state update stages."""

    name: str
    role: str
    model_name: str | None
    parameters: tuple[TypedBinding, ...]
    state: tuple[TypedBinding, ...]
    inputs: tuple[TypedBinding, ...]
    outputs: tuple[TypedBinding, ...]
    local_variables: tuple[TypedBinding, ...]
    stages: tuple[AlgorithmStage, ...]
    numeric_mode: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class HybridAlgorithmSpec:
    """Language-owned selection/fusion over declared algorithm instances."""

    name: str
    inputs: tuple[PortContractSpec, ...]
    outputs: tuple[PortContractSpec, ...]
    members: tuple[tuple[str, str], ...]
    stages: tuple[ComponentCallSpec, ...]
    locals: tuple[CompositionLocalSpec, ...]
    output_expressions: tuple[CompositionOutputSpec, ...]
    choices: dict[str, Any]
    failure_policy: Any
    execution_budget_s: float | None = None
    source_span: SourceSpan | None = None
    # Source-ordered declarations that affect composition execution. The
    # separately indexed fields above remain convenient for semantic passes.
    execution_body: tuple[dict[str, Any], ...] = ()

@dataclass(frozen=True)
class SubsystemSpec:
    """Typed function-like wrapper with explicit source-ordered member calls."""

    name: str
    members: tuple[tuple[str, str], ...]
    inputs: tuple[PortContractSpec, ...] = ()
    outputs: tuple[PortContractSpec, ...] = ()
    stages: tuple[ComponentCallSpec, ...] = ()
    locals: tuple[CompositionLocalSpec, ...] = ()
    output_expressions: tuple[CompositionOutputSpec, ...] = ()
    properties: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None
    execution_body: tuple[dict[str, Any], ...] = ()

@dataclass(frozen=True)
class StateTransitionSpec:
    """Backend-neutral transition row with a lowered guard expression."""

    source_state: str
    destination_state: str
    guard: Any
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class StateActionSpec:
    """Named zero-argument status operation invoked synchronously by the owner."""

    phase: str
    state: str
    operation: str
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class StateMachineSpec:
    """Finite-state supervisor owned and evaluated by one scheduled context."""

    name: str
    state_type: str
    initial_state: str
    states: tuple[str, ...]
    owner: str
    consumers: tuple[str, ...]
    transitions: tuple[StateTransitionSpec, ...]
    required_state_capacity: int
    required_transition_capacity: int
    actions: tuple[StateActionSpec, ...] = ()
    evaluation_boundary: str = "after_owner_run"
    max_transitions_per_activation: int = 1
    state_delivery: str = "latest_snapshot_per_consumer"
    consumer_activation: str = "scheduled_independently"
    source_span: SourceSpan | None = None
    properties: dict[str, Any] = field(default_factory=dict)

@dataclass(frozen=True)
class ChannelSpec:
    """Bounded producer-to-subscribers route with independent fan-out delivery."""

    name: str
    source: str
    producer: str
    consumers: tuple[str, ...]
    capacity: Any
    overflow: str
    ownership: str
    delivery_semantics: str = "independent_per_consumer"
    destination: str | None = None
    maximum_age: Any = None
    memory_region: str | None = None
    properties: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class ProjectRoutineSpec:
    """Project-owned zero-argument callback required by runtime or component hooks."""

    name: str
    owner_component: str | None = None
    parameters: tuple[TypedBinding, ...] = ()
    return_type: str = "void"
    execution_context: str = "task"
    source_span: SourceSpan | None = None

    @property
    def qualified_name(self) -> str:
        return f"{self.owner_component}.{self.name}" if self.owner_component else self.name

@dataclass(frozen=True)
class TaskSpec:
    """Scheduled component invocation with timing and resource requirements."""

    name: str
    invocation: TaskInvocationSpec
    properties: dict[str, Any]
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class LoopSpec:
    """One BareMetal main-loop invocation with explicit timing policy."""

    name: str
    invocation: TaskInvocationSpec
    properties: dict[str, Any]
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class TasksetSpec:
    """Scheduler scope assembled from named tasks and runtime constraints."""

    name: str
    properties: dict[str, Any]
    tasks: tuple[TaskSpec, ...]
    interrupts: tuple[dict[str, Any], ...] = ()
    resources: tuple[dict[str, Any], ...] = ()
    constraints: tuple[dict[str, Any], ...] = ()
    source_span: SourceSpan | None = None
    channels: tuple[ChannelSpec, ...] = ()

@dataclass(frozen=True)
class OperatingSystemSpec:
    """Runtime root owning modes, direct loops/tasks, optional groups, and policy."""

    name: str
    properties: dict[str, Any]
    tasksets: tuple[TasksetSpec, ...]
    tasks: tuple[TaskSpec, ...] = ()
    state_machines: tuple[StateMachineSpec, ...] = ()
    interrupts: tuple[dict[str, Any], ...] = ()
    channels: tuple[ChannelSpec, ...] = ()
    resources: tuple[dict[str, Any], ...] = ()
    constraints: tuple[dict[str, Any], ...] = ()
    source_span: SourceSpan | None = None
    loops: tuple[LoopSpec, ...] = ()

@dataclass(frozen=True)
class ProtocolFieldSpec:
    """A wire field with explicit type, offset, width, and encoding metadata."""

    name: str
    type_spec: TypeSpec
    properties: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class ProtocolMessageSpec:
    """A message with flat fields or an ordered fragment layout."""

    name: str
    message_id: int | None
    fields: tuple[ProtocolFieldSpec, ...] = ()
    fragments: tuple[dict[str, Any], ...] = ()
    properties: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None
    flags: tuple[dict[str, Any], ...] = ()

@dataclass(frozen=True)
class ProtocolSpec:
    """Complete protocol declaration retained for MCU encoder/parser generation."""

    name: str
    registry_id: str | None
    transport: str | None
    framing: dict[str, Any]
    header: dict[str, Any]
    sequence: dict[str, Any]
    checksum: dict[str, Any]
    messages: tuple[ProtocolMessageSpec, ...]
    receiver: dict[str, Any]
    properties: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None
    payload: dict[str, Any] = field(default_factory=dict)
    behavior: dict[str, Any] = field(default_factory=dict)
    sections: dict[str, Any] = field(default_factory=dict)

@dataclass(frozen=True)
class StandaloneApplicationSpec:
    """Complete application declarations for UETD's standalone generator."""

    project_name: str
    components: tuple[ComponentSpec, ...]
    models: tuple[ModelSpec, ...]
    custom_algorithms: tuple[CustomAlgorithmSpec, ...]
    subsystems: tuple[SubsystemSpec, ...]
    operating_system: OperatingSystemSpec
    target_id: str
    project_routines: tuple[ProjectRoutineSpec, ...] = ()
    project_parameters: tuple[TypedBinding, ...] = ()
    board_id: str | None = None
    numeric_profile: dict[str, Any] = field(default_factory=dict)
    records: tuple[RecordSpec, ...] = ()
    hybrid_algorithms: tuple[HybridAlgorithmSpec, ...] = ()
    protocols: tuple[ProtocolSpec, ...] = ()
    enums: dict[str, EnumSpec] = field(default_factory=dict)
    contract_imports: tuple[ContractImportSpec, ...] = ()
    variants: tuple[VariantSpec, ...] = ()
    flags_types: tuple[FlagsSpec, ...] = ()
    fixed_point_types: tuple[FixedPointSpec, ...] = ()
    boundary_records: tuple[BoundaryRecordSpec, ...] = ()
    type_interchange: TypeInterchangeManifest = field(default_factory=TypeInterchangeManifest)
    globals: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None
