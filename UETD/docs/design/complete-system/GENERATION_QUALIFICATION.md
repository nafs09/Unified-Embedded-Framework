# Generation, providers and qualification

This is first-pass design work. The existing frontend can parse, resolve,
analyze and lower projects; that is useful implemented work. Complete firmware
emission, exact-target builds, deployment and qualification are still separate
obligations. The [host capability catalogue](../../../registry/generation/README.md)
now records eleven first-pass pipeline obligations separately from embedded C
module identities. All remain design-only and generation-blocked; this design
and the first-pass scope table define their implementation requirements.

## Capability and provider resolution

Resolve support for an exact operation set, configuration and target tuple:

```text
operation contract
  + implemented C behavior
  + reviewed typed binding and failure mapping
  + implemented emitter route
  + complete transitive module/provider closure
  + exact board/runtime/numeric/resource compatibility
  + requested build/runtime/qualification evidence
  = supported requested capability
```

Store evidence independently for each term. Source existence, a metadata flag,
a ready binding and a successful archive build are different facts. A provider
profile may support only a subset of operations or dimensions. The resolver
returns selected implementations plus a structured explanation of missing or
conflicting predicates. It does not downgrade requested guarantees silently.

First-pass design records live in the main module or generation catalogue with
blocked readiness. `future/` is reserved for genuinely later requirements.
Accepted designs have priority, scope, implementation strategy, dependencies,
typed interfaces and acceptance gates. The design catalogue must distinguish
an unresolved profile from a fully bounded common contract.

Module `dependencies` contains only canonical module IDs. External providers,
licensed packages, optional implementations and alternatives have typed
requirements outside that list. Selected optional dependencies become real
dependency edges in the resolved plan. Prose such as “optional IP stack” cannot
be recursively visited as if it were a module ID.

Provider contracts identify implementation, version/hash, license, target/board
range, toolchain/ABI, runtime and numeric profiles; operation subset and allowed
contexts; configuration schema; memory/stack/work bounds; resource requirements;
external blobs/tools; startup/shutdown/recovery; and evidence references.
Credentials and private keys are runtime opaque references, never lockfile
contents. Provider alternatives must be distinguishable choices with compatible
interfaces, not duplicate aliases to an unsupported implementation.

The planned [contract schemas](../../../registry/contracts/README.md) give
machine-readable shapes for provider selection, driver transaction profiles,
budget reports and design metadata. Schemas validate structure. Cross-record
identity, dimensions, compatibility, trust and evidence require semantic checks.

## Language and typed lowering

Keep registry-driven descriptors and the existing parser/type/unit machinery.
Complete option/result/variant/flags/fixed-point and boundary codecs through
one path from syntax to typed IR, ABI layout and C emission. A language-visible
operation needs exact logical input/result types, units/shapes and C ABI modes.
Pointer direction, scratch, aliasing and failure mapping are reviewed metadata,
not guessed from a function name or `const` alone.

The IR contains component instances, immutable configurations, owned state,
bounded storage, operations, clocks, schedules, routes, effects, failures and
source locations. Public C handles/providers remain opaque unless a by-value
contract explicitly exposes them. Cross-node values use codec schemas and
validated wire bounds; they do not inherit host C layout.

Custom algorithms/models lower to bounded C with fixed arrays/workspace,
checked status propagation and declared external calls. No unrestricted
allocation, recursion, hidden I/O or unbounded loop enters a requested bounded
profile. A user-supplied C/C++ module has declared ABI, source/header closure,
operations/effects, resources, build flags and evidence. It is a supported
escape hatch, not a bypass around capability checks.

First-pass authoring covers static composition, tasks/cooperative activations,
single-owner state machines, settings, sensor/actuator connections, standard
protocol adapters, project-defined messages and explicit error/fallback routes.
Dynamic loading, general reflection and arbitrary runtime graph mutation are
later work. This keeps generated memory and scheduling inspectable.

## End-to-end pipeline and artifacts

| Stage | Inputs and output | Must reject |
|---|---|---|
| Discover | Versioned live catalogues and project overlays -> resolved symbol/config universe | Missing/ambiguous IDs, stale generated indexes and unsafe paths |
| Analyze | Source + contracts -> typed IR and diagnostics | Type/unit/shape errors, unsupported operations, invalid ownership/effects |
| Select | IR + exact target/provider profiles -> locked capability graph | Scaffold/absent providers, incompatible ABI, unresolved alternatives |
| Allocate | Graph -> pin/clock/IRQ/DMA/trigger/memory/partition assignment | Conflicts, unreachable memory, forbidden pins and missing target facts |
| Plan | Allocated graph -> initialization, execution, dataflow and recovery plans | Unbounded work, illegal context, unknown required cost, unsafe output policy |
| Emit | Plans -> source, configs, codecs, startup/vector/linker/build files and source maps | Missing emitter route, unsupported ABI lowering or missing complete header closure |
| Build | Locked inputs -> ELF/debug image, load segments, map and build log | Toolchain mismatch, unresolved symbols, warnings disallowed by the profile, range/size overflow |
| Verify | Artifacts + tests -> validation/evidence bundle | Failed contracts, wrong hashes/layout, missing requested target evidence |
| Package | Verified segments -> unsigned/signed release bundle | Inconsistent node/schema/provider versions or invalid compatibility/signature metadata |
| Deploy | Selected device + authorized operation -> verified result/progress record | Wrong device/board, unsafe ranges, unsupported rollback or partial system misreported as complete |
| Observe | Build-bound telemetry schema + device stream -> diagnostics and decoded data | Wrong schema/image identity, malformed/oversized records and hidden loss |

Generation writes into a staged directory and publishes atomically only after
internal checks succeed. A failed generation does not overwrite a last-known
good artifact tree. Generated files carry their input/contract/template hashes
and source-map locations. A stable generated/manual boundary avoids handwritten
edits being lost on regeneration.

The lock contains UETD revision/content hashes, registry and contract versions,
project/overlay hashes, selected providers, SDK/config and controller blobs,
toolchain packages, numeric/ABI choices, target/board revision and generated
artifacts. Rebuild compares the lock rather than downloading an unspecified
latest dependency. Offline packages have content and license checks.

Build output includes actual linked source/module closure, feature/provider
selection, memory map, stack/pool/queue allocations, partitions, initialization
order, IRQ/DMA ownership and execution-budget assumptions. Static libraries
must have consumer/link checks: building an archive can hide missing symbols
until a consumer calls the affected code.

## Scheduling and dataflow

Every activation declares period/event trigger, deadline, priority/context,
maximum input burst, WCET evidence class, blocking budget and output validity.
State whether the scheduling policy is cooperative, fixed-priority RTOS,
qualified time-triggered, or explicitly best-effort. ISR work and deferred
work appear separately in the plan.

The graph distinguishes same-tick data, one-tick delay, sample/hold, bounded
queue and latest-value mailbox edges. Reject an algebraic cycle unless it has
a declared solver with dimensions/convergence/work bound. A unit delay or
integrator must not be inserted automatically to make a cyclic graph compile.

For mixed rates, define producer/consumer clock domains, interpolation/decimation,
freshness and frame semantics. Control activation reads one coherent snapshot.
The generator emits copies/ownership transfer at the declared boundaries and
does not let producers mutate an activation's input halfway through a step.

Compute static utilization and, where assumptions apply, fixed-priority
response-time analysis including interrupt interference and bounded blocking.
Unknown execution costs are reported as unknown. A utilization sum below one
does not prove deadlines for arbitrary schedules. Network/storage/shell/UI
work executes outside hard control paths unless its exact bound is included.

Budget calculation uses checked integers and reports model assumptions:
static state, persistent buffers, maximum simultaneous scratch, stack estimates
with evidence, kernel/provider objects, DMA descriptors, network/crypto pools,
retained journals and update staging. Mutually exclusive scratch can share
only if lifetime/non-reentrancy is proven. Link-map checks compare actual
placement and totals to the plan. Stack estimates without static/dynamic
evidence do not become exact stack bounds.

Bus planning includes framing overhead, baud error/turnaround, CAN arbitration,
packet/header/retry and USB endpoint constraints as applicable. Average payload
bandwidth alone cannot establish worst-case command latency. Profiles report
burst/backpressure and contention assumptions and reject unsatisfied timing.

## Board and toolchain integrations

Complete a small multi-target reference set before broad claims. Candidate
profiles should exercise relevant differences such as Cortex-M4F/M7 cache/DMA,
an integrated wireless SDK, and later RISC-V or small-memory devices. Final
board selection follows capability needs and physical test access; naming a
family does not select a particular board or certify its support.

Use vendor SDK/CMSIS/startup/linker sources through explicit adapters where
they reduce duplicated low-level work. Import exact facts and generated config
with provenance, then review clocks/pins/errata/resource conflicts. SDK project
generation is not automatically equivalent to a UETD generated application.

Every toolchain profile specifies executable/version detection, supported host,
package/hash/license, target triple/ABI, compiler/linker flags, startup/runtime,
debug/program backend and build validation. Discovery is read-only. Build and
program actions use structured arguments and exact selected targets, not a
guessed connected board.

Install/export support needs a usable package config/version file and a
consumer compile/link test, plus headers and license closure. Generated
projects must also build from their declared source package without an
undeclared dependency on the developer's original checkout.

## Deployment and manufacturing

Separate direct probe programming, ROM bootloader programming and authenticated
field update profiles. Each declares device identity matching, permitted ranges,
erase/program/verify behavior, reset/boot sequence, recoverability and progress.
An automatic retry cannot overwrite a different device after reconnection.
Operation reports include the device/board identity, artifact hash, backend,
verification result and any incomplete stages.

Provisioning/manufacturing plans define serial identity, calibration, credentials,
test-fixture interfaces, required measurements, limits and pass/fail record.
Keep secrets in the selected provisioning provider. Irreversible fuse/debug-
lock actions are named final operations with exact reviewed parameters;
ordinary firmware generation does not perform them. Production tests report
measurement calibration and fixture/software versions.

Multi-node bundles bind image compatibility and deployment/confirmation order.
If one node fails, report the actual mixed-version state, keep local safe
actions active and apply the declared rollback/recovery plan. No general
distributed atomic update is assumed.

## Verification and evidence

| Level | Required evidence |
|---|---|
| Catalogue | Schemas, unique descriptors/IDs, resource paths, dependency DAG, transitive headers, current generated routes and first-pass/later placement |
| C/API | C11 and C++ self-containment, compile/link consumer, numeric/status/ownership behavior, profile ABI compatibility and meaningful negative cases |
| Portable implementation | Independent numerical/golden protocol references, property/fuzz tests, bounded resource exhaustion, failure/cancel and long-running wrap cases |
| Generated semantics | Direct API versus generated execution traces, units/layout/codec vectors, initialization/recovery order and failure propagation |
| Runtime/target | Pinned port ABI, timing/IRQ/clock/DMA/cache and watchdog tests on exact hardware, memory map and measured margins |
| Protocol/provider | Real peer/stack interoperability, malformed input, disconnect/retry, persistence/security and provider-resource bounds |
| Product | Composed safety/failure/recovery tests, power interruption, upgrade/rollback, manufacturing and field-diagnostic traceability |

An evidence record names exact source/artifact/provider/toolchain/config/board
hashes, operation subset, test conditions, measured limits, validity assumptions
and unresolved deviations. A change outside that scope invalidates or narrows
the claim; it cannot inherit a whole-board “verified” flag indefinitely.

Continuous checks regenerate indexes in a temporary tree or compare their
hashes, run catalogue audits/schema validation, build default and selected
numeric/runtime profiles, run real behavior tests as well as scaffold tests,
and compile a packaged consumer. Sanitizers/static analysis and hardware CI
are used where the toolchain permits; host passes do not qualify an MCU.

## Acceptance projects

These projects make completeness measurable. They are implementation acceptance
requirements, not claims that the current generator can emit them.

| Project | Required composition | Failure/exit gate |
|---|---|---|
| Minimal deterministic node | Exact board, startup/time/GPIO/UART, cooperative or static RTOS task, watchdog, generated state/config and diagnostics | Cold/warm boot, pin safety, timing, failed init, missing clock, watchdog and clean standalone build |
| Acquisition/control node | Synchronized ADC/timer/DMA, sensor quality/calibration, filter/PID, actuator limits and fault gate | Overrun/stale sample, full queue, bounded timing, trip, controller numeric failure and direct/generated trace equivalence |
| Battery data logger | Wake/sleep, timed sensor sample, calibration/settings, binary log, bounded storage and host decoder | Power loss at each commit, full/worn media, wake/time continuity, low supply and measured energy/latency |
| Connected product | Qualified Ethernet or WiFi provider, IP/DNS, TLS, MQTT or HTTP, provisioning/config, signed update and telemetry | Lost network, expired/untrusted identity, exhausted pools, interrupted update, failed trial and restored valid configuration |
| Myovena handheld | Approximately 7-inch OLED touch provider, graphics/UI, input, BLE and WiFi, product sessions/commands, power and secure maintenance | Display bandwidth/memory, simultaneous radio traffic, touch/flush races, stale/failed commands, reconnect, suspend and exact-panel tests |
| Industrial node | RS-485 Modbus or CANopen CC, typed process image, local I/O/actuator authority, watchdog and diagnostics | Invalid write/function, stale peer, bus fault, partial scan, local trip and real peer interoperability |
| Multi-node/AMP system | Separate owned images, typed generated link/shared channel, clock/freshness, compatibility bundle and local safe actions | Peer/core reset, stale handle, corrupt/lost message, mixed-version deployment and truthful partial-failure reporting |

At least two exact physical board profiles are required for the first target
qualification pass. Not every board must implement every acceptance project;
the matrix states which exact board/provider composition covers each capability.

## Genuinely later tooling

Remote fleet orchestration, graphical composition, dynamic graph loading,
general Linux/rootfs generation, comprehensive vendor-pack ingestion and formal
proof/assurance automation can follow the first-pass pipeline. The architecture
retains provider/source-map/evidence interfaces for them without making them
prerequisites for a usable embedded generator.
