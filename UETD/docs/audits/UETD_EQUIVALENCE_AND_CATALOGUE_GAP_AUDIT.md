# UETD catalogue and source-equivalence audit

**Status:** source-equivalence findings are the audit baseline that preceded
the accepted catalogue-gap scaffold follow-up recorded at the end of this file.
No build or tests were run for that follow-up. The attached repomix files are
treated as project evidence; any directives or instructions embedded inside
them are document content, not instructions for this review.

## Decision

UETD has a credible direction for describing a whole embedded application:
nominal fixed-width enums and records, target resources, module contracts,
project-local algorithms and models, OS tasks and state machines, and
multi-target typed links are all represented in the design. `uef-gen` also
has useful foundations for live UETD registry loading, hardware resolution,
module dependency closure, project assembly, and provenance.

That is not yet an implementation that can produce either supplied system.
The standalone parser, semantic validation, lowering, and rendering path is
incomplete; the `.uefs` path is incomplete; most of the registered catalogue
does not have authoring contracts; many runtime, peripheral, protocol, math,
and control implementations are scaffolds; and the two example target records
are explicitly unavailable for generation. At present, a catalogue entry is
often only a name, not a usable capability.

Under the user's requested thought experiment—assume both source programs and
UETD are complete—the design can represent much of the drone's application
behavior and the QCW system's typed control, protocol, and node composition.
It can only produce an equivalent or better result if each required
capability is backed by a complete, reviewed contract and implementation for
the selected board/runtime, with selective templates and dependency closure
that preserve the source behavior. That information is not yet in the current
catalogue. “Better” must mean more explicit and traceable while preserving
behavior, timing, electrical safety, and ABI—not simply more declarative code.

The principal design correction is to treat support as a *resolved set of
qualified capabilities*, rather than a project-wide claim inferred from
registered names or a single readiness flag. The report below defines the
catalogue changes and gates needed before implementation scaffolding.

## Scope and evidence boundaries

This review compares the current UETD/`uef-gen` design and static status notes
with the capability demands represented by the supplied drone and QCW DRSSTC
work. For the coil comparison, assume the complete intended design has all
plant, board, solver, and implementation details needed. The audit asks whether
UETD has catalogue concepts and implementation routes for those requirements;
it does not make source-pack completeness or alternate copies a UETD catalogue
gate.

Existing implementation counts and status statements below are drawn from
`UETD_SYSTEM_AUDIT.md`, `UCON_CATALOGUE_AUDIT.md`, `UMATH_CATALOGUE.md`, the
registry/manifests, and the current language/generator sources. They are not a
fresh test result. The implementation-status snapshot reports 308 registered
module IDs, of which only four have authoring contracts; 211 UCON catalogue
keys with only PID and lead-lag registered implementations; and 34 UMATH
modules, of which 11 are selectable while 23 remain scaffolds. Counts are
useful inventory signals, not measures of application coverage.

## What “equivalent” must mean

An equivalence claim needs a requirements-to-artifacts trace for the selected
source revision and target configuration. Each required source behavior must
map to one or more of:

1. a UETD-owned portable implementation and its typed authoring contract;
2. a target/board-bound driver or runtime capability with verified resources;
3. bounded project-owned application code emitted from a typed `.uef` model;
4. a pinned external implementation with a typed interface, provenance, build
   instructions, and explicit qualification ownership; or
5. a clearly reported unsupported item that blocks a complete-equivalence
   result.

The generated project must preserve externally observable behavior: state and
fault transitions, sensor units/calibration/freshness, actuator encoding and
safe states, task and interrupt context, timing budgets, startup/shutdown,
watchdog policy, message ABI, core/node ownership, and hardware resource
assignments. Output also needs to be inspectable: generated files identify
their UETD contract/template/source inputs, and users can trace an emitted
operation back to a source declaration and selected implementation.

UETD can make an implementation more explicit than the source by checking
enum domains, message layouts, units/shapes, dependency closure, resource
conflicts, bounded work, and missing safety policies. It must not claim a
behavioral improvement when it has only replaced a concrete driver or model
with a placeholder.

## Current blockers by design area

| Area | Present foundation | Hole that prevents complete equivalence |
|---|---|---|
| Catalogue identity and readiness | UETD owns module, target, runtime, and control registries. Authoring contracts can declare typed parameters/ports, numerical constraints, scheduling/composition, operations, and generation routes. | Registry presence is commonly mistaken for usable support. Most modules lack authoring contracts; a `generation.status` of draft/ready/unsupported does not separately prove implementation, selected target backend, dependencies, generated route, or qualification. The readiness model needs independent evidence axes and a derived per-project result. |
| Standalone language | `.uef` design includes nominal enums, records, fixed arrays, typed module roles, project algorithms/models/protocols, OS declarations, tasks, tasksets, channels, and state machines. | Parser, validation, lowering, and standalone generation remain scaffolds. Enum syntax is designed but not executable today. A construct in the grammar/example is not proof that the emitter or runtime implements it. |
| System language | `.uefs` design composes independently built nodes and typed links, retaining local safety authority. | Parser, resolver, contract validation, system build planning, and per-node artifact generation remain scaffolds. Cross-node and H745 M7/M4 lifecycle facts need target-owned records. |
| Selective templates | `uef-gen` has module selection and transitive dependency closure for its current input path; templates are UETD-owned data/routes. | The standalone application spec does not reach the current `generate_project()` pipeline. Templates do not yet render the complete `.uef` application/OS description. A selected module can be rejected when scaffold-only, but selecting a readiness-complete module set is not yet tied to the standalone compiler. |
| Target and board data | Exact-chip and development-board registries, provenance fields, and fail-closed generation flags exist. | Chip records still expose broad peripheral groups where exact package pins, alternate functions, clock routes, DMA/DMAMUX requests, IRQ facts, memory ownership, linker/startup, errata, and board circuits are required. STM32F722RET6, STM32H745XIH6, and STM32G474VET6 support must be judged from exact board profiles, not family names; the H745 and G474 records cited by the coil plan are `generation_available: false`. |
| Peripheral/driver layer | UHAL/UPAL APIs and UETD module source/header manifests exist; future contracts detail UART framing/half-duplex, capture publication, synchronized ADC/PWM, USB profiles and Ethernet MAC/PHY ownership. | These added mode contracts are design-only and target operations remain target-dependent. Promotion requires typed APIs, target capability mapping, resource conflict rules and backend evidence; sensor-specific drivers remain user-authored. |
| Runtime/OS and safety | Bare-metal and FreeRTOS surfaces, OS/task/state-machine language design, periodic status/staleness supervisor, health-lease watchdog gate, bounded recoverable-fault queue, and watchdog descriptors exist. | Target startup/scheduling generation and watchdog hardware qualification remain separate work. The gate controls only feeds routed through it; retained crash context, update recovery, external hardware interlocks and capability-linked evidence remain future services. |
| Types, units, and ABI | `.uef`/`.uefs` designs define nominal fixed-width enums and records; project protocols map fields explicitly instead of relying on C struct layout. Future contracts specify `Option`/`Result`, tagged variants, typed flags, fixed point, bounded text/bytes, shared-memory ABI, and persistent codecs. | Those types are design-only until parser, typechecker, lowering, generated representation and both language frontends support them. Existing enum/record support does not define a cross-core ABI or persistent schema evolution. |
| Math/control | UMATH owns common numerical APIs; UCON contains a large named algorithm catalogue; PID and lead-lag are the implemented registered algorithms. | Catalogue breadth greatly exceeds working implementations. Key drone estimation/filter/mixer and coil estimator/optimizer/plant operations depend on missing algorithms and UMATH foundations. Typed state/configuration, dimensions/units, failure results, memory/work bounds, numerical policy, target support, and validation evidence must be complete per algorithm. |
| Protocols and external components | UPROTO has a working PPM decoder and named CRSF/SBUS/MAVLink/DSHOT/UAVCAN surfaces; custom project protocol and external endpoint concepts exist. | Most protocol cores are fail-closed or scaffolds. Transport-independent framing vs UART-bound APIs and ISR/task ownership are unresolved. For external FPGA, UETD currently specifies the MCU endpoint contract; it does not generate or qualify FPGA RTL, transceiver/GTP, timing constraints, or electrical path. |
| Build, package, and deployment | `uef-gen` has staged project assembly, build/deploy abstractions, manifest/provenance models, dependency selection, and target discovery surfaces. | Standalone build/flash is not an operational end-to-end route for the examples; exact compiler/startup/linker profiles and target deployment profiles are incomplete. External source and generated outputs need pinned package/version/license/hash handling. Partial system builds must never be presented as complete. |
| Qualification | Existing docs identify host/hardware test areas and target-specific concerns. | No current test tree is reported in the static audit, and neither example has generated-vs-reference conformance evidence. Catalogue `ready` must not substitute for compile, numerical, resource, timing, fault-injection, protocol-interoperability, and hardware evidence at the claimed assurance level. |

### Catalogue readiness model to adopt

Keep `generation_available` as a fail-closed, derived gate for one exact
configuration. Add explicit evidence for these independent stages to the UETD
contracts/manifests:

- **Authoring contract:** parameter/port/state/operation schemas complete and
  reviewed, with units, shapes, ranges, contexts, and error results.
- **Implementation:** actual source/template or pinned external package exists;
  it is built, versioned, and license/provenance checked.
- **Target binding:** exact part and board satisfy the feature's resource,
  memory, clock, interrupt, DMA, startup, and runtime requirements.
- **Generation route:** allowlisted renderer supports this construct and
  emits the expected source/artifacts; route version is recorded.
- **Verification:** evidence level and results are recorded for contract,
  host behavior, target build, timing/resource budget, HIL, and hardware as
  applicable.
- **Configuration resolution:** all project-required values and capability
  dependencies resolve with no invented defaults.

The compiler should report each axis and the blocking diagnostic. It should
not promote a module because its name is registered, source files exist, an
example mentions it, or a generic target family advertises a peripheral.

## Cross-domain catalogue coverage beyond the examples

The two systems are stress cases, not the definition of an embedded framework.
UETD also needs reusable capability contracts for ordinary bare-metal and RTOS
products such as instruments, industrial controllers, robots, connected
devices, motor drives, and sensor gateways. The inventory below identifies
requirements that must be represented even when neither example exercises
them. Each can be optional per product and target; omission must be explicit
and validated rather than silently assumed.

| Embedded concern | Catalogue capability needed | Current design/repository gap |
|---|---|---|
| Reset, startup and shutdown | Reset-cause capture; boot phases; clock and memory init dependencies; constructors policy; peripheral init ordering; safe output defaults before RTOS; orderly shutdown and brownout behavior; per-core start/release; linker/boot image contracts. | Target profiles are not complete for exact boards, and standalone lowering does not generate the lifecycle. A runnable `main()` or RTOS source tree is not enough to establish startup correctness. |
| Scheduling and concurrency | Bare-metal superloop, tick/tickless RTOS, ISR/deferred-work boundaries, periodic/event tasks, deadlines, WCET/stack/queue budgets, resource locks, affinity, atomicity and delivery policies. | Language concepts exist, but parser/lowering and qualified runtime profiles are missing. Generic SPSC or single-core critical sections cannot be treated as multicore/ISR-safe by default. |
| Peripheral portfolio | GPIO/EXTI; UART/USART; SPI/I2C; timer/PWM/HRTIM/capture/encoder; ADC/DAC/comparator; DMA; CAN/CAN-FD/LIN; USB/Ethernet; storage/flash/RTC; watchdog and reset. | Hardware group names do not capture exact instances, modes, routing or drivers. UPAL sources remain broadly scaffolded. Availability must be recorded per part, package, board and operating mode. |
| Configuration and persistence | Typed defaults and ranges; schema/version migration; calibration storage; atomic commit and power-loss behavior; wear limits; manufacturing identity and bounded NVM access. | A project `parameter` is not a persistent-data contract. Logging/storage adapters and target NVM behavior are incomplete; no general migration and crash-consistency contract is established. |
| Communications and connectivity | Typed framing, transport-independent codecs, discovery/provisioning where used, retries/timeouts, flow control, security hooks, bus-off/recovery, network stacks and version compatibility. | Some protocol names exist, but implementations, typed authoring contracts, transport policy and qualification are incomplete. Wireless, IP/network and secure provisioning support cannot be inferred from those names. |
| Safety, health and diagnostics | Independent watchdogs; health leases; reset/fault record; boot self-check; degraded-mode policy; safe outputs; bounded event/log buffers; counters/trace; diagnostic transport; service mode. | UAPP fault reporting is incomplete, watchdog semantics are not fully bound to hardware/runtime profiles, and most fault/supervisor concepts lack end-to-end generated enforcement and fault-injection evidence. |
| Security and field maintenance | Optional secure boot, signed image/update, rollback policy, trust/key storage, debug lock, authenticated maintenance, image compatibility and recovery mode. | These are not established by generic build/flash abstractions. The catalogue needs explicit security capabilities and external crypto/boot dependencies where these are product requirements; targets lacking them must be marked unsupported. |
| Memory, power and device lifecycle | Static allocation budgets, stack/heap policy, DMA buffers, MPU/cache setup, sleep/wake and clock transitions, peripheral ownership, hot-plug/reinitialization when applicable. | Memory/core/cache facts and lifecycle operations are not comprehensively modeled in target records or generated runtime behavior. No dynamic allocation policy should be inferred from bounded-looking application syntax. |
| Validation and product evidence | Simulated peripheral contracts; host substitutes; fault injection; protocol vectors; static resource map; WCET/stack/size reports; HIL and hardware qualification; reproducible artifact provenance. | Current system audit reports no tests tree and no example-level generated conformance. Existing audit data says static checks only; they do not qualify outputs. |

These concerns should not become a giant mandatory runtime. Keep portable
contracts small, add target/runtime implementations only for selected
capabilities, and let the resolver reject combinations whose full dependency
and evidence chain is absent. Security and network services, for example,
should be enabled only where the product needs them, but the generator should
not imply those guarantees when they are omitted.

## Example compatibility matrix

| Capability | Drone source demand | QCW DRSSTC source demand | UETD/`uef-gen` assessment |
|---|---|---|---|
| Language and values | Flight/pilot modes, arming/failsafe/landing, configuration, sensor/control values. | Controller/executor modes, fault codes, node IDs, typed setpoints and measurements. | Nominal enums/records are a good fit on paper. Enum parsing/typechecking/lowering is unfinished, so neither example currently generates these semantics. |
| Full embedded OS | Sensor acquisition, receiver, estimator/control loops, telemetry, watchdog/supervision, startup and motor-safe lifecycle. | H745 M4 capture/phase work, M7 estimation/planning, G4 local timer/protection work, per-core/node channels, local state machines, watchdogs and fault paths. | Task/taskset/interrupt/channel/root design exists, but frontend, target runtime and generated scheduling/startup are incomplete. Need OS operations and hardware resource ownership as explicit capabilities. |
| Sensors and peripherals | SPI IMU, barometer, I2C magnetometer, GNSS and receiver UARTs, ADC inputs, timer/PWM/DShot motor output, USB telemetry, GPIO/EXTI. | Synchronized ADC/sensor sampling, high-rate timer input capture and output staging, DMA/IRQ, fibre endpoint transport, shared memory/cache/barriers. | Current target records and driver catalogue do not provide enough exact resources or working drivers. The current drone sketch models a single timer motor resource, while the supplied implementation used four half-duplex UART DShot paths; that is a concrete topology mismatch to resolve in the target/source mapping. |
| Math and control | Attitude/navigation estimation, filtering/notches, cascaded rate/velocity/position control, motor mixing and camera-mode control. | Phase reconstruction, state/covariance estimation, explicit energy policy or online NMPC-SQP, finite-set switching-state MPC, plant propagation and objective/constraints. | PID/lead-lag alone are insufficient. The catalog contains many names, but the needed estimator/filter/MPC/Jacobian/ODE/spectral functions are mostly absent or scaffolds. Each algorithm needs typed fixed memory/state, failure behavior and target budget. |
| Protocols and data validity | CRSF receiver, DShot actuator link, MAVLink telemetry, possibly USB framing. | Custom framed transport, sequence/integrity rules, atomic fragmented-setpoint commit, freshness, faults/telemetry/configuration, dual clock domains and external FPGA endpoint. | Project-local protocol DSL is suitable conceptually, but codec generation is unfinished and established protocols are not complete. |
| Safety/watchdogs | Arming interlocks, loss-of-receiver failsafe, motor kill, sensor freshness, supervisor, independent watchdog/reset paths. | Hardware inhibit, local executor authority, stale command handling, interlocks/desaturation/thermal limits, FPGA command watchdog, H745/G4 watchdogs, fault latching and rearm. | Safety can be expressed in OS FSM/supervisor constructs but is not enforceable from current catalogue declarations alone. Need concrete independent watchdog and actuator inhibit contracts, reset cause/fault persistence, and fault-injection verification. |
| Multi-target/external logic | One MCU target in this source snapshot. | Dual-core H745, distinct G4 MCU, two FPGA endpoints. | `.uefs` model is structurally aligned, but not executable; H745 startup/cache/link facts are missing; no FPGA RTL backend. A complete generated deliverable needs declared external HDL artifacts or explicitly bounded non-generation scope. |
| Build and evidence | Target-specific startup, linker, compiler flags, binary, resource map and bench evidence. | Separate core/node images, link endpoints, FPGA outputs, protocol/CDC/timing/electrical qualification. | Current target records are blocked from generation; no end-to-end standalone build/flash proves parity. |

### Drone-specific compatibility findings

The drone example is a useful single-target acceptance case because it combines
continuous sensor input, discrete receiver modes, cascaded estimation/control,
multiple update rates, actuator encoding, telemetry, and safety behavior. The
language has enough *designed constructs* to describe much of this at the
application level, but its current example is not a complete behavioral
translation:

- `PilotMode` and `FlightMode` demonstrate useful nominal enums, but the source
  state/type inventory must be reconciled with the source (including arm state
  and takeoff behavior); omitted states cannot be repaired by parser support.
- Its peripheral declarations are symbolic. The source's exact bus/pin/IRQ/DMA
  bindings, driver lifecycle, device identity/calibration, and safe startup are
  not supplied by a usable F722 board/driver profile.
- The DShot execution path differs: source uses four half-duplex UARTs; the
  sketch declares one `motor_output_timer`. An equivalent generated design
  needs per-motor channel/bus topology and an explicit supported encoding and
  half-duplex turnaround contract, or a reviewed behavior-preserving migration.
- The current catalogue does not supply the full DShot, CRSF, MAVLink, sensor,
  estimator, notch/filter, allocation/mixing, and watchdog implementations
  needed to turn the sketch into a complete image.
- Startup calibration/readiness sequencing, sensor age/degradation policy,
  receiver failsafe details, watchdog feed ownership, and motor-output inhibit
  must be traced from source into generated runtime behavior, not summarized by
  a generic `supervisor` declaration.

Thus this is a suitable equivalence gate once source facts and the exact board
are frozen. It should require generated-source comparison at module and
state-transition boundaries, plus bus/resource/timing checks.

### QCW DRSSTC-specific compatibility findings

The coil design adds capabilities not exercised by the drone: complex numeric
optimization, multiple execution cores and chips, asynchronous links, remote
clock domains, FPGA/serial endpoints, fail-safe local authority, and a strict
cycle/validity protocol.

- Treat the complete intended system as requiring one or more multicore MCU
  nodes, a separate local executor/safety node, and FPGA endpoints. UETD's
  capability model must express per-core execution and per-node builds,
  independently owned clocks/memory/resources, typed links, local safe-state
  authority, and externally implemented hardware endpoints. The exact topology
  is a project input; this capability coverage is the catalogue requirement.
- The intended implementation requires a plant model, constraints/objective,
  explicit policy data, online solver, thermal model, MCU HAL/LL, bounded
  transport codec, and FPGA implementation/constraints. Under the requested
  complete-system assumption these are real inputs/implementations that the
  catalogue must accept, typecheck, select, bind, and report—not unresolved
  holes in the source pack.
- UETD's custom algorithm/model and hybrid constructs can describe a controller
  boundary, but the catalogue does not currently implement the needed UKF,
  phase estimator, ENMPC, RTI-SQP, finite-set MPC, and numerical foundations.
  Plant dimension, units, solver work/memory budgets, candidate feasibility,
  numeric failure and deadline fallback need typed contracts.
- Cross-core samples and cross-node messages need distinct delivery semantics:
  latest-value cycle-tagged mailboxes may coalesce control requests; faults
  and measurement events may require FIFO. Clock timestamps must remain
  receiver-local unless a synchronization model proves comparability. Cache
  maintenance and barriers are core/target operations, not generic channel
  decorations that a backend may ignore.
- UETD's `external_rtl` boundary intentionally leaves FPGA logic outside the
  MCU generator. For system-level equivalence, require a manifest pinning the
  RTL source/revision, FPGA part/toolchain, constraints, interface checks,
  generated bitstream identity and qualification owner—or explicitly label
  FPGA artifacts as separately supplied and unqualified.
- Safety is distributed: FPGA output gate, local G4 executor, H745 controller,
  inter-core supervision and hardware interlocks each have distinct authority
  and watchdog/reset behavior. One software supervisor or a generic watchdog
  module cannot stand in for them.

## Catalogue and repository design updates required

### 1. Make capability contracts executable evidence

For each selectable module, define a reviewed contract for configuration,
inputs/outputs, units and dimensions, enum validity, errors, state ownership,
memory bounds, numerical mode, execution context, timing/work bounds,
dependencies, target constraints, and safe failure behavior. Keep declarative
metadata separate from algorithm implementation and per-target driver source.
An entry is not selectable until both the semantic contract and codegen route
exist and their target-qualified implementation is available.

The registry should express capability dependencies (for example, a DShot
receiver/timer/DMA feature requires a specific timer-channel mode and a
supported half-duplex turnaround path), allowed substitutions, and mutually
exclusive resource ownership. Validation should report the missing capability
chain rather than only “module unknown/not ready.”

### 2. Define canonical target-resource and board contracts

Replace broad group-only facts for generation-ready targets with verified,
source-cited records for exact package pins and alternate functions, peripheral
instances/channels, clock-tree limits and derived rates, DMA/DMAMUX routes,
IRQ numbers/priorities/affinity, core-visible memory/cache attributes,
linker/startup/reset ownership, boot order, errata constraints and reserved
board nets. Board profiles must bind schematic roles to those resources and
include transceiver, gate-driver, inhibit, sensor and actuator polarity/safe
states where those affect software behavior.

Driver contracts should include init/deinit ordering; ISR vs task APIs; bounded
work and buffers; DMA ownership/cache rules; timeouts/cancellation; overflow
and bus recovery; calibration/unit transforms; sample timestamps and age; and
propagated errors. Generate resource claims so the resolver can detect pin,
timer, DMA, IRQ, memory and core conflicts before source emission.

### 3. Treat OS behavior as first-class generated design

Represent startup phases and dependencies, target/core ownership, task and ISR
triggers, periods/deadlines/WCET budgets, stack and queue capacities, affinity,
resource locks, message delivery semantics, missed-deadline policy, fault
reporting, controlled shutdown, reset and recovery. Add a concrete watchdog
contract covering hardware watchdogs, software health leases, boot grace,
feeding authority, window restrictions, expiry action, reset-cause capture and
rearm policy. Require an independently enforced safe actuator state for each
physical output owner.

The generated entry points and peripheral initialization should remain
readable and traceable. The project source should not have to hand-author
vendor startup/ISR code to fill a generator omission; handwritten portable UETD
driver source is a supported implementation route, but its contract and
target-binding evidence must be registered in UETD.

### 4. Connect selective templating to the standalone application IR

The intended `uef-gen` path should be:

1. Parse `.uef`/`.uefs` with source spans and recoverable diagnostics.
2. Typecheck against one pinned UETD catalogue snapshot, resolving nominal
   types, units, module ports, calls, protocol fields, and OS semantics.
3. Resolve exact target, board, runtime, core, pins, clocks, DMA, IRQ, memory,
   and dependencies into an immutable project/system IR.
4. Build a capability and module graph from actual project references; select
   only required UETD implementation units/templates and their transitive
   dependencies. Exclude unrelated catalogue entries.
5. Emit bounded project-owned algorithm/model code and thin module/driver
   configuration wrappers; reuse UETD implementations rather than cloning
   algorithms into project output.
6. Render startup, runtime, resource maps, protocol codecs, application entry
   points, build inputs and per-node/core artifacts through allowlisted
   UETD-owned routes.
7. Validate generated interfaces, resource use, source closure, bounds, build
   plan and provenance before publishing any output.

Separate project input, generated output, vendored UETD source, external
dependencies, and manual actions in the output tree and manifest. Make output
deterministic for identical source/catalogue/toolchain inputs. A component with
no renderable route, missing dependency, external pin, or manual prerequisite
must be an explicit blocker; no generic placeholder may be reported as a
completed equivalent implementation.

### 5. Cover embedded concerns beyond the two examples selectively

The catalogue should be broad enough to describe common embedded operating
systems and peripherals, but support claims must remain per target. The
capability inventory needs reviewed families for GPIO/EXTI, timer/PWM/HRTIM,
capture/encoder, ADC/DAC/comparators, UART/SPI/I2C, CAN/CAN-FD, USB, Ethernet,
storage/flash, DMA/cache/MPU, clocks/RTC, watchdog/reset, bootloader/update,
diagnostics, and common device buses. More specialized targets can add FPGA,
DSP, RISC-V, safety island, wireless, or secure boot capabilities without
pretending every backend supports all features.

Each feature needs resource semantics, error and lifecycle behavior, relevant
timing/security constraints, and a selection rule. A generic driver interface
should preserve meaningful differences between targets rather than flattening
them into an API that cannot express a target's DMA, trigger, power or safety
behavior.

## Design and implementation gates

Do these in order before scaffolding either full acceptance project:

1. **Freeze acceptance requirements.** Record source revisions and
   requirements-to-behavior mapping for the drone and complete QCW system.
   Resolve project-specific values such as topology, wire fields and external
   artifact ownership when preparing a real generation request; keep those
   project inputs separate from reusable catalogue capabilities.
2. **Create trace matrices.** For each source behavior, record source file/
   function/state, UETD contract/module, project declaration, generated
   artifact, target resource, timing/safety requirement and verification
   evidence. Mark source-only items and UETD-only assumptions explicitly.
3. **Close catalogue contracts.** Prioritize one vertical slice at a time:
   basic startup/GPIO/UART/ADC/timer/interrupt/DMA/watchdog; sensor and
   telemetry protocols; enum/record runtime support; baseline math/control;
   then multicore and custom FPGA-linked system. Add no selectable stub.
4. **Resolve target profiles.** Establish board-specific F722, H745 and G474
   profiles only from exact schematics, vendor manuals/datasheets/errata,
   toolchain and startup data. Keep generation blocked until every used
   resource/profile field has a source and verification state.
5. **Implement the DSL-to-generator seam.** Complete the standalone frontend
   and lower to a stable private application/system IR. Then connect that IR to
   UETD module planning, target resolution, and route selection.
6. **Verify fidelity.** Require parser/type diagnostics; golden generated
   source/manifest; dependency and output determinism; host unit/numerical and
   boundary checks; target compile/link/map/stack/size checks; protocol
   vectors and interoperability; timing/WCET evidence; fault injection and
   HIL/hardware safe-state tests. Match assurance level to intended deployment.
7. **Publish qualified support.** Declare support only for an exact source,
   board, core/runtime, module set, toolchain and external-artifact revision.
   State remaining assumptions and per-node/per-core status in manifests.

## Related repository documents

- [System implementation snapshot](UETD_SYSTEM_AUDIT.md)
- [UCON catalogue details](UCON_CATALOGUE_AUDIT.md)
- [UMATH catalogue](../catalogues/UMATH_CATALOGUE.md)
- [Standalone language design](../../../docs/design/PROJECT_LANGUAGE_DESIGN.md)
- [Multi-target and multicore plan](../../../docs/roadmap/MULTI_TARGET_AND_MULTICORE_PLAN.md)
- [Repository implementation plan](../../../docs/roadmap/IMPLEMENTATION_PLAN.md)
- [uef-gen implementation TODO](../../../uef-gen/docs/TODO.md)

## Evidence anchors in the repository

- `uef-gen/src/uef_gen/language/frontends/project/parser.py`,
  `validation.py`, `lowering.py`, and `compiler.py` define the standalone
  frontend boundary; parser/validator/lowering methods still contain explicit
  scaffold failures.
- `uef-gen/src/uef_gen/language/frontends/system/` contains the separate system-language
  contract and its unfinished compiler boundary.
- `uef-gen/src/uef_gen/generation/pipeline.py` accepts the existing hardware
  configuration path; it is not yet the standalone application-IR renderer.
- `uef-gen/src/uef_gen/modules/selector.py` performs fail-closed selection and
  dependency closure for its current route. The gap is integration with the
  standalone application IR and broader selected capability graph.
- `UETD/registry/modules/authoring_contract.schema.json` is a useful starting
  schema for typed module authoring and generation metadata; the per-module
  contracts are still sparse.
- `UETD/registry/targets/chips/manifest/stmicroelectronics/stm32f722ret6.json`,
  `stm32h745xih6.json`, and `stm32g474vet6.json` illustrate that part identity
  and peripheral-group metadata do not themselves provide an available board
  generation profile.
- `UETD/docs/audits/UETD_SYSTEM_AUDIT.md`, `UETD/docs/audits/UCON_CATALOGUE_AUDIT.md`, and
  `UETD/docs/catalogues/UMATH_CATALOGUE.md` record the current layer/module counts and
  implementation status used by this review.

## Accepted design-gap scaffold follow-up

The compatibility assessment above remains the acceptance baseline: a
design-only roadmap entry does not make a source feature generatable. The
follow-up implemented the current bounded health/log/fault/watchdog runtime
paths and added a machine-readable, non-selectable future catalogue for the
additional gaps identified in the assessment. Its linked UETD and `uef-gen`
contracts now state operational bounds, ownership, target/profile constraints,
recovery/freshness rules, and promotion gates for data services, peripheral
extensions, industrial protocols, language types, external hardware/evidence,
and secure field updates.

The future catalogue can be loaded for roadmap inspection and reference
integrity. It is deliberately disconnected from the selectable module
registry, project completions, normal umbrella headers, and generation routes.
No new source-equivalence claim follows until those contracts are promoted,
implemented and qualified for the exact target configuration.
