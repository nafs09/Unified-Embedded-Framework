# UEF Project Language (UEF-PL)


UETD owns the catalogues and implementation contracts for targets, boards,
modules, runtimes, public C types, UMATH operations, and numeric capabilities.
`uef-gen` owns the grammar and translates a validated project into a typed
application description and generated source. Project files own application-
specific models, bindings, schedules, custom algorithms/protocols, and
compositions. `.uef` and `.uefs` remain the file extensions and CLI inputs.

The lexer is locale-independent: identifiers are ASCII and case-sensitive,
decimal/scientific and hexadecimal integer tokens are finite, signs are
operators, and strings use the documented escape set. It recognizes `..` as a
range token before considering a decimal point, so `0..N` cannot be read as
two malformed floating-point values. A dimension is a positive integer
literal or a compile-time constant reference; it is resolved before lowering.

The parser, verifier, lowering, and generator remain scaffolds. This document
sets their intended contract; examples demonstrate the language design and are
not claims that `check`, `generate`, `build`, or `flash` already accept every
construct shown. Current implementation status is in
[`UETD_SYSTEM_AUDIT.md`](../../UETD/docs/audits/UETD_SYSTEM_AUDIT.md); source-equivalence,
catalogue-readiness and selective-generation requirements are in
[`UETD_EQUIVALENCE_AND_CATALOGUE_GAP_AUDIT.md`](../../UETD/docs/audits/UETD_EQUIVALENCE_AND_CATALOGUE_GAP_AUDIT.md).
For a first-time user walkthrough, see the
[UEF-PL user handbook](../guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md). The
[technical guide](../guides/UEF_PROJECT_LANGUAGE_GUIDE.md) explains source
structure and language rules. Registry-derived module/type/UMATH inventories
and the unit table are maintained separately in the
[generated catalogue reference](../reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md)
and [dimension-grouped unit reference](../reference/UEF_PL_UNITS_REFERENCE.md).
For a practical map of the JSON indexes, manifests, schemas, and source
discovery order, see [UEF-PL registries and discovery](../guides/UEF_PL_REGISTRIES.md).

## Source layout

A target project is organized so the declarations read like a C header, then a
runtime outline, then detailed definitions:

```uetd
project GenericRobot {
  target = "esp32-devkitc-v4";
  numeric {
    scalar = f32; accumulator = f64;
    rounding = nearest_even; overflow = reject;
    non_finite = reject; fused_multiply_add = disabled;
    matrix_layout = row_major;
  }
  parameter timing : RobotTiming;

  record WheelSample { speed : f32<m/s>; valid : bool; }
  enum RobotMode : u8 { stopped = 0, manual = 1, autonomous = 2 }
  sensor left_wheel : encoder;
  algorithm speed_loop : pid;
  actuator left_motor : h_bridge;
  subsystem left_drive;

  operatingsystem RobotRuntime {
    runtime = FreeRTOS;
    component_failure_policy = assert_safe_state;
    task wheel_control {
      run = left_drive(command = fixed_drive_command);
      rate = 200 Hz; priority = 4; deadline = 5 ms;
    }
  }

  implementation {
    sensor left_wheel { peripheral = left_encoder; counts_per_revolution = 2048; }
    algorithm speed_loop { reference = left_drive.request; measurement = left_wheel.speed; sample_time = 5 ms; }
    actuator left_motor { peripheral = left_pwm; command = speed_loop.output; safe_state = 0; }

    subsystem left_drive {
      input request : f32<m/s> { rate = 200 Hz; max_age = 10 ms; }
      output effort : f32 { rate = 200 Hz; max_age = 5 ms; }
      use left_wheel as encoder;
      use speed_loop as controller;
      use left_motor as motor;
      stage sample = encoder();
      stage control = controller(reference = request, measurement = sample.speed);
      stage applied = motor(command = control.output);
      output effort = control.output;
    }
  }
}
```

The language uses explicit role words for components that need role-specific
validation (`sensor`, `actuator`, `algorithm`, `protocol`, `peripheral`, and
`bus`). Software component IDs are resolved from UETD's module registry. A
peripheral type such as `gpio` or `exti` is a target resource, so it is
resolved from the selected UETD chip/board peripheral catalogue instead. Every
other directly authored UETD software module is declared by its registered
identifier: `watchdog control_watchdog;`, `fault fault_manager;`, or
`fpga coil_link;`. There is no generic `component` keyword and no uef-gen
module-ID list.

`record` introduces a fixed-shape, typed application value. Records are useful
for subsystem interfaces, algorithm ports, task messages, and system link
payloads. They do not describe a graph node. The implementation section is the
only place for detailed component/model/algorithm/protocol bodies; each header
signature has exactly one matching body.

`enum` introduces a nominal finite type with an explicit fixed-width integer
representation, for example `enum PilotMode : u8 { fpv = 0, camera = 1 }`.
Omitted values increment from the preceding value, beginning at zero, in source
order. Values must fit the declared type and be unique. Any enum used in a wire
or persistent format must assign explicit values so adding or reordering cases
cannot silently change the ABI. Enum cases are referenced as `PilotMode.fpv`;
bare case names are valid only where the grammar explicitly scopes them, such
as an FSM transition endpoint.

Top-level `parameter` declarations name required project inputs that should not
be guessed by a template or hidden in YAML. The type can be a scalar, fixed
array, enum, or declared record. A parameter may have an explicit default; if
it has no default, `check` can analyze its use but `generate` must report it as
unresolved until the author supplies a value. This makes design examples
honest while hardware measurements and control targets are still unknown.
The `string` type is compile-time configuration only: it is interned into the
generated configuration and is not valid for runtime ports, protocol fields,
or dynamically allocated application data.

An enum's declared `u8`, `u16`, or other representation is a language-level
ABI promise. A C compiler's native `enum` representation is not guaranteed to
match it. Generated wire/shared/persistent values must therefore use an
explicit fixed-width storage type and named constants (or a target-checked
representation), validate decoded values, and define behavior for reserved or
unknown values. Do not rely on compiler defaults for enum size or signedness.

## Registry ownership and identifiers

`UETD/api.json` is the UETD tooling path index. Its module registry is the sole
index of software-module identifiers; each `DSL_descriptor` is the source
spelling and each manifest owns authoring, ports, operations, readiness,
dependencies, numeric/runtime/target limits, and generation routes. Target,
development-board, and peripheral/resource IDs resolve through their UETD
catalogues. `api.json` also points to the header-derived type and UMATH
function indexes and the canonical unit catalogue. C headers remain
authoritative for declarations; module manifests own implementation readiness;
the unit registry owns dimensions and exact unit scales.

`api.json` points to catalogue index files; each catalogue index points to its
own record files. The `uef-gen` language registry stores grammar and
language-only forms, not copies of UETD module or math names. The detailed
registry layout, discovery behavior, and safe edit workflows are in the
[registry guide](../guides/UEF_PL_REGISTRIES.md).

A module is available to the language as soon as its descriptor exists in the
UETD index. A registered-but-unimplemented module remains discoverable and gets
a readiness warning in the editor and `uef-gen check`; generation must block
when its selected implementation route is unavailable.
Peripheral types are checked against the selected target's UETD resource data.
Adding an ordinary software module must not require a parser branch, a second
module registry, or an extension update.

For readability, a UETD-backed component body repeats its canonical
`DSL_descriptor` after the closing brace, for example:

```uef
algorithm tonal_probe : goertzel;

implementation {
  algorithm tonal_probe {
    input samples : f32[256]<V>;
    input frequency : f32<Hz>;
    output magnitude : f32<V>;
    sample_rate = 96 kHz;
  } goertzel;
}
```

The parser requires an exact match with the constructor declaration; the
suffix is a readability cue, not a second registry source. It applies to
registered peripherals, buses, sensors, actuators, algorithms, protocols, and
direct module declarations. Project-defined algorithms, protocols, models,
hybrids, and subsystems keep their own language forms and have no UETD suffix.

The UMATH header index is for discovery and does not turn a C prototype into a
safe DSL contract by itself. Each callable function needs reviewed UETD metadata
for argument types/modes, buffer shapes and bounds, aliasing, numeric/unit
effects, failure behavior, target availability, and bounded work/scratch. A
function is callable only when both its UETD implementation and its DSL binding
are ready. The generated guide keeps scaffold and signature-only entries
visible so missing work is obvious rather than silently accepted.

The uef-gen language registry contains grammar rules and language-only forms
such as `map`, `select`, and composition clauses. It does not mirror UETD module
IDs, type declarations, function signatures, parameters, capabilities, or
supported targets. VS Code static highlighting colors the grammar; semantic
tokens, completion, hover, signatures, and readiness diagnostics resolve the
live UETD API-linked catalogues.

## Deterministic source rules

The language is compiled, not interpreted. Its front end tokenizes and parses
source into a source-located syntax tree, resolves names and types against the
project, imported `.uefs` contracts, and current UETD registries, validates
hardware/runtime constraints, then lowers into a finite typed application
specification. The generator consumes that specification and UETD-owned
generation routes. No Python/C/C++ execution, arbitrary expression evaluation,
or hidden YAML authoring step is involved. `check` and editor diagnostics use
the same front end and semantic passes as generation.

Names are case-sensitive. A target is an exact quoted UETD target ID. A module
descriptor, setting, operation, runtime, math function, peripheral, core, and
policy must resolve to a typed UETD or project contract; unknown properties are
errors rather than accepted strings. Values are finite, statically shaped, and
side-effect free. Operators have the precedence listed in the type section.
`and` and `or` short-circuit left to right. Parentheses control grouping.
Duplicate declarations, duplicate settings, unresolved required values,
unsupported conversions, and underspecified effects are diagnosed; the
compiler never fills them with guessed defaults.

Execution order exists only where the source explicitly defines it: custom
algorithm lifecycle functions and equations; ordered subsystem/hybrid stages;
first-match `choose` branches; custom protocol field/fragment order; FSM
transition order; and the selected runtime's scheduling contract. Declaration
and property order otherwise has no runtime meaning. Compiler passes preserve
authored order for ordered constructs. Generation is reproducible for the same
source, UETD registry/template digest, generator version, target, and project
inputs. Numeric rounding, overflow, saturation, and fixed-point policy come
from the numeric profile or the referenced UETD contract; they are never
backend guesses.

## Types, expressions, and bounded computation

UEF-PL has `bool` plus fourteen numeric spellings, each linked to its UETD C
alias by an explicit `dsl_name` in UETD's API-linked type registry. The exact
mapping is:

| UEF-PL type | UETD C alias | Meaning |
|---|---|---|
| `f32` | `uetd_f32_t` | 32-bit floating point |
| `f64` | `uetd_f64_t` | 64-bit floating point |
| `i8`, `i16`, `i32`, `i64` | `uetd_i8_t`, `uetd_i16_t`, `uetd_i32_t`, `uetd_i64_t` | Signed fixed-width integers |
| `u8`, `u16`, `u32`, `u64` | `uetd_u8_t`, `uetd_u16_t`, `uetd_u32_t`, `uetd_u64_t` | Unsigned fixed-width integers |
| `q15`, `q16`, `q31`, `q63` | `uetd_q15_t`, `uetd_q16_t`, `uetd_q31_t`, `uetd_q63_t` | UETD fixed-point representations |

The project `numeric` profile selects `scalar` and `accumulator` formats. UETD's
`umath_scalar_t` and `umath_accumulator_t` resolve from
`UMATH_SCALAR_TYPE` and `UMATH_ACCUMULATOR_TYPE`; every generated translation
unit must use the same settings. A `q*` value type does not by itself make a
module or UMATH operation support that fixed-point mode. The module, operation,
runtime, and target contracts must all declare compatible support. `string` is
available only for UETD contracts that declare compile-time configuration; it
is not a runtime value.

The project value model is fixed-size scalars, arrays, matrices, project
records, nominal enums, and UETD by-value types whose module contracts expose
them. `Received<T>` is a protocol envelope with a value, local receive
timestamp, and sequence metadata. The UETD type registry indexes every
discovered public-header declaration, including declarations from `ucore`,
UMATH, and UETD modules. `uef-gen` loads those names from the registry linked by
`UETD/api.json`; it does not maintain a copied UETD type allowlist. The complete
generated table reports each C name, optional DSL spelling, declaration kind,
visibility, readiness (`public_header` or `future_header`), and source header
in the [UEF-PL catalogue](../reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md#public-uetd-c-types-and-enums).

Discovery is not permission to put a typedef into project state. The registry
contains ordinary UETD contract types, UMATH value candidates, and the
fourteen explicit numeric spellings. Only a compatible UETD authoring contract
can expose a non-primitive C type by value. Opaque handles, arbitrary pointers,
dynamic containers, and function pointers remain private to their owning
contract; a declaration under a `future/` header is not an implemented API.
Runtime strings and nullable values are excluded; optional data uses an
explicit validity field or typed protocol envelope.

Units are dimensional types, not comments. UETD's unit registry defines the SI
dimensions plus count, information, and symbol-rate dimensions used by embedded
interfaces. Each unit resolves to dimensions, an exact scale, and (for affine
units) an offset. Addition and comparison require the same normalized unit
mapping; equal physical dimensions alone do not permit adding `m` to `cm`.
`convert_unit<UNIT>(value)` is the explicit checked conversion. In SI terms it
applies `target = (source × source_scale + source_offset - target_offset) /
target_scale`, then checks the selected numeric range and rounding policy.
Affine units such as `degC` are absolute temperatures: absolute minus absolute
produces `delta_degC`; absolute plus a temperature difference produces an
absolute temperature; adding two absolute temperatures and subtracting an
absolute temperature from a difference are errors. Affine units cannot appear
in products or powers. `cast<T>` changes scalar representation only and
preserves units. The checker also rejects shape mismatch, unsupported matrix
dimensions, and implicit numeric conversions. A literal takes the uniquely
expected type when one exists; otherwise a fractional literal uses the
selected UMATH scalar type and an integer literal uses `i32`. Unrepresentable
or ambiguous values are errors. `cast<T>(value)` checks range/overflow and
cannot convert between enums and integers.

```uetd
signal error : f32<m> = reference.position - estimate.position;
signal covariance : f32[3, 3];
signal corrected = select(estimate.valid, estimate.position, safe_position);
```

Operators have this precedence, from tightest to loosest: indexing/member
access/slicing; right-associative `**` power; unary `+`, `-`, and `not`;
left-associative `*` and `/`; left-associative `+` and `-`; one comparison;
`and`; then `or`. Boolean operators short-circuit left to right. `^` is not an
arithmetic operator; it is reserved for unit exponents such as `m/s^2`, avoiding
collision with C's XOR spelling. `**` accepts a scalar base and a
compile-time-known integer exponent from -16 through 16; arrays, matrices, and
affine absolute quantities are not exponentiation operands. Unit dimensions
and exact scales are raised by that exponent, and a negative exponent checks
division by zero. Use `sqrt` for supported square roots. Addition/subtraction
on fixed arrays is elementwise with identical shapes and compatible unit
mapping. `*` supports scalar arithmetic and scalar scaling under the selected
numeric profile. Matrix/matrix or matrix/vector products use a registered UETD
operator overload whose exact types and shapes are determined by the same
binding used for its named function; ambiguous vector/vector and elementwise
matrix products are rejected. Other aggregate kernels use a registered UMATH
function inside a custom algorithm or nonlinear model. Comparisons return one scalar
boolean: numeric operands require matching units and shapes, booleans/enums
support equality only, and records/arrays are not compared wholesale.
`map(field = expression, ...)` constructs a declared record, with every field
present exactly once. `select(condition, yes, no)` evaluates only the selected
branch and both values must have identical types, units, and shapes. A math
function is written by its UETD operation name, for example `sin(angle)` or
`multiply(left, right)`; overloaded operations such as `cross(a, b)` resolve
from their UETD-declared argument types, shapes, and units. Family-qualified
compatibility aliases remain available. The C `umath_*` symbol is never
written in the project. Every registry operation, alias, and signature appears
in the generated guide and editor completions. Calls are allowed only in custom
algorithm bodies and nonlinear model implementation bodies. Their binding
contracts lower DSL values/results to the C ABI, provide fixed output/work
storage, and turn UETD status failures into the enclosing computation's
failure policy. Each binding declares its type, unit, shape, aliasing,
numeric-effect, availability, and bounded-work rules.

Operators use the same checked binding path when a UETD contract declares an
overload for the operand types and shapes. Matrix `*`, for example, may lower
to a registered matrix-multiply binding, while scalar `*` follows the
project's checked numeric policy. An overload cannot expand into a scope where
UMATH calls are forbidden, bypass readiness checks, or hide an ambiguous
shape. The base language does not define punctuation for remainder, bitwise,
or shift operations; module-owned integer operations state their width and
signedness behavior.
Indices are zero-based. Slices are half-open read-only values; assignments may
target only a whole declared value, record field, or single array element.

Custom algorithm and nonlinear model bodies may use conditionals and loops
only when every loop range is fixed or has an explicit statically checked
maximum. Nested iteration counts are included in the operation budget.
Assignments are limited to declared outputs, state, and locals; every output
must be assigned on every successful path. No recursion, file access, I/O,
dynamic allocation, pointer arithmetic, arbitrary method calls, or execution
of embedded user source is available. A registry function call is not
arbitrary code: its operation name resolves to exactly one UETD UMATH overload
with a typed binding contract, module, and readiness record.

Every project selects a numeric policy. It specifies scalar and accumulator
types, rounding, overflow, non-finite, fused-operation behavior, and matrix
layout. Integer division truncates toward zero; the rounding field controls
real-valued operations and explicit numeric casts. The selected UETD backend
must implement those semantics for the target or report a blocking
compatibility error; it must never substitute compiler defaults. Division by
zero, invalid math domains, overflow, non-finite results, UMATH error statuses,
and failed run-time bounds checks report a component error. The component and
operating-system failure contracts determine the response. Custom algorithm
outputs and next-state values commit together only after a successful
activation, so a failed calculation cannot publish a partial result.

The DSL covers a complete application by keeping each concern in its owner:
UETD registries provide targets, peripherals, software-module contracts,
runtime/type/unit/math contracts, and implementation templates; `.uef` provides one
target's values, models, custom equations/protocols, component composition,
mode machine, and schedule; `.uefs` provides independent targets, shared wire
types/protocols, links, and delivery policies. The lowered typed specification
must retain every authored field and source span. If required semantics has no
UETD or language lowering contract, `check` reports it and generation stops
instead of dropping the field. This makes the representation sufficient for a
complete UETD system when corresponding UETD contracts and generation routes
exist; it does not claim those backends are implemented today.

## Declarations

| Declaration | Purpose |
| --- | --- |
| `sensor name : descriptor;` | Bind a UETD sensor contract to application signals. |
| `actuator name : descriptor;` | Bind a UETD actuator/output contract and its safe behavior. |
| `algorithm name : descriptor;` | Instantiate one UETD-native algorithm. |
| `protocol name : descriptor;` | Instantiate a UETD-native protocol. |
| `watchdog name;`, `fpga name;`, etc. | Instantiate another directly authorable UETD module by its own descriptor. |
| `enum Name : u8 { ... }` | Define a nominal fixed-width value type; values are explicit when they cross a wire or persistent boundary. |
| `custom algorithm name;` | Define a project-local bounded algorithm absent from UETD. |
| `hybrid algorithm name;` | Compose declared algorithms through ordered calls, typed selection/fusion rules, and a defined failure response. It is a language construct, not a UETD module/strategy. |
| `custom protocol name;` | Define a fixed-storage project-local wire contract. |
| `subsystem name;` | Wrap declared components behind a typed interface and explicit ordered calls. It contains no algorithm equations. |
| `model name : continuous|discrete;` | Describe plant or process dynamics with matrices or equations. |
| `operatingsystem name { ... }` | Select a UETD runtime; declare modes, tasks, and resource policy. |

`custom` is reserved for genuine project-specific mathematics or protocol
behavior. Ordinary application wiring, named values, typed records, subsystem
interfaces, and hybrid selection are DSL features, not reasons to create a
custom algorithm. A custom algorithm has typed inputs, outputs, state,
parameters, locals, and bounded lifecycle functions. It cannot contain
arbitrary C/Python code, imports, recursion, dynamic allocation, or
runtime-bounded loops.

## Subsystems and contracts

A subsystem is a reusable function-like wrapper around already-declared
components. It may contain sensors, algorithms, custom algorithms, hybrid
algorithms, protocols, actuators, or other subsystems. It states a typed public
contract, the update/freshness/clock rules for each port, and which members
execute in what order. Membership alone never means execution or wiring. A
subsystem is a fixed composition: each unguarded `stage` runs once, in source
order, and its named arguments bind the member inputs. Do not put selection
logic or mathematical equations here. Use a hybrid algorithm when the result
depends on selecting or fusing algorithm results.

```uetd
subsystem state_estimation;

implementation {
  subsystem state_estimation {
    input imu_sample : ImuSample { rate = 1 kHz; max_age = 2 ms; };
    input pressure_sample : PressureSample { rate = 100 Hz; max_age = 20 ms; };
    output estimate : VehicleEstimate { rate = 480 Hz; max_age = 3 ms; };
    output health : EstimatorHealth { rate = 480 Hz; max_age = 3 ms; };

    use attitude_filter; use navigation_filter;
    stage attitude = attitude_filter(imu = imu_sample);
    stage navigation = navigation_filter(
      imu = imu_sample,
      pressure = pressure_sample,
      attitude = attitude.attitude
    );
    output estimate = map(attitude = attitude.attitude, navigation = navigation.state);
    output health = map(attitude_valid = attitude.valid, navigation_valid = navigation.valid);
  }
}
```

Each input/output declares a type (record, scalar, fixed vector/matrix) and may
state its nominal update rate, maximum data age, and clock domain. A caller
passes subsystem inputs as named arguments on its task's `run` call. Inside the
subsystem, `use` gives declared members readable local names and each `stage`
invokes exactly one member with named arguments. A stage runs once in source
order for each subsystem invocation; its named result is available only to
later stages. A typed `let` declares an immutable expression at that point;
it may use inputs, earlier locals, or stage results but cannot call a component.
Every output is explicitly returned with `output`. This is function
composition with a visible call order, not a graph inferred from
connections. A component listed with `use` but never called by a stage is an
error. Members can be UETD modules, custom algorithms/protocols, hybrids, or
nested subsystems when their contracts provide a callable operation.

The task's `run` call supplies the public input arguments and invokes the
subsystem once per task activation. The completed result is committed as one
output snapshot under that instance's name; consumers must satisfy its declared
freshness and clock-domain contract. Direct component tasks use the same form,
with no argument list when the selected UETD operation has no inputs. The
verifier checks caller arguments, stage arguments, return types, units, shapes,
rates, freshness, and ownership against UETD or project contracts. Missing
ports, required arguments or outputs, stale data, and unhandled failures are
errors. A subsystem cannot define hidden behavior, silently convert a type, or
make a remote call.

## Hybrid algorithms

A hybrid algorithm is a project-level composition, not a registered strategy.
Its members are named declared algorithms; its interface, execution sequence,
selection conditions, output contract, failure response, and execution budget
are visible in the source. A hybrid may do ordered sequential fusion, branch
selection, or both; it is not limited to two fixed slots or one pre-registered
fusion policy. `stage` calls establish a pipeline explicitly. `choose` selects
among compatible results. `output` declares what leaves the hybrid.

```uetd
hybrid algorithm energy_planner;

implementation {
  hybrid algorithm energy_planner {
  input request : EnergyPlanRequest { rate = 1 kHz; max_age = 2 ms; };
  input thermal : ThermalFeedback { rate = 1 kHz; max_age = 2 ms; };
  output plan : EnergyTrajectory { rate = 1 kHz; max_age = 2 ms; };

    use explicit_policy as table;
    use online_solver as sqp;
  stage table_result = table(request = request, thermal = thermal);
  stage sqp_result = sqp(request = request, thermal = thermal)
    when request.force_online or not table_result.within_envelope;

  choose plan {
    when not request.force_online and table_result.valid and table_result.within_envelope: table_result.trajectory;
    otherwise: sqp_result.trajectory;
    }
    on_failure = invalid_plan;
    execution_budget = 1 ms;
  }
}
```

`choose` is ordered and lazy: the first true `when` wins, and later branch
expressions are not evaluated. `and` and `or` also short-circuit from left to
right, so a guard can safely check that an earlier stage ran before reading its
result. `otherwise` is required unless the output explicitly allows an invalid
result. Every branch must return the same type, unit, shape, and update
contract. A guarded stage is skipped when its condition is false; a later
choice that reads its result must prove the result is valid or select another
branch. Verification checks member ports,
stage order, branch reachability, failure behavior, and bounded cost. `enmpc`
is an ordinary UETD algorithm, not itself the hybrid. The project hybrid states
how ENMPC and a fallback solver are connected.

The same rules express the full controller hybrid, where a continuous result
feeds a finite-set switching controller:

```uetd
hybrid algorithm coil_controller {
  input estimate : CoilEstimate { rate = 1 kHz; max_age = 1 ms; };
  input request : EnergyRequest { rate = 1 kHz; max_age = 1 ms; };
  input previous : CoilSetpoint { rate = 1 kHz; max_age = 1 ms; };
  output command : CoilSetpoint { rate = 1 kHz; max_age = 1 ms; };

  use energy_planner as continuous;
  use switching_mpc as finite_set;
  stage plan = continuous(estimate = estimate, request = request);
  stage selected = finite_set(
    estimate = estimate,
    continuous_reference = plan.reference,
    previous = previous
  );
  output command = map(
    duty = plan.duty,
    frequency_shift_hz = plan.frequency_shift_hz,
    audio = plan.audio,
    switching_gate_pattern = selected.switching_gate_pattern,
    enable = plan.valid and selected.valid
  );
  on_failure = disabled_setpoint;
  execution_budget = controller_budget;
}
```

The table/SQP hybrid supplies continuous reference and energy values. Finite-set
MPC evaluates only the reviewed switching patterns and returns an opaque
gate-pattern identifier. The command carries both outputs coherently; the
target driver remains responsible for mapping approved patterns to electrical
outputs and enforcing dead time and inhibit behavior.

## Models, algorithms, and protocols

A `model` states the dynamics a control or estimation algorithm relies on.
Linear models declare dimensions, explicit `state_order`, `input_order`, and
`output_order`, plus `A/B/C/D`; the declared order fixes matrix flattening and
the checker derives each element's unit from its output row and state/input
column. Nonlinear models declare every derivative/next-state value and every
output in a bounded implementation function. That function may call UEF-PL
UMATH operations such as `multiply`, `mul`, `cross`, quaternion operations,
and other registered kernels without
expressing every computation as a specially proven equation node. The checker
still parses the small language grammar and validates call types, dimensions,
units, buffers, bounds, statuses, and readiness; it never executes arbitrary C.
Continuous-time models must name the integration/discretization method and
sample time. Records may group state values with different units; homogeneous
arrays cannot silently mix metres, velocity, and angles.

A `custom algorithm` declares typed inputs, outputs, parameters, persistent
state, fixed workspaces, and lifecycle functions such as `initialize`,
`predict`, `update`, and `step`. Its body can use bounded control flow and any
public UMATH function registered by UETD. Calls use the UEF-PL name and its
typed DSL signature; they do not expose C pointer arguments, output pointers,
or status-enum plumbing. A function binding records how logical values map to
C arguments and output buffers. The shared backend inserts status checks and
follows the enclosing component's failure policy. There is no user
function import mechanism and no arbitrary C/Python body. Unsupported
functions, unbounded cost, bad shapes, invalid units, or incompatible numeric
modes are errors; registered scaffold functions remain discoverable but block
generation with a diagnostic tied to their UETD manifest.

A `custom protocol` declares exact byte layout and finite behavior: transport
role, frame boundaries, version, byte order, field offsets/widths, sequences,
checksums, fragmented transaction rules, freshness, receiver timeout,
malformed-frame handling, and bounded queues. Protocol declarations describe
wire-format facts, not physical pin maps or handwritten drivers. UMATH calls
are prohibited in protocol declarations and protocol-runtime descriptions.
Protocol coordination uses registered protocol operations and OS channels;
V1 has no `hybrid protocol` construct.

The registered UETD `fpga` module is MCU-side endpoint bookkeeping for an
externally coded FPGA: link qualification, whether a complete valid update has
arrived, and its receiver-local timestamp. Its `send.<message>`,
`receive.<message>`, and `fresh.<message>` ports are derived from the selected protocol and the
endpoint role in the `.uefs` link contract. Each message carried by that link
must be bound exactly once, either to a component port or to a typed operating
system channel. The link determines direction; the compiler must reject a
message bound at the wrong endpoint or a message with no application binding.

Each active protocol message names its logical record type, and every wire
field names the corresponding record field with `application_field`. Fields
whose wire width or numerical format differs from the application value must
also declare range, scale, rounding, and overflow behavior. This prevents the
encoder from guessing a mapping from similar field names. Protocol receive
timestamps and sequence numbers are receiver-generated envelope metadata on
`receive.<message>` and are not serialized unless the protocol explicitly
declares wire fields for them. A receive port has type `Received<T>`, with
`value : T`, the protocol sequence, and `received_at_us` from the local monotonic
clock; a send port consumes the logical `T` value and generates wire framing,
sequence, and checksums. UETD builds the MCU-side codec and connects these ports;
it adds no message fields or peer identity handshake and does not generate FPGA
RTL.

For example, the Tesla G474 endpoint sends each raw timer capture as a typed
`measurement` record. The H745 M4 receives the frame and extends its native G4
timer ticks; an explicit M4-to-M7 channel carries the reconstructed phase into
the M7 sensor snapshot. The M7's setpoint travels back through a typed mailbox
to the H745 endpoint's `send.setpoint` port. The G4 endpoint receives the
committed setpoint and keeps local freshness, interlock, and timer authority.

## Operating system, tasks, and tasksets

`operatingsystem` is the execution contract for one target. It owns startup and
readiness, the application FSM, runtime, loop/task activation, interrupts,
channels, resource ownership, timing, failure policy, and safe output
transitions. The source reads as the execution outline; the referenced runtime
and target contracts give each property its exact meaning.

Initialization follows declared module dependencies. Safety-critical outputs
remain in their safe state until required initialization succeeds. Every
activation reads a consistent input snapshot, calls its declared component or
subsystem, and commits output/state only on success. Failed calls, stale input,
overflow, or missed deadlines follow a declared policy; the compiler invents
no retry, stale-data reuse, or priority tie-break.

`runtime = BareMetal` declares exactly one root `loop name { ... }`. The
generator lowers it to the application's `for (;;)` main loop, with monotonic
UETD time used for declared periods when the selected target supplies a timer.
The loop has one callable invocation and explicit rate or free-running,
deadline, and overrun/failure behavior. It is cooperative: it has no task
creation, preemption, blocking synchronization, or FreeRTOS-style queue
guarantees. It may call a subsystem once per pass and must not declare tasks or
a `taskset`.

`runtime = FreeRTOS` declares statically allocated tasks with explicit priority,
stack, period or event trigger, deadline, core affinity, guard, and missed-
deadline response. The registered runtime manifest defines the actual
scheduling and synchronization support. Tasks may be direct children of the
OS root. `taskset` is optional and only groups work that shares a scheduler,
core, memory/channel policy, resource ownership, or interrupt boundary; a lone
group with no shared policy is omitted. H745 M7/M4 tasksets are meaningful;
links between separate MCUs belong in `.uefs`, not a taskset.

An interrupt handler is restricted to operations marked ISR-safe by both the
target and module contracts. It may signal a task or acknowledge hardware, but
does not run control algorithms or blocking calls. One peripheral/IRQ has one
declared owner unless its UETD contract provides a safe shared service.

Project-owned callbacks are declared separately from UETD operations with
`project routine name() returns status;` or
`project routine component.name() returns status;`. These zero-argument
contracts have bodies supplied by the project integration. State-machine
hooks must return status so callback failure reaches UETD; a `void` callback
cannot be used as an FSM hook. The default context is `task`. An interrupt may
call a project routine only when that declaration and the interrupt both
specify `context = isr_signal_only`; a task or loop named directly as the
interrupt handler is deferred work and also requires that interrupt context.
This keeps a missing UETD operation contract from being mistaken for a project
callback, while preserving an explicit declaration path for project-owned code.

A channel is an explicit typed route with bounded capacity, overflow, freshness,
and ownership rules. `source`, `destination`, `producer`, and `consumers`
describe the route and its scheduled endpoints. `consumers = [task_a, task_b]`
means broadcast: lowering provides an independent bounded mailbox to each
subscriber. A shared queue with several receivers is invalid because it is
work-sharing and one task can consume another task's message. When
`destination` is a component input, exactly one subscribed task must run that
component; the channel supplies that task's input, so the argument must not
also appear in its `run` call. Additional subscribers read their independent
`channel.value` snapshots. A `TASK.trigger` destination is an event route:
`channel.not_empty` wakes the listed task and acknowledges one queued event at
activation start. An `ENDPOINT.send.MESSAGE` destination queues the typed
outbound message for the endpoint's service task. The verifier checks the
combined channel and explicit argument bindings against the complete callable
signature; declaration order never changes delivery or scheduling.

For example, a single-loop controller is explicit about the backend and its
only recurring work:

```uetd
operatingsystem BalanceRuntime {
  runtime = BareMetal;
  loop control_loop {
    run = balance_system();
    rate = 200 Hz;
    deadline = 5 ms;
    on_miss = set_motors_safe;
  }
}
```

`run` names one callable contract, `rate` establishes the release period,
`deadline` is measured against the same monotonic clock, and `on_miss` names a
registered safe action. If the target has no suitable timer or the runtime
cannot enforce a requested property, validation blocks generation. OS
declaration/property order is not a scheduler tie-break; all required ordering
must be stated by dependencies, triggers, FSM transition order, or the selected
runtime's documented scheduler.

```uetd
operatingsystem FlightRuntime {
  runtime = FreeRTOS;
  state_machine flight_mode : FlightMode {
    state disarmed;
    state armed;
    initial disarmed;
    owner = safety_monitor;
    consumers = [];
    transition disarmed -> armed when safety.arm_permitted;
  }
  task safety_monitor { run = safety; rate = 1 kHz; priority = 7; deadline = 1 ms; }
  taskset m7_estimation { core = M7; scheduler = fixed_priority;
    task imu_update { run = state_estimation; trigger = imu.data_ready; rate = 1 kHz; priority = 6; deadline = 800 us; }
    task navigation_update { run = navigation_filter; rate = 480 Hz; priority = 5; deadline = 2 ms; }
  }
}
```

The FSM declares states with `state NAME;` before `initial`, then exactly one
`owner` and one `consumers` list before its transitions. The identifier after
`:` names the synthesized fixed-width enum type. The owner is one RTOS task or,
for BareMetal, the sole cooperative loop. It alone initializes and calls the
UETD FSM API. Consumers are RTOS tasks that receive independent latest-value
state snapshots at their own scheduled activation; the owner is an implicit
reader and must not be repeated. Listing a consumer does not wake or
reprioritize it; use a channel trigger or registered notification if a task
must run on a state change. A task reading another machine's `.state` must
appear in that machine's consumer list. An owner task cannot be gated with
`when`. Every operating system declares one `component_failure_policy` for
task, loop, and FSM callback failures.

At activation start, the owner receives and binds its synchronized channel
inputs, then runs its component invocation. A failed invocation follows
`component_failure_policy` and does not evaluate guards against incomplete
outputs. After success, the owner captures one immutable snapshot of those
inputs and the completed outputs, then evaluates its machines in declaration
order. Machines owned by one task share the
activation-start state view, so a commit cannot change a later machine's guard
during that activation. Each machine commits at most one transition. A
BareMetal loop ticks its machines once per iteration. The future adapter maps
each activation to one private synthetic UETD tick event and expands `any`
transitions into exact-source rows while preserving priority. RTOS ingress
arrives through synchronized UOS queues/notifications and updates the owner's
snapshot; it does not call the UAPP instance directly. BareMetal uses
cooperative bounded delivery and ISR-safe latches, not unsupported RTOS queues
or mutexes.

Each consumer receives its own race-free latest-value state mailbox, which
replaces an older unread snapshot and does not preserve state-change history.
This differs from channel delivery, where each listed consumer receives an
independent bounded FIFO mailbox. The future adapter must implement both
publication rules using the selected runtime's supported synchronization.
Generated FSM callbacks must not post arbitrary events to the owning UAPP
instance; UEF-PL transitions are guard evaluations over the activation
snapshot. The owner coordinates tasks through declared state/command routes,
but it does not control the RTOS scheduler or synchronously stop another task.
Tasks that affect outputs must obey that state/command contract or route
outputs through an explicitly owned actuator path. Safety-critical inhibition
needs an independent hardware or UETD safety path.

The first true matching transition wins. `any` includes its destination; a
same-state transition runs `on_exit` and `on_enter` again. Persistent level
guards can repeat entry actions, so exclude a latched destination when that is
not intended. Hooks run synchronously on the owner, must be bounded and
non-blocking, and return UETD status. Failure latches the machine fault and
follows the OS component-failure policy; outputs must be made safe before
reset. An FSM owner coordinates other tasks through declared state and command
routes but does not own RTOS scheduling or stop other tasks instantaneously.
Safety-critical inhibition needs an independent hardware or UETD safety path.

The frontend parses, validates, and lowers this contract but does not yet
provide executable firmware. UETD's C API is event-driven, requires exact
source states, and has no `any` or snapshot-evaluation operation. Firmware
generation remains blocked until the adapter, bounded ingress and fan-out
storage, callback failure behavior, and target-specific schedule contracts
are implemented and verified. Lowered machine records carry the required state
capacity and expanded transition-row capacity. The emitter must set UETD's
shared `UAPP_SM_MAX_STATES` and `UAPP_SM_MAX_TRANSITIONS` to at least the
largest per-machine requirements and publish those same definitions to all
translation units; each capacity is capped at 255 by UETD's `u8` counts.

## Multi-target applications

A `.uef` file describes one independently built target. A distributed
application uses a `.uefs` system source to name the target projects, define
shared project-owned link contracts once, bind local endpoints, and describe
their deployment relationship. It does not merge remote modules into a local
application, create a NEXUS graph, or make remote ports callable like local
functions. Every link has explicit message types, direction, transport,
protocol-defined versioning (when the wire format has it), sequence/freshness,
timeout, recovery, and delivery semantics. The language does not invent a
contract-ID handshake. Each node retains local output authority and safe-state
behavior.

Shared records, enums, and protocols in a `.uefs` file are collected in a
contract pass before its node projects are compiled. A node's `contract import`
loads those declarations and the link-specific endpoint facts for its local
endpoints; it does not recursively compile the system or its sibling projects.
For example, the G474 board profile owns the timer rate and counter width for
capture samples, and the `.uefs` telemetry link exposes that same profile to the
H745 endpoint as `controls_endpoint.capture_clock`. The H745 phase-reconstruction
algorithm therefore uses the sender's exact timer profile without duplicating
it as a second project value. This keeps one owner for shared wire and endpoint
facts without creating a project/system import cycle.

The QCW DRSSTC example uses one STM32H745XIH6 controller (M4 capture timestamp
extension and phase reconstruction; M7 sensor snapshot, estimation, and
control), one STM32G474VET6 local executor, and paired external FPGA endpoints.
The G4 captures the timer edges and transmits their native counts. The H745
`energy_planner` hybrid chooses an explicit
ENMPC policy result or an online NMPC-SQP fallback. Its second `coil_controller`
hybrid passes that continuous result into finite-set switching-state MPC and
emits one coherent command. The G4 validates freshness and local safety before
staging that command at a timer-safe update boundary. Exact boards, cross-MCU
transport, timer/pin mapping, plant model, policy table, solver and measured
timing remain project inputs.

The G4 local PLL is measurement data; it does not take over actuator authority
during link loss. Link loss, stale commands, or failed local qualification
inhibit output. The controller-to-executor link uses the specified 10-byte
logical frames and eight-fragment setpoint transaction. MCU-side `fpga`
declarations use the project protocol for message encoding, parsing, freshness,
and typed ports. They do not define FPGA RTL or invent a separate
contract-ID/hash handshake.

UETD's future registry currently describes PLC process-image and external
artifact qualification as design-only work. PLC is expected to become a sibling
endpoint category to FPGA after its contract is promoted into the main module
registry. Until then UEF-PL does not invent a `plc` declaration or claim that
PLC endpoint generation is supported.

## Editor, verification, and CLI

The planned commands are:

```text
uef-gen check flight_controller.uef
uef-gen generate flight_controller.uef
uef-gen build flight_controller.uef
uef-gen flash flight_controller.uef
```

The Visual Studio and VS Code extensions connect to the same language server
for diagnostics, completion, hover, signature help, definition/reference and
implementation navigation, rename, document/workspace symbols, semantic
tokens, formatting, folding, and quick fixes. These editor actions resolve
from the shared typed AST and live UETD catalogue; the TextMate grammar provides
immediate coloring while semantic tokens distinguish resolved names. The
`.uef`/`.uefs` checker and backend-neutral lowering are implemented. The
standalone firmware emitter, complete `.uefs` system emitter, build, and flash
routes remain incomplete and must report an explicit unavailable diagnostic
instead of returning a success-shaped placeholder.

For the flight-controller parity target, the important check is that the DSL
preserves component identities, signal units/shapes, state transitions,
controller equations and parameter values, multi-rate schedule, safety/output
authority, and target-owned peripheral bindings in typed form. UETD templates
then produce the implementation. The drone is the strongest single-target
compatibility example; the coil pair exercises multi-target contracts. A
manual parity claim remains conditional until the relevant UETD contracts and
backend routes exist and generated output is compared with those source
projects.

## V1 boundary

V1 includes project validation/generation, bare-metal `loop()` projects,
registered RTOS targets, and wired serial debug output. It may print
diagnostics from the CLI and stream logs over a cable while the program runs.
Wireless debug/flash protocols, NEXUS graph editing, and user-authored
general-purpose code are outside the initial standalone language.

The examples are: a generic differential-drive robot; the source-derived
STM32F722 drone; the multi-target QCW Tesla coil; an Abyssal ROV using model
predictive control and disturbance rejection; and a high-complexity active
vibration-isolation platform. Values that depend on a selected carrier, plant,
or measured safety case are clearly illustrative rather than commissioning
values.
