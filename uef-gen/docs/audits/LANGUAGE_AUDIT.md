# UETD standalone language audit

**Scope:** .uef single-target projects, .uefs multi-target composition,
their grammar/type/operator contracts, component registration, and the AST and
lowered-application scaffolds.

**Verdict:** The language surface is expressive enough to describe a complete UETD application and a
multi-target system without a NEXUS graph. Its control flow, value model, and
resource bounds can be deterministic and safe under the rules below. It is
deliberately not a general-purpose interpreter. The current UETD registry is not
yet sufficient to close every type and call in the examples: canonical
units/dimensions/conversions and typed pure-math operation signatures are
missing. Those belong in UETD's registry/contracts, not in a second uef-gen
list. Generation is still scaffold-only.

**Implementation status:** The repository does not yet prove that claim at
runtime. The project-language parser, semantic validation, lowering, system
composition, generation, build, and flashing entry points still contain
scaffold/TODO paths. Examples and grammar describe the target design; they are
not evidence that check, generate, build, or flash already accept or emit a
correct application. The flight-controller project remains the source
equivalence case.

## Coverage audit

| Area | Language representation | Audit result |
| --- | --- | --- |
| Project and target | Exact target ID, numeric profile, project parameters, records, enums, constructor declarations, one operatingsystem, then implementation bodies | Sufficient. The order keeps the header-like declarations and OS outline readable. |
| UETD components | Role declarations plus any registered module's exact DSL_descriptor; settings, ports, operations, readiness, dependencies, and target support come from UETD | Correct ownership. Unknown names are errors; registered modules without authoring contracts remain visible but block generation. |
| Types and values | Fixed-width integers/floats, booleans, nominal enums, fixed arrays/matrices, records, compile-time strings, and Received<T> | The value model is sufficient, including compile-time symbolic dimensions and fixed arrays of received envelopes. Type checking is not registry-closed yet: UETD has no canonical unit catalogue, so unit dimensions/conversions cannot be authoritatively resolved from the current registry. |
| Operators | Explicit precedence for indexing/slices, right-associative power, unary, multiplicative, additive, comparison, and, or; short-circuit booleans | Deterministic. Slices are read-only; writes target one value/field/element. Remainder, shifts, bitwise, and named functions must resolve to typed UETD operations. The current registry does not publish typed pure-math overloads for example calls such as `abs`, `sin`, `cos`, `atan2`, `clamp`, and `zeros`. |
| Models | Linear matrices with state/input/output order; nonlinear continuous derivative and discrete next equations; output equations; optional Jacobians and explicit discretization | Sufficient in the language contract. Equation completeness, shape, dimensions, unit compatibility, and bounded solver work must be checked before generation. |
| Custom algorithms | Typed ports, state, parameters, locals, lifecycle functions, equations, conditionals, and compile-time bounded loops | Sufficient for user-authored bounded mathematics. No embedded C/Python, recursion, arbitrary calls, or data-dependent loops. |
| Subsystems | Aliased use membership, source-ordered stage calls with named arguments, typed immutable let values, explicit outputs | Sufficient and deterministic. Membership does not imply execution; there is no inferred graph. |
| Hybrid algorithms | Any number of registered/custom algorithm members, ordered stages, guarded stages, typed output selection, fallback/failure policy, execution budget | Sufficient for estimator fusion, controller cascades, ENMPC/SQP selection, and the finite-set MPC stage. |
| Protocols | Fixed message/fragment layouts, logical records, explicit field mappings, encoding/scaling, sequence/checksum/freshness/timeout/recovery contracts | Sufficient for deterministic MCU-side encoding and parsing. FPGA RTL remains external; fpga is only the registered MCU endpoint contract. |
| Runtime | Direct tasks, optional meaningful tasksets, state machines, interrupts, bounded channels/resources, priorities, rates, deadlines, and explicit failure behavior | Sufficient only when the selected target/runtime registry provides the scheduling, memory, synchronization, and safe-state facts the project relies on. |
| Multi-target .uefs | Independent target projects, shared records/enums/protocols, explicit directed links, endpoint contracts, delivery/recovery policy, external endpoints | Sufficient without remote calls or graph semantics. Each node remains an independent build. |
| Editor/tooling | Grammar highlighting plus live UETD-backed diagnostics/completion/hints | Structure is present. Syntax highlighting is static; semantic verification and registry-backed editor behavior still need implementation. |

## Registry data gaps found

- **Units:** No authoritative UETD unit/dimension/conversion registry was found.
  Manifests contain unit strings, but the compiler needs canonical base and
  derived dimensions, scale/offset rules, and allowed conversions. Add that
  data under UETD's registry and have `.uef` and `.uefs` resolve it there.
- **Pure mathematical operations:** `umath/core.json` describes source files
  and dependencies but does not expose typed DSL-callable overloads. Extend
  UETD's existing authoring contract/registry with names, overload signatures,
  units/shapes, purity, domain/range behavior, and target availability. Do not
  hard-code a parallel function list in uef-gen.
- **Generic protocol operations:** Custom protocol declarations need a
  registered generic runtime contract for encode/decode/send/receive, including
  argument/result types, buffer bounds, error behavior, and target support.
  Protocol configuration syntax alone cannot supply those runtime guarantees.
- **Registered-module completeness:** A `DSL_descriptor` makes a module
  discoverable, but a module still needs its typed authoring contract and
  generation route before it is callable or generatable. The editor warning /
  check error distinction is sound; the compiler must not silently lower an
  incomplete contract.

## Determinism and safety rules

1. **No name guessing.** Names resolve exactly and case-sensitively to project
   declarations, an imported system contract, or a UETD registry contract.
   Unknown properties and missing required settings are diagnostics.
2. **No hidden control flow.** A task has one declared callable invocation.
   Subsystem/hybrid calls run in source order; locals are immutable and
   evaluated once. FSM transitions use one snapshot, first-match order, and at
   most one transition per runtime boundary.
3. **No hidden conversion.** Numeric width, signedness, units, dimensions,
   rounding, overflow, non-finite behavior, matrix layout, and FMA behavior
   have explicit contracts. Casts are checked; enums stay nominal.
4. **No implicit model dynamics.** Every state has one explicit derivative or
   next-state equation; every output is assigned. Linear matrix order is
   explicit. Continuous discretization names a UETD-registered method and
   period.
5. **No guessed wire ABI.** Every message has bounded layout and a logical
   payload contract or is explicitly raw. The checker rejects overlapping or
   out-of-bounds fields, unmarked gaps, duplicate IDs, lossy conversions, and
   ambiguous checksum ranges. Received payload and receiver-local sequence,
   time, and freshness metadata are distinct.
6. **No inferred scheduler.** Source order is not task priority. Runtime
   contracts define release, preemption/tie-breaking, ISR behavior, queue
   ownership, atomic publication, memory ordering, timer precision, and miss
   handling. Missing facts block generation.
7. **Bounded storage/work.** Arrays, queues, buffers, loop counts, and work
   budgets are static. Cross-core/external data uses a typed bounded channel
   or protocol endpoint with ownership, age, and synchronization facts.
8. **Reproducible output.** Same source, exact input values, registry/template
   digests, generator version, and target produce the same typed specification
   and generated artifacts.

## Language and scaffold changes made in this audit

- Added typed immutable let locals for composition expressions that are not
  component calls. They cannot mutate or invoke members.
- Made model updates explicit with derivative state.path = ...; and
  next state.path = ...;. A model cannot depend on an inferred _dot name.
- Added explicit state/input/output ordering for matrix-based models and
  defined elementwise unit derivation against those ordered bases.
- Extended expression grammar and AST coverage for dotted registered math
  functions, explicit checked casts, and multidimensional indices/slices.
- Corrected lifecycle-parameter grammar, symbolic array dimensions and arrays
  of `Received<T>`, integer-only unit exponents, and the stale hybrid-protocol
  `bind` production. Added explicit range-token priority and restricted
  assignment targets to remove slice-copy ambiguity.
- Expanded registry semantics for type checking, model completeness, protocol
  layout validation, task/runtime determinism, and fixed memory/work.
- Made primitive types, operator precedence/contracts, and lexical token rules
  explicit registry data, then updated the strict language-registry schema and
  loader so those rules are not discarded or rejected.
- Added missing parser/lowering IDs to the dispatch allowlist. Before that, the
  registry named valid constructs that the loader itself would reject.
- Added a strict `.uefs` registry schema and made its type/operator contract
  explicitly reuse the `.uef` core rules rather than create duplicate facts.
- Added AST/application-spec fields to preserve composition locals and output
  expressions instead of dropping them during future lowering.
- Updated the drone, optical bench, ROV, Tesla H745/G4 examples where
  subsystem dataflow had depended on implicit member bindings; the remaining
  H745 capture/plant algorithms are intentionally incomplete engineering
  inputs and must fail a ready-to-generate check until authored.
- Updated .uefs record syntax and static UETD syntax highlighting for the
  current grammar.

## Required implementation gates

The design is ready for backend work, not a claim that backend work is done.
The following gates are required before v1 can be called operational:

1. Add runtime schema validation for both registries, then implement
   grammar/AST parsing and source recovery for every production in
   src/uef_gen/language/registry/registry.json and
   src/uef_gen/language/frontends/system/registry.json, including typed task calls,
   stages, locals, output expressions, model updates, protocol fields, and
   system contracts.
2. Implement one shared type/unit/shape/range checker used by CLI commands,
   editor hints, and generation. It must consume UETD operation/component
   signatures rather than contain a second list of modules or functions.
3. Complete UETD authoring contracts for callable operations, property schemas,
   typed ports, failure behavior, generation routes, target support, a
   canonical unit/dimension/conversion catalogue, pure-math overloads, and all
   runtime facts used by the examples. A module with incomplete metadata can
   receive an editor warning but cannot be generated.
4. Validate runtime schedule/resource/memory/channel facts against the exact
   target and selected runtime, including multicore cache/barrier/clock policy.
5. Validate and lower exact protocol layout plus sender/receiver endpoint
   direction; encode/decode by protocol contracts, never C struct layout.
6. Lower all declarations losslessly into the standalone application
   specification, preserve source spans, and reject any unsupported
   construct/property instead of ignoring it.
7. Connect the typed spec to UETD templates, the existing build manager, and the
   registered flash route. Build/flash must stop before side effects when
   validation or artifact/target identity checks fail.
8. Exercise the primary flight-controller source-equivalence case and
   multi-target Tesla links after those implementation gates exist. The Tesla
   example still needs the real plant model, capture reconstruction equations,
   measured limits, timing, and board values before it can be generated as a
   complete coil system.

This audit does not expand v1 into wireless debug/flash or a NEXUS graph
editor. Wired CLI diagnostics and the MCU-side FPGA communications contract
remain within the stated standalone scope.
