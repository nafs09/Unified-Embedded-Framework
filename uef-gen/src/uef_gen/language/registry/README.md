# uef-gen language registry

This package owns the standalone `.uef` grammar, syntax-version metadata,
fixed parser/lowering dispatch IDs, editor grammar facts, and language-only
value/composition forms. It does not contain chips, sensors, algorithms,
protocols, math functions, runtimes, or other UETD module catalogues.

Language version 13 uses a C-like order: fixed typed records and enums, module
signatures, one `operatingsystem` behavior outline, then one detailed
`implementation` block. It supports UETD-backed role forms, direct UETD module
identifiers, project-local `custom algorithm`/`custom protocol` bodies,
contract-bearing `subsystem`s, general `hybrid algorithm` composition, dynamic
models, the BareMetal root `loop`, RTOS tasks, and optional `taskset` scopes.
State machines declare their states inline with `state NAME;` before the
initial state and transitions. Those declarations synthesize the named
fixed-width enum type after the colon, so projects do not keep a separate enum
list synchronized with the FSM. Every UETD-backed implementation ends with
the same `DSL_descriptor` selected by its constructor declaration, for example
`algorithm tonal_probe : goertzel;` and
`algorithm tonal_probe { ... } goertzel;`. A mismatch is a source error.
Project-defined custom algorithms/protocols, models, hybrids, and subsystems
keep their own language-defined forms and have no UETD suffix. The
lexer/parser, semantic validation, type interchange, and lowering are
implemented; firmware emission is not.

`project routine name()` and `project routine component.name()` declare
zero-argument project callbacks with an optional `returns void|status` result.
They are explicit call contracts, not embedded source bodies. FSM hooks require
`returns status`; UETD operations used as hooks must publish `result = status`
and accept no arguments. The default execution context is `task`; use
`context = isr_signal_only` only for a callback safe to invoke from an ISR.
Both the declaration and interrupt must state that context. A task or loop can
also be named as an interrupt handler as deferred work, with the interrupt
marked `isr_signal_only`. FSM actions resolve against project callback
declarations or the selected UETD module's published operation contract; a
draft UETD contract does not silently authorize arbitrary operation names.

Every state machine names one owner and its explicit state consumers. The
owner alone calls its UETD FSM instance. The lowered guard model runs once per
owner activation: after a successful owner invocation, it captures one
immutable input snapshot, processes one private synthetic tick, and publishes
the committed state to independent latest-value mailboxes for consumers. Those
mailboxes replace stale snapshots and do not preserve state-change history.
Consumers are sampled on their own scheduled activations; listing one does not
wake or reprioritize its task. Use a channel trigger or a registered
notification when work must run because the state changed. Every operating
system declares one `component_failure_policy` for task, loop, and FSM hook
failures.
External RTOS inputs reach the owner through synchronized UOS ingress; they do
not call the FSM API from other tasks or ISRs. BareMetal uses its one
cooperative loop and bounded local delivery without RTOS queues or mutexes.
The owner coordinates other tasks through declared state/command routes but
does not control the scheduler or stop tasks synchronously. Safety-critical
inhibition remains an independent hardware/system path. The UEF-PL FSM adapter
and firmware emitter are not implemented yet.

The packaged `registry.json` and `schema.json` describe UEF-PL syntax, not UETD
components. `dispatch.py` maps the JSON's fixed dispatch IDs to allowlisted
implementation code. UETD-owned names and contracts are loaded separately from
the UETD registries. See [UEF-PL registries and discovery](../../../../../docs/guides/UEF_PL_REGISTRIES.md)
for the full source-of-truth map and contributor workflow.

A subsystem declares typed public ports, explicit aliased membership,
source-ordered stage calls, immutable typed `let` values, and explicit output
expressions. Stages call one member at a time with named arguments; membership
alone has no execution or wiring effect. A hybrid algorithm declares
input/output contracts, any number of algorithm members, guarded ordered stages,
typed output choices, failure behavior, and a bounded execution budget. Hybrid
selection is language behavior; UETD algorithms such as `enmpc` remain ordinary
UETD-native algorithms.

Models use explicit `derivative state.path = expression;` and
`next state.path = expression;` equations rather than inferred state-update
names. Matrix models declare state/input/output order. Checked casts and
multidimensional indexing/slicing preserve fixed shapes; a non-provably safe
array access is a validation error.

The language numeric spellings are `f32`, `f64`, `i8`/`i16`/`i32`/`i64`,
`u8`/`u16`/`u32`/`u64`, and `q15`/`q16`/`q31`/`q63`, plus `bool`. UETD's
API-linked type registry supplies the explicit C-name-to-DSL-name mapping and
all other discovered header declarations. The backend must preserve the
registry's visibility and readiness: ordinary C typedefs are not project
values unless an owning authoring contract permits them, and UMATH value
candidates require a compatible contract. The generated UEF-PL catalogue
lists every discovered type and its source header.

Protocol receive ports use the language-owned `Received<T>` envelope. `T` is
the logical record declared by the imported protocol; sequence and receiver
monotonic time are generated envelope metadata. This generic form is reserved
for transport values and is not a general user-defined type system.

`operatingsystem` owns its runtime modes, tasks, and resource declarations.
Tasks may be direct OS children. A taskset groups tasks only when common core,
scheduler, memory/channel, or resource ownership makes the group meaningful.

Expression precedence and FSM transition order are normative: indexing, `**`
power, unary, multiplication/division, addition/subtraction, comparison,
`and`, `or`;
short-circuit evaluation; and first matching FSM transition in source order.
Composition stage order is explicit and declaration/property order is otherwise
not a runtime order. The compiler builds a typed, source-located application
specification and does not execute the DSL as a general-purpose interpreter.
The checker and lowering produce the backend-neutral application
specification. The standalone firmware emitter has not yet been implemented.

The complete public UMATH function index is generated from UETD headers and
linked through `UETD/api.json`. Source calls resolve by each entry's UEF-PL
`dsl_name`; raw `umath_*` C symbols are hidden. Calls are valid only inside
custom algorithm and nonlinear model implementation bodies. Every function
must have both an implemented UETD module and reviewed DSL binding metadata
before generation; the header index alone does not grant call permission.
Operator spellings declared in a function's binding contract use the same
signature, scope, readiness, and failure checks as a named call. The `^` is
reserved for unit exponents such as `m/s^2`; it is not an arithmetic operator.

Every registered module identifier is resolved through the current UETD module
index's `DSL_descriptor` and linked manifest. Missing authoring metadata stays
visible and produces readiness warnings/errors. Adding modules never requires
adding an ID to this grammar registry or a Python parser switch. `map` and
`select` remain language-only bounded value forms.

Multi-target `.uefs` composition is parsed, validated, and lowered by the
system frontend. Its firmware emitter remains unavailable. The design is
described in
`../../../../../docs/roadmap/MULTI_TARGET_AND_MULTICORE_PLAN.md`; it references local `.uef` projects and
shared link contracts, not NEXUS graph nodes.

## Adding syntax

1. Update this `registry.json` with a production and construct description.
2. Add fixed parse/lowering dispatch IDs to `dispatch.py` only for actual DSL
   syntax. Registry JSON cannot import Python functions.
3. Extend parser, verifier, lowering, and stable source diagnostics in
   `../project/`; update application-spec dataclasses to preserve the
   authored contract.
4. Update VS Code TextMate syntax, snippets, language documentation, and
   examples. Dynamic UETD descriptors stay registry-resolved.

## Adding UETD-backed facts

Add targets/modules/runtimes, the canonical unit/dimension/conversion
catalogue, and typed math-operation overloads to UETD's own indexes/manifests.
Do not duplicate descriptor sets, module port facts, supported math names,
unit metadata, or readiness in uef-gen. Custom project models and algorithms
remain local and must not be added to the UETD global registry.

The registered `fpga` module is the MCU-side FPGA interface. The application
protocol describes which values are sent/received, when they are valid, and how
they are encoded. Link direction comes from the system contract; active messages
require explicit typed bindings and wire fields map explicitly to logical
record fields. The FPGA RTL and physical transceiver remain external target
work; no project protocol fields or handshake are invented by the language.
