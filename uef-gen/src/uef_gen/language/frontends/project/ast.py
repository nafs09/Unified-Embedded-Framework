"""Private source-located AST for the standalone UETD project language.
Nodes represent declarations and mathematical expressions. This model is
and contains no executable user code.
"""

from __future__ import annotations
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

@dataclass(frozen=True)
class SourcePosition:
    """One source position; offset counts code points; line/column are one-based."""

    offset: int
    line: int
    column: int

@dataclass(frozen=True)
class SourceSpan:
    """Half-open source range associated with a node or diagnostic."""

    path: Path | None
    start: SourcePosition
    end: SourcePosition

@dataclass(frozen=True)
class Literal:
    """Finite scalar literal with an optional authored unit."""

    value: str | int | float | bool
    unit: str | None
    span: SourceSpan

@dataclass(frozen=True)
class Identifier:
    """Symbolic enum or registry key, without resolving it during parsing."""

    value: str
    span: SourceSpan

@dataclass(frozen=True)
class Reference:
    """Dotted component/property/state reference such as `imu.angular_rate`."""

    path: tuple[str, ...]
    span: SourceSpan

@dataclass(frozen=True)
class UnaryExpression:
    """Whitelisted unary arithmetic or boolean operation."""

    operator: str
    operand: Value
    span: SourceSpan

@dataclass(frozen=True)
class BinaryExpression:
    """Whitelisted arithmetic, comparison, or boolean operation."""

    operator: str
    left: Value
    right: Value
    span: SourceSpan

@dataclass(frozen=True)
class MathCallExpression:
    """Registry-named UMATH call lowered through its reviewed binding contract."""

    callee: str
    arguments: tuple[Value, ...]
    span: SourceSpan

@dataclass(frozen=True)
class CastExpression:
    """Explicit checked conversion to one declared numeric type."""

    destination: TypeSpec
    value: Value
    span: SourceSpan

@dataclass(frozen=True)
class UnitConversionExpression:
    """Explicit scale/offset conversion to a compatible registered unit."""

    target_unit: str
    value: Value
    span: SourceSpan

@dataclass(frozen=True)
class IndexSubscript:
    """One scalar index or half-open slice of an array dimension."""

    start: Value
    end: Value | None = None

@dataclass(frozen=True)
class IndexExpression:
    """Bounded multidimensional array index or per-axis half-open slice."""

    value: Value
    subscripts: tuple[IndexSubscript, ...]
    span: SourceSpan

@dataclass(frozen=True)
class ListValue:
    """Ordered vector literal, property list, or component reference list."""

    items: tuple[Value, ...]
    span: SourceSpan

@dataclass(frozen=True)
class MatrixLiteral:
    """Rectangular row-major matrix literal."""

    rows: tuple[tuple[Value, ...], ...]
    span: SourceSpan

@dataclass(frozen=True)
class RecordValue:
    """DSL-native named value assembled by `map(field = expression, ...)`."""

    fields: tuple[tuple[str, Value], ...]
    span: SourceSpan

@dataclass(frozen=True)
class SelectValue:
    """DSL-native, bounded conditional expression with type-matched branches."""

    condition: Value
    if_true: Value
    if_false: Value
    span: SourceSpan

@dataclass(frozen=True)
class VariantConstructorExpression:
    """Construction of one case of Option, Result, or a local variant."""

    constructor: str
    positional_value: Value | None = None
    fields: tuple[tuple[str, Value], ...] = ()
    span: SourceSpan | None = None

@dataclass(frozen=True)
class MatchArm:
    """One ordered, scoped pattern and expression branch."""

    case_name: str
    binding_name: str | None
    field_bindings: tuple[tuple[str, str], ...]
    expression: "Value"
    span: SourceSpan

@dataclass(frozen=True)
class MatchExpression:
    """Exhaustive value match over one nominal enum/variant/constructor."""

    subject: "Value"
    arms: tuple[MatchArm, ...]
    default_expression: "Value | None"
    span: SourceSpan

@dataclass(frozen=True)
class FlagsOperationExpression:
    """Typed operation on one nominal flags value and member."""

    operation: str
    value: "Value"
    member: str
    span: SourceSpan

@dataclass(frozen=True)
class TypeSpec:
    """Scalar/domain or language constructor type with shape and unit."""

    name: str
    # Dimensions preserve compile-time identifiers until parameter resolution;
    # lowering must not collapse a symbolic bound into an unknown/None shape.
    shape: tuple[int | str, ...] = ()
    unit: str | None = None
    span: SourceSpan | None = None
    type_arguments: tuple["TypeSpec", ...] = ()

@dataclass(frozen=True)
class TypedBinding:
    """Typed input, output, state, parameter, or model variable declaration."""

    name: str
    type_spec: TypeSpec | None
    value: Value | None
    span: SourceSpan
    role: str = "binding"
    properties: tuple[Property, ...] = ()

@dataclass(frozen=True)
class EnumVariant:
    """One fixed-width enum case, preserving whether its value was authored."""

    name: str
    value: int | None
    span: SourceSpan

@dataclass(frozen=True)
class EnumDefinition:
    """Project-owned nominal integer type used by ports and FSMs."""

    name: str
    storage_type: str
    variants: tuple[EnumVariant, ...]
    span: SourceSpan

@dataclass(frozen=True)
class RecordDefinition:
    """Project-owned fixed record; boundary metadata is never inferred."""

    name: str
    fields: tuple[TypedBinding, ...]
    span: SourceSpan
    boundary: "RecordBoundary | None" = None

@dataclass(frozen=True)
class VariantCaseDefinition:
    """One fixed-payload tagged case; ``tag`` may be implicit locally."""

    name: str
    tag: int | None
    fields: tuple[TypedBinding, ...]
    span: SourceSpan

@dataclass(frozen=True)
class VariantDefinition:
    """Project-owned closed variant with a fixed-width discriminant."""

    name: str
    tag_type: str
    cases: tuple[VariantCaseDefinition, ...]
    span: SourceSpan

@dataclass(frozen=True)
class FlagsMember:
    """Named bit position in a nominal fixed-width flags type."""

    name: str
    bit: int
    span: SourceSpan

@dataclass(frozen=True)
class FlagsDefinition:
    """Nominal bitset whose allowed members define its valid mask."""

    name: str
    storage_type: str
    members: tuple[FlagsMember, ...]
    span: SourceSpan

@dataclass(frozen=True)
class FixedPointDefinition:
    """Project-owned binary fixed-point representation and arithmetic policy."""

    name: str
    storage_type: str
    fractional_bits: int
    unit: str
    rounding: str
    overflow: str
    span: SourceSpan

@dataclass(frozen=True)
class RecordBoundary:
    """Explicit shared-memory or persistent encoding contract for a record."""

    kind: str
    version: int
    schema_id: str | None = None
    alignment: int | None = None
    field_offsets: tuple[tuple[str, int], ...] = ()
    field_encodings: tuple[tuple[str, str], ...] = ()
    atomic_fields: tuple[str, ...] = ()
    producer: str | None = None
    consumer: str | None = None
    memory_order: str | None = None
    memory_barriers: tuple[str, ...] = ()
    cache_policy: str | None = None
    publication_sequence_field: str | None = None
    integrity: str | None = None
    migration_policy: str | None = None
    span: SourceSpan | None = None

@dataclass(frozen=True)
class ContractImport:
    """Authored relative path to one `.uefs` shared-contract source."""

    path: str
    span: SourceSpan

@dataclass(frozen=True)
class Equation:
    """One declarative assignment in a model or algorithm stage."""

    target: Reference | IndexExpression
    expression: Value
    span: SourceSpan
    mode: str = "assign"

@dataclass(frozen=True)
class AlgorithmStage:
    """One C-like named lifecycle function in a custom algorithm definition."""

    name: str
    parameters: tuple[TypedBinding, ...]
    body: tuple[AlgorithmStatement, ...]
    span: SourceSpan

@dataclass(frozen=True)
class BoundedLoop:
    """Fixed-range loop with bounds that semantic validation must prove static."""

    variable: str
    start: Value
    stop_exclusive: Value
    body: tuple[AlgorithmStatement, ...]
    span: SourceSpan

@dataclass(frozen=True)
class ConditionalStatement:
    """Boolean branch inside a bounded algorithm/model statement block."""

    condition: Value
    then_body: tuple[AlgorithmStatement, ...]
    else_body: tuple[AlgorithmStatement, ...]
    span: SourceSpan

@dataclass(frozen=True)
class StateTransition:
    """Guarded transition in a finite state-machine declaration."""

    source_state: str
    destination_state: str
    guard: Value
    span: SourceSpan

@dataclass(frozen=True)
class StateAction:
    """Declared project routine or UETD operation invoked by an FSM action."""

    phase: str
    state: str
    operation: Reference
    span: SourceSpan

@dataclass(frozen=True)
class Property:
    """Named scalar/list/reference property inside a typed component block."""

    name: str
    value: Value
    span: SourceSpan

@dataclass(frozen=True)
class CompositionStage:
    """One ordered call to a declared member inside a hybrid or subsystem.
    Stages execute in source order. Each named argument is an explicit member
    input binding; later stages may refer to the named result of an earlier
    stage. This keeps standalone execution order visible without introducing
    NEXUS graph nodes or relying on an inferred graph schedule.
    """

    name: str
    member: str
    arguments: tuple[tuple[str, Value], ...]
    guard: Value | None = None
    on_failure: Value | None = None
    span: SourceSpan | None = None

@dataclass(frozen=True)
class CompositionLocal:
    """Immutable typed expression introduced by a composition-level let."""

    name: str
    type_spec: TypeSpec
    expression: Value
    span: SourceSpan

@dataclass(frozen=True)
class CompositionOutput:
    """Explicit expression assigned to one public composition output."""

    name: str
    expression: Value
    span: SourceSpan


AlgorithmStatement = Equation | BoundedLoop | ConditionalStatement

@dataclass(frozen=True)
class Declaration:
    """One signature, implementation, project value, model, or runtime node.
    `kind` identifies the language form. `registry_key` identifies a UETD-owned
    constructor for a signature. `prototype` distinguishes a header signature
    from its same-named detailed definition in the implementation section.
    `implementation_block` is a source-order container whose children are
    matched to signatures during validation. Nested blocks, typed bindings,
    and equations remain explicit source data and do not imply graph nodes or
    graph edges.
    """

    kind: str
    name: str | None
    registry_key: str | None
    signature_type: str | None = None
    properties: tuple[Property, ...] = ()
    children: tuple[Declaration, ...] = ()
    bindings: tuple[TypedBinding, ...] = ()
    stages: tuple[AlgorithmStage, ...] = ()
    composition_stages: tuple[CompositionStage, ...] = ()
    composition_locals: tuple[CompositionLocal, ...] = ()
    composition_outputs: tuple[CompositionOutput, ...] = ()
    equations: tuple[AlgorithmStatement, ...] = ()
    span: SourceSpan | None = None
    prototype: bool = False
    # Model syntax keeps the presence of an explicit (possibly empty) equation
    # block distinct from its equations. This lets validation reject empty
    # dynamic models without confusing them with matrix-only models.
    model_form: str | None = None
    # General component declarations may publish an output expression in
    # addition to declaring its typed output port. Composition nodes use the
    # same value shape through ``composition_outputs``.
    output_assignments: tuple[CompositionOutput, ...] = ()

@dataclass(frozen=True)
class ProjectDocument:
    """Parsed source root before UETD registry and type resolution."""

    name: str
    declarations: tuple[Declaration, ...]
    span: SourceSpan
    source_path: Path | None = None
    parameters: tuple[TypedBinding, ...] = ()
    enums: tuple[EnumDefinition, ...] = ()
    records: tuple[RecordDefinition, ...] = ()
    variants: tuple[VariantDefinition, ...] = ()
    flags: tuple[FlagsDefinition, ...] = ()
    fixed_point_types: tuple[FixedPointDefinition, ...] = ()
    contract_imports: tuple[ContractImport, ...] = ()


Value = (
    Literal
    | Identifier
    | Reference
    | UnaryExpression
    | BinaryExpression
    | MathCallExpression
    | CastExpression
    | UnitConversionExpression
    | IndexExpression
    | ListValue
    | MatrixLiteral
    | RecordValue
    | SelectValue
    | VariantConstructorExpression
    | MatchExpression
    | FlagsOperationExpression
)

@dataclass
class SourceMap:
    """Map standalone spec/generated source locations back to `.uef` spans."""

    entries: dict[str, SourceSpan] = field(default_factory=dict)

    def add(self, generated_path: str, span: SourceSpan) -> None:
        """Record a normalized generated symbol path and reject ambiguity."""
        normalized = ".".join(part for part in generated_path.replace("\\", ".").split(".") if part)
        if not normalized:
            raise ValueError("A source-map key must contain at least one path segment")
        existing = self.entries.get(normalized)
        if existing is not None and existing != span:
            raise ValueError(f"Conflicting source spans map to generated symbol {normalized!r}")
        self.entries[normalized] = span

    def find(self, generated_path: str) -> SourceSpan | None:
        """Find an exact mapping or the nearest mapped generated parent."""
        current = generated_path
        while current:
            if current in self.entries:
                return self.entries[current]
            current = current.rpartition(".")[0]
        return None
