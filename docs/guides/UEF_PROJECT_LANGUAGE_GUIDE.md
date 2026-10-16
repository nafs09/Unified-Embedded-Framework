# UEF Project Language (UEF-PL): technical guide

This document describes the source structure and intended semantics of UEF-PL
for people authoring projects and implementing the compiler. For a first-time
user walkthrough, start with the [UEF-PL user handbook](UEF_PROJECT_LANGUAGE_HANDBOOK.md).

UEF-PL is a typed, declarative language for describing one embedded target in
`.uef`, or a set of independently built targets and their links in `.uefs`.
Its declarations and implementation blocks use a C-like order and layout, but
the language does not embed C, C++, or Python. `uef-gen` is intended to lower
the checked description into UETD components and target-specific project files;
NEXUS is not required.

> **Implementation status:** the `.uef`/`.uefs` lexer, parser, registry
> discovery, semantic checker, and lowering are implemented. The standalone
> firmware emitter is not, so projects can be checked but not yet generated,
> built, or flashed through this language pipeline. Examples describe intended
> designs and are not proof of a physical board configuration.

## File structure

A `.uef` source is read in this order:

1. `project` identity, exact target, numeric policy, and project-owned types.
2. Component declarations using role keywords and registered UETD descriptors.
3. One `operatingsystem` block describing runtime behavior and activation.
4. One `implementation` block containing bindings and detailed definitions.

The operating-system block appears before the detailed implementation so a
reader sees the system's runtime structure before the individual settings.
Project types are fixed-layout records, fixed-width enums, and fixed-size
arrays or matrices. Unspecified target parameters remain explicit inputs; the
compiler must not guess hardware values.

The following excerpt shows the shape; it omits the detailed component and
subsystem definitions that a complete source file needs.

```uetd
project MotorController {
  target = "<registered-board-id>";
  numeric {
    scalar = f32; accumulator = f64;
    rounding = nearest_even; overflow = reject;
    non_finite = reject; fused_multiply_add = disabled;
    matrix_layout = row_major;
  }

  record WheelSample { speed : f32<m/s>; valid : bool; }
  sensor wheel : encoder;
  algorithm speed_loop : pid;
  actuator motor : h_bridge;
  subsystem motor_speed_loop;

  operatingsystem Main {
    runtime = BareMetal;
    loop control { run = motor_speed_loop(); rate = 200 Hz; deadline = 5 ms; }
  }

  implementation {
    // Bind the registered sensor, algorithm, actuator, and subsystem here.
  }
}
```

This is a structural example. A target ID, module descriptor, and every
component setting must resolve to actual UETD registry data before a real
project is accepted. See the runnable-shape examples in
[`uef-gen/examples`](../../uef-gen/examples/README.md).

## Components and UETD names

Role keywords make the component's purpose explicit: `sensor`, `actuator`,
`algorithm`, `protocol`, `peripheral`, and `bus`. A declaration such as
`sensor wheel : encoder;` resolves `encoder` through the canonical
`DSL_descriptor` in UETD's module index. Modules without a role keyword use the
registered descriptor directly, for example `watchdog safety;` or
`fpga coil_link;`.

Every UETD-backed implementation repeats the descriptor after its closing
brace, for example `algorithm tonal_probe { ... } goertzel;`. The suffix must
match the descriptor in the constructor declaration (`algorithm tonal_probe :
goertzel;`). This applies to registered modules and role-based components;
project-defined `custom algorithm`, `custom protocol`, `model`, `hybrid
algorithm`, and `subsystem` definitions do not take a suffix.

`uef-gen` does not maintain a second list of UETD module names. The module index
provides discovery; the module manifest provides source/dependency information
and, when present, the authoring contract for settings, typed ports,
operations, limits, readiness, and code-generation route. A known descriptor
may still be unavailable for generation when that contract or implementation
is incomplete.

`peripheral` declarations describe target resources such as timers, GPIO,
interrupts, or buses. Their valid roles come from the selected target's UETD
catalogue. Board pin assignments and physical driver details remain explicit
target bindings; the language does not generate handwritten device drivers.

Project callbacks are declared explicitly in the project header. Use
`project routine safety.begin_controlled_descent() returns status;` for a
zero-argument callback owned by the project and associated with `safety`.
`project routine assert_hard_kill();` declares a project-level callback without
a component owner. These declarations are call contracts, not embedded source
bodies; the project/backend provides the callback implementation. Their default
execution context is `task`. State-machine hooks must return `status`; a void
routine cannot report failure to UETD and is rejected as an FSM hook. An ISR
callback must be declared as `project routine safety.signal_trip() context =
isr_signal_only;` and its interrupt must also set `context = isr_signal_only`.
State-machine entry and exit actions may call one of these routines or a
zero-argument, task-context, status-returning operation listed in the
component's UETD authoring contract. Interrupt handlers may instead name a task
or loop as deferred work; such an interrupt must also declare
`context = isr_signal_only`.
An unfinished UETD contract produces a readiness warning and suggests an
explicit project-routine declaration when that is the intended ownership.

## Types, expressions, and units

UEF-PL values include `bool`, the fourteen UETD-backed numeric spellings below,
project records and enums, fixed-shape arrays/matrices, and `Received<T>` for
protocol receive values. `string` is limited to compile-time configuration
properties whose UETD contract allows it.

| UEF-PL type | UETD C alias |
|---|---|
| `f32`, `f64` | `uetd_f32_t`, `uetd_f64_t` |
| `i8`, `i16`, `i32`, `i64` | `uetd_i8_t`, `uetd_i16_t`, `uetd_i32_t`, `uetd_i64_t` |
| `u8`, `u16`, `u32`, `u64` | `uetd_u8_t`, `uetd_u16_t`, `uetd_u32_t`, `uetd_u64_t` |
| `q15`, `q16`, `q31`, `q63` | `uetd_q15_t`, `uetd_q16_t`, `uetd_q31_t`, `uetd_q63_t` |

The `numeric` block selects the UMATH `scalar` and `accumulator` formats. They
resolve to UETD's configured `umath_scalar_t` and `umath_accumulator_t` for the
whole generated program. A `q*` value type does not guarantee that every
algorithm or target supports that numeric mode; UETD's module, operation,
runtime, and target contracts must agree.

UETD's type index contains every C declaration discovered in its public
headers. `uef-gen` reads that index through `UETD/api.json`; the language does
not copy the list into its own registry. The
[generated UETD type catalogue](../reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md#public-uetd-c-types-and-enums)
shows each declaration's optional DSL spelling, kind, visibility, header
status, and owning header. The index supports discovery and editor help; it
does not make every C declaration a project value. Non-primitive by-value
types require a compatible owning UETD contract. Pointers, function pointers,
driver handles, and dynamic containers stay inside that contract. Types under
`future/` headers are indexed for visibility but are not ready for generation.

Conversions are explicit. There is no implicit integer widening, enum/integer
conversion, shape broadcasting, or unit conversion. `cast<T>(x)` changes the
numeric representation under checked range and overflow rules;
`convert_unit<T>(x)` converts between compatible registered units. Addition
and comparison require compatible value types and unit scales. Absolute
temperature values use affine conversion rules; temperature differences use a
separate unit.

Unit dimensions and exact scales are owned by
[`UETD/registry/units/registry.json`](../../UETD/registry/units/registry.json).
Use `^` for integer unit exponents (`m/s^2`) and `**` for numeric powers. The
[UEF-PL unit reference](../reference/UEF_PL_UNITS_REFERENCE.md) groups the
registered units by dimension and presents their mappings using mathematical
notation.

The language defines a fixed operator precedence and does not rely on C's
implicit conversions or unspecified evaluation behavior. See
[Types, Expressions, and Bounded Computation](../design/PROJECT_LANGUAGE_DESIGN.md#types-expressions-and-bounded-computation)
for the complete operator table and typing rules.

## UETD math in custom implementations

Project authors call the operation name, for example `cross(a, b)`, `add(a, b)`,
or `sin(angle)`, without writing the underlying `umath_*` C name or pointer
signature. When several UETD functions share an operation name, the compiler
selects an overload from the argument types, shapes, and units recorded in the
UETD binding contracts. Family-qualified compatibility aliases such as
`vec3_cross` remain available. The intended scope is deliberately narrow: UMATH calls are allowed
inside custom algorithm bodies and nonlinear model equations/steps. They are
not available in protocol declarations, hardware bindings, operating-system
guards, subsystem wiring, or ordinary component settings.

The UMATH registry maps public header declarations to operation names,
compatibility aliases, and typed overload contracts. Each callable contract
defines argument/result
types and shapes, mutation, units and numeric effects, bounded storage and
work, failure behavior, and availability. A header declaration by itself is
not enough to make a call valid. If any overload in a selected name group lacks
typed parameters, the compiler rejects the call instead of guessing. The generated
[catalogue reference](../reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md) lists
discovered names and their implementation and binding status.

## Models, algorithms, protocols, and composition

- A **model** describes plant or process dynamics. A linear model states fixed
  state/input/output ordering and matrices. A nonlinear model uses bounded
  equations or a restricted imperative body and may call registered UMATH
  functions. Continuous models identify their integration/discretization
  method and sample interval.
- A **custom algorithm** defines typed inputs, outputs, parameters, persistent
  state, and bounded lifecycle/step operations. It is the language mechanism
  for project-specific estimation, control, or numerical logic that is not a
  registered UETD algorithm.
- A **custom protocol** declares a wire format and its transport-facing
  contract: framing, field encoding, versioning, sequencing, checksum coverage,
  limits, timeouts, freshness, malformed-frame handling, and recovery. It
  does not contain handwritten driver code or arbitrary UMATH calls.
- A **subsystem** wraps explicitly declared components behind typed inputs and
  outputs. Its interface contract defines the units, rates/freshness, failure
  behavior, and each component binding. Its ordered stages define what runs;
  listing a component alone does not execute it.
- A **hybrid algorithm** composes registered or custom algorithm instances
  behind one typed interface and states the selection/fallback policy, state
  transfer, validity conditions, failure response, and execution budget. It is
  a project-level composition, not a UETD module name.

Standalone UEF-PL describes typed composition in source. It does not create
NEXUS graphs or arbitrary graph nodes.

## Operating system and timing

`operatingsystem` is the execution contract for a target. It declares runtime,
startup and readiness behavior, state machines, loops or tasks, channels,
interrupt restrictions, resource ownership, timing limits, failure policy,
and safe output transitions.

`runtime = BareMetal` uses one explicit cooperative root `loop`. It does not
provide preemption, task creation, blocking queues, mutexes, or semaphores.
`runtime = FreeRTOS` uses statically bounded tasks with declared priority,
stack, activation, deadline, and miss policy. A `taskset` is optional and has
a purpose only when it groups tasks under a shared scheduling, core, memory,
or resource policy; it is not required when there is only one task.

Activations use explicit input snapshots and commit results only on success.
The design rejects silent stale-data reuse, implicit retries, unbounded loops,
recursion, dynamic allocation, and unbounded workspaces. State-machine
transitions use a declared evaluation order and one state commit per owner-task
activation. All hardware, scheduling, queue, and memory limits must be
supported by the exact target contract.

State actions use `on_enter state call component.operation;` or
`on_exit state call component.operation;`. A call resolves either to an
explicit `project routine component.operation();` declaration or to an
operation in that component's UETD contract. This keeps project callbacks
distinct from UETD-provided operations and prevents a missing UETD contract
from being mistaken for a valid call.
Project routines may return `void` or UETD `status`. State-machine hooks must
use `returns status` so a failed action reaches the UETD fault latch. UETD
operations used as hooks must publish `result = status` in their authoring
contract and accept no arguments. Hooks execute synchronously in the owner
context, so they must be bounded and non-blocking. A generated UEF-PL hook must
not post arbitrary events to its owning FSM: the project FSM evaluates its
guard snapshot on one synthetic owner-activation tick, while external input
arrives through the owner's declared ingress routes.

`any` includes the destination state. A same-state transition runs `on_exit`
and `on_enter` again; a persistent safety level can therefore repeat entry
actions every evaluation. Exclude the destination in a latched guard when the
machine should stay in that state. Put fault-latching transitions before other
global transitions, and make terminal/latched states reachable only through an
explicit recovery policy.

Each state machine names exactly one `owner` task (or the sole BareMetal loop)
and one explicit `consumers = [task_a, task_b]` list. The owner alone
initializes, evaluates, resets, and calls callbacks on the UETD FSM object.
Consumers receive independent read-only latest-value copies of committed
state at their own scheduled activation. Listing a consumer does not wake or
reprioritize it; use a channel trigger or registered notification when a task
must run on a state change. Owner tasks cannot use `when`; their rate/period or
trigger defines the activation boundary. At activation start, the owner
receives and binds synchronized channel inputs, then runs its component
invocation. After success, it snapshots those inputs and completed outputs
once, evaluates its machines in declaration order, and commits at most one
transition per machine. State reads
in a task or another FSM require that task/owner to be listed as a consumer.

The current `.uef` path only parses, checks, and lowers state machines to
backend input. The future UETD adapter maps one owner activation to one
synthetic tick event, expands `any` into exact-source rows, updates the owner's
input snapshot from synchronized RTOS ingress, and publishes an independent
latest-value state copy per consumer. Those state copies are race-free
per-consumer mailboxes that replace stale snapshots; they are not FIFO event
queues. The UETD FSM's internal event queue remains owner-only and is not a
multi-task ingress channel. Under FreeRTOS, the intended input route is a
dedicated UOS ingress queue, separate from both the FSM queue and the state
mailboxes. The current UOS result/context contract and exact target
qualification are incomplete; see the [UOS contract](../../UETD/docs/design/UOS_CONTRACTS.md).
BareMetal uses its single cooperative loop and requires a target-qualified
ISR latch or ring before accepting interrupt input; the current UOS backend
does not provide that mechanism or RTOS queues/mutexes. The OS must
declare exactly one `component_failure_policy` for task, loop, and FSM callback
failures. An FSM owner coordinates application work through explicit routes but does not
control the RTOS scheduler or stop other tasks synchronously. Tasks that can
affect outputs must either obey the published state/command contract or route
their output through an explicitly owned actuator path. Hardware-critical
inhibition remains independent. Callback failures latch the machine fault and
follow the OS failure policy; recovery must establish a safe output state
before reset.

### State-machine syntax

Declare the state set inside the machine before choosing its initial state or
listing transitions:

```uef
state_machine flight_mode : FlightMode {
  state disarmed;
  state armed;
  state failsafe = 7;
  initial disarmed;
  owner = safety_monitor;
  consumers = [];
  transition any -> failsafe when safety.trip_latched;
  transition disarmed -> armed when safety.arm_permitted;
  transition armed -> disarmed when not receiver_snapshot.value.valid;
}
```

The type after `:` is synthesized from these declarations with `u8` storage;
it can be used by component ports and expressions as `FlightMode`. Unassigned
state values use declaration order beginning at zero. Explicit values must be
unique and fit in `u8`. Do not declare a parallel `enum` for this state type.
Exactly one `initial` state must name a declared state. Transition guards read
one immutable activation snapshot; the first matching transition in source
order wins. `owner` is required; `consumers` is required even when empty. Use
`any` for a source-independent transition and put safety transitions first
when they need priority. Optional `on_exit state call component.operation;`
and `on_enter state call component.operation;` actions must refer to
zero-argument status-returning operations in the component's UETD contract or
project routines declared `returns status`.

## `.uefs` system files

A `.uefs` file combines independently built `.uef` projects and declares
shared types/protocols, endpoint bindings, link direction, transport, freshness,
timeouts, and recovery. A remote endpoint is a communications contract, not a
local function call. `fpga` describes the MCU-side protocol endpoint; UETD does
not generate FPGA RTL. PLC endpoint semantics remain future work until their
contracts are promoted into the active UETD registry.

## Registries and editor support

The [registries and discovery guide](UEF_PL_REGISTRIES.md) explains the JSON
ownership model, how `uef-gen` locates a UETD checkout, and which file to edit
when a module, type, unit, target, or grammar changes. The machine-generated
[catalogue reference](../reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md) is an
inventory, not a second source of truth.

The repository includes VS Code and Visual Studio extensions. Both associate
`.uef` and `.uefs`, use the same TextMate grammar, and can connect to the shared
`uef-gen-language-server`. The Python server analyzes UEF-PL; LSP is a
language-neutral editor protocol. It provides completion, hover, diagnostics,
signature help, local and registry-backed definition navigation, references,
rename, document/workspace symbols, semantic tokens, formatting, folding, and
quick fixes. Visual Studio presents linked Quick Info through its native
editor surface. Editor behavior depends on the installed extension version and
the server process using the current checkout; see the [editor integration
overview](../../uef-gen/editors/README.md) for build and troubleshooting steps.

## CLI status

The currently implemented frontend commands are:

```powershell
uef-gen check project.uef
uef-gen generate project.uef --dry-run
```

`uef-gen check` runs the implemented lexer, parser, UETD registry discovery,
semantic validation, and lowering. `generate --dry-run` validates and resolves
the project without writing firmware files. The firmware emitter is not
implemented, so normal generation, build, and flash remain unavailable for
UEF-PL projects. See the [implementation status](../../uef-gen/README.md#project-language-frontend-scaffold)
for the current generation boundary.

## Example projects

Examples are grouped by how much of the design they demonstrate:

- [Bare-metal loop controller](../../uef-gen/examples/basics/bare_metal_loop_controller.uef)
- [Generic robot controller](../../uef-gen/examples/basics/generic_robot_controller.uef)
- [STM32F722 drone controller](../../uef-gen/examples/showcases/stm32f722_drone_controller.uef)
- [Abyssal ROV](../../uef-gen/examples/showcases/abyssal_rov.uef)
- [Tesla-coil multi-target system](../../uef-gen/examples/showcases/tesla_coil/tesla_coil_system.uefs)

They are language/design examples and are not currently flashable generated
projects.
