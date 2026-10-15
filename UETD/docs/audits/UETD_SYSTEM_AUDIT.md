# UETD system-wide audit


This file is the implementation-status snapshot. The broader capability and
source-equivalence review, including selective generation and requirements
beyond the supplied systems, is tracked in
[`UETD_EQUIVALENCE_AND_CATALOGUE_GAP_AUDIT.md`](UETD_EQUIVALENCE_AND_CATALOGUE_GAP_AUDIT.md).

## Structural checks

- The public C header scan found **1,589** API-like declarations after adding
  the health snapshot API and future protocol operation outlines; each has a
  source definition. Header-only inline functions and target-conditional
  declarations are also present.
- All **25** `c_function` symbols recorded in `api.json` resolve to a
  declaration/definition in the tree. The module manifest's referenced source
  paths resolve. The API registry remains intentionally focused on the
  generator-facing UCON/UPAL/UMID/UHAL operations described in Part XIII.
- Part XIV lists host and hardware test trees, but there is currently no
  `tests/` directory. Examples exist, though hardware examples are not evidence
  of target support. No tests were added or run for this audit.

## Standalone DSL and generator readiness

The standalone language is not yet an executable generation pipeline. The
language design and examples are useful implementation targets, but they are
not acceptance evidence for generated firmware.

- `uef-gen/src/uef_gen/language/frontends/project/lexer.py`, `parser.py`,
  `validation.py`, and `lowering.py` still raise `NotImplementedError` for the
  frontend phases. `compile_project_file` and `compile_project_text` return
  `project_language_frontend_incomplete` without producing hardware
  configuration or a `StandaloneApplicationSpec`.
- `system_language` has the same blocker: `.uefs` compilation and validation
  return `system_language_frontend_incomplete`.
- Build and flash adapters remain scaffolded. The standalone `.uef` route
  cannot reach a compiled image, and target-specific flash profiles are not
  complete. Thus `check`, `generate`, `build`, and `flash` are not operational
  for the examples yet.
- The extension supplies TextMate highlighting, registry-derived semantic
  tokens, and basic line-level warnings for registered modules whose authoring
  or generation contracts are incomplete. It does not yet parse the language,
  type-check settings, resolve member ports, or provide context-aware
  completion and hover. Semantic tokens currently cover software-module
  descriptors only; target-resource names are not resolved there yet.
- A static scan of the UETD module index found **308** registered module IDs and
  `DSL_descriptor` values, with **no descriptor collisions**. Only **4** of
  those manifests currently contain an `authoring` contract: `h_bridge`,
  `pid`, `enmpc`, and `fpga`. `h_bridge`, `pid`, and `enmpc` are draft and have
  draft generation status; `fpga` is reviewed and generation-ready, with an
  explicit contract for endpoint message types, field mappings, receiver-local
  timestamps, and required application bindings. The
  remaining **304** identifiers are discoverable by spelling, but do not yet
  have enough UETD-owned parameter/port/readiness metadata for semantic
  validation or standalone generation.
- There is not yet a UETD-owned callable math-function/overload registry for the
  language. Module descriptors alone cannot validate formula calls, units,
  dimensional operations, or numerical overloads.
- The example scan found `gpio` and `exti` used as peripheral types but absent
  from the software-module index. These are target resources, not missing
  software modules, so the grammar now treats peripheral type names as
  `target_peripheral_ref` values. UETD target manifests still expose only broad
  peripheral-group strings and lack a complete canonical resource/pin/IRQ/DMA
  catalogue; the target-side resolver and validation contract remain to be
  implemented.
- The Tesla executor example now targets the user's selected **STM32G474VET6**
  part. Its chip record is present, but it is unavailable for generation
  pending board, pin, clock, timer, startup/linker, build, and flash profiles.
  The example now follows the revised architecture in the supplied source:
  G474 captures timer edges and sends raw counts over Fibre; H745 M4 extends
  timestamps and reconstructs phase; H745 M7 builds the sensor snapshot and
  runs estimation/control. The H745 board and multicore profile remain
  unqualified.

These are release blockers for the user's stated acceptance criteria. Do not
describe the examples as compilable or the standalone pipeline as complete
until parser, validation, lowering, UETD rendering, build, and flash paths work
end to end and are exercised against at least the drone and coil sketches.
This pass did verify JSON syntax for 731 registry/config files, Python syntax
for 85 `uef-gen` source files, and the VS Code extension's JavaScript syntax.
It also cross-checked all 308 module descriptors, 52 chip entries, 16 board
entries, family-to-chip part lists, and example target IDs; no reference or
syntax errors were found in those checks. These checks do not replace schema
validation, C/C++ compilation, or end-to-end generation tests.

## Layer-by-layer state

| Layer | Current state | Main work before release |
|---|---|---|
| UCORE | Shared storage types, status conversion, time API, assertions, and limits are present. Target-selected UHAL supplies the time implementation. | Review integer overflow, time-source, and assertion policy against each supported compiler/target. |
| UMATH | Shared scalar/fixed-point, vector, complex, matrix/Cholesky, quaternion, rigid-transform, and polynomial APIs are implemented. The catalogue now classifies 19 Phase 0 priority scaffolds, 3 Phase 1 scaffolds, and one later sparse-CSR scaffold; all remain fail-closed and unavailable to generation. | Implement and numerically qualify the Phase 0 math foundation needed by nonlinear estimators, spectral PLL work, and predictive control; keep every scaffold blocked from generation until reviewed. |
| UHAL | Host simulation exists. Cortex-M APIs have declarations and target sources; most target functions remain no-op or neutral scaffolds. Target-family records describe six families but are not exact-part board profiles. | Implement and verify clocks, GPIO, IRQ/NVIC, cache/MPU, memory, timing, fault capture, and atomic semantics for a selected board. |
| UPAL | Per-peripheral APIs and source files exist, including SDMMC and separate FDCAN-FD. Most operations return `UETD_NOT_SUPPORTED` or are no-op scaffolds. | Implement target-backed DMA, serial, timer, conversion, CAN, storage, watchdog, and other drivers; Phase 1 SDMMC/FDCAN are not hardware-ready. |
| UOS | Bare-metal and FreeRTOS source surfaces exist. FreeRTOS Kernel 11.3.1 is staged with an upstream lock and one generic ARM Cortex-M4F GCC profile; the copied tree has not been hash-verified and no board is qualified. A SafeRTOS API-operation outline exists, but is not selectable. | Verify the local FreeRTOS copy against upstream, add only profiles for selected targets, then validate configuration, timeout, ISR, storage, scheduler, startup, and linker behavior. Add SafeRTOS only with a consumer-provided package and verified port contract. |
| UMID | Ring buffer, bounded health registry, coherent snapshots, freshness query, and bounded UART text logger are implemented. Device-specific drivers remain consumer-authored scaffolds; persistent storage, calibration records, and binary blackbox are design-only future services. | Implement device/board drivers by hand with units/calibration, synchronization, timestamps, stale-data behavior, propagated UPAL errors, and SDMMC/FatFS error mapping. Promote the future storage contracts when atomic commit, recovery and backend semantics are agreed. No exhaustive per-device registry is planned. |
| UPROTO | PPM is a fixed-memory interval decoder. DSHOT, CRSF, SBUS, MAVLink, and UAVCAN/DroneCAN APIs remain fail-closed. Six Part XV.3 future modules now have operation-level outlines but no stable typed contract, output template, or dispatch entry. | Implement/validate current protocol cores, resolve the transport and deferred-processing boundary, then promote future modules one by one. |
| UAPP | Transitional component lifecycle, bounded run-to-completion event/state machine, periodic status/staleness supervisor, health-lease watchdog gate, bounded recoverable-fault queue, and task-context fault dispatch exist. The FSM is single-owner, flat, non-ISR-safe, and requires static configuration-table lifetimes. Its queue rejects newest events with `UETD_OVERFLOW`; critical-event recovery is application-owned. Fatal backup metadata still has zero PC/LR placeholders; durable crash history and secure update remain design-only future services. | Preserve one transition owner; route cross-task/ISR ingress through target-safe queues, require callers to handle overflow, and establish safe-state cleanup before reset after callback failure. Define richer hierarchy/orthogonal behavior only against a concrete consumer. Qualify the target watchdog adapters and ensure every feed for a watchdog is routed through one gate. Implement architecture-captured crash context and retained-record commit/acknowledgement under the future fault-history contract. |
| UCON | Detailed catalogue and function-level scaffolds are covered by `UCON_CATALOGUE_AUDIT.md`. PID and lead-lag are the only registered implementations. | Resolve the CKF wording and deadbeat priority conflicts, and specify typed contracts before promoting stubs. |

## Findings fixed in this pass

1. **Ring-buffer synchronization:** the old implementation used `volatile`
   counters with `atomic_thread_fence`; fences do not make non-atomic index
   accesses a sound concurrent contract. The implementation now uses UHAL's
   32-bit atomic operations, documents one producer/one consumer, bounds
   occupancy before copying, and requires callers to quiesce both sides before
   init/flush. The Cortex-M UHAL atomic backend now uses a short nesting-safe
   PRIMASK critical section. This is single-core and not NMI-safe; multicore
   sharing remains outside the contract.
2. **Health registry snapshots:** `uapp_supervisor_tick` previously read a
   borrowed registry pointer after the registry lock had ended. It now copies
   one entry with `umid_health_entry_copy` while holding the critical section.
   The older pointer API remains for serialized callers and is documented as a
   borrowed view.
3. **Module dependency precision:** removed unused UART/ring-buffer dependencies
   from the generic MAVLink stream adapter, removed the unused timer dependency
   from PPM's elapsed-interval decoder, and removed the unused ring-buffer
   dependency from SBUS. Added the direct UHAL dependency used by the ring
   buffer, health registry, and UAPP fault path.
4. **Protocol execution context:** CRSF/SBUS header comments now direct parsing
   to a bounded task or deferred interrupt path, matching Part XVII's
   integration guidance. Their frame publication still needs a concurrency
   contract.
6. **Spec revision comments:** updated 89 public-header/source comments from
   the stale V1.1 reference to the current V1.2 specification.
7. **Fault reporting honesty:** the nonfatal fault stub now states why it does
   not claim that the event was recorded and lists the sink, lifetime, and
   interrupt-context requirements for implementation.
8. **UCORE/UHAL ownership:** moved `uetd_time_now_us` and delay implementations
   into the selected UHAL backend and removed the UHAL include from UCORE's
   assertion source. The module graph no longer needs an impossible
   `ucore`↔`uhal` dependency for the portable time source.
9. **Ignored build artifacts:** 47 tracked paths under UETD `.vs/` and `out/`
   were removed from the Git index because the existing `.gitignore` already
   excludes them. Their local IDE/build files remain on disk.

## Specification and release gaps

1. **Target support claims:** Part III calls several architecture tiers
   “Complete UHAL + UPAL support,” including TI C2000. The current CMake target
   choices are only `HOST` and generic `CORTEX_M`; the repository has no TI
   C2000 or RISC-V UHAL backend, and the Cortex-M source is not board-qualified.
   Either qualify the promised target matrix or relabel these entries as
   planned coverage before presenting the spec as current support.
2. **SafeRTOS:** Parts II and the target statement name a SafeRTOS adapter, but
   there is no selectable backend or external package contract. The new source
   is only a work outline. The licensed dependency, exact API release, port,
   storage, and ISR rules must be supplied before implementation.
3. **Phase 1 peripherals:** SDMMC, FatFS diskio, and FDCAN-FD files and metadata
   exist, but this only proves source/API scaffolding. SDMMC and FDCAN-FD
   operations are not target implementations; the FatFS adapter cannot
   complete I/O until SDMMC does.
4. **UPROTO transport policy:** Part X says the same protocol frame generator
   must be transport-independent, while CRSF and SBUS public APIs currently
   take `upal_uart_t*`. Part X also says to process from UART IDLE ISR; Part
   XVII assigns work to a bounded task/deferred interrupt path. Reconcile these
   statements and decide whether the public API is byte-stream based or
   UART-bound before implementing those parsers.
5. **Fault API:** Recoverable reports now enter a bounded RAM queue, include a
   copied/truncated detail and monotonic timestamp, expose a dropped counter,
   and can be dispatched to a recovery handler from task context. This queue
   does not survive reset. Fatal reports still store zero PC/LR placeholders
   and request a software reset; the future retained crash-history contract
   specifies architecture context, atomic persistence and acknowledgement.
6. **Concurrency:** the new ring buffer has an explicit SPSC contract. The text
   logger serializes task/main producers on one core, uses one drain consumer,
   and is not an ISR formatter or cross-core queue. Health reports and coherent
   snapshots use the UHAL critical section; supervisor staleness is configurable.
   `flush` and initialization require both sides stopped, and multi-producer/multi-
   consumer access is unsupported. `umid_health_entry_copy` is the safe
   snapshot path. CRSF/SBUS frame freshness and publication, UPROTO parser
   context, and host multi-threaded critical-section behavior still need
   contracts. Cortex-M UHAL atomics briefly mask all maskable interrupts and
   are not suitable for NMI or multicore use.
7. **FreeRTOS packaging:** `third_party/FreeRTOS-Kernel-11.3.1/` now contains the
   kernel source, MIT license, upstream tag/commit lock, and UETD integration
   manifest. The local copy has not been checksum-compared with the upstream
   commit. The generic ARM Cortex-M4F GCC profile still requires a generated
   `FreeRTOSConfig.h` and a matching cross-toolchain; it is not board support.
   uef-gen target generation remains unavailable until an exact target record
   is completed and qualified.
8. **Tests and installed package:** the spec's Part XIV test tree is absent.
   CMake installs the library and headers but does not yet install a package
   config plus UETD metadata/templates for downstream `uef-gen` use. Decide
   whether the supported distribution is a source checkout or an installed
   SDK, then add that packaging contract.
9. **V1.2 catalogue conflicts:** the separate UCON audit still records the CKF
   third-degree spherical-radial versus stale Gauss-Hermite wording and the
   DEADBEAT_MPC priority inconsistency. These are specification edits, not
   algorithm implementations.

## Added UPROTO operation outline groups


- **MSP:** profile validation, initialization, bounded byte feed, request
  encoding, response extraction, reset.
- **ExpressLRS:** profile validation, CRSF binding, link update, coherent link
  snapshot, reset.
- **UAVCAN v1:** initialization, bounded spin, publish, subscribe, reset.
- **LIN:** schedule validation, initialization, header scheduling, response
  validation, reset.
- **J1939:** initialization, frame processing, address claim, PGN send/receive,
  reset.
- **IEC 60870-5:** profile selection, initialization, bounded parsing, ASDU
  encoding, event extraction, reset.

Before a proposal becomes selectable, replace the type-erased envelope with a
fixed-size typed API; define protocol version/profile, frame and payload limits,
transport dependencies, ISR/task ownership, error/freshness policy, external
library/license inputs, bounded memory/work, and UETD-owned wrapper templates.

## UMATH priority review

The catalogue was reclassified after checking it against the active NEXUS work:
nonlinear ES-EKF/InEKF/UKF, predictive control, PLL/spectral control, trajectory
generation, and the planned medical estimator. Phase 0 now contains 19
scaffolds, including Jacobians and forward-mode autodiff, quadrature/ODE steps,
FFT/windows, SO(3)/SE(3), LU/LDLᵀ/QR/SVD/eigensystems, probability/statistics,
least squares, and trajectory math. Phase 1 contains SO(2), SE(2), and seeded
random streams. Sparse CSR is the sole later UMATH scaffold.

A static consistency pass confirmed 34 UMATH registry entries match the phase
lists and their declared source/header/dependency paths resolve. All 122
UMATH status-returning declarations have source definitions. The 23
scaffold-only module records remain unavailable to generation, and every
scaffold function has a focused TODO in its source.

These phase labels change implementation order only. The scaffold APIs remain
outside the stable umbrella, carry `available_for_generation: false`, and
return `UMATH_NOT_IMPLEMENTED`. Phase 0/1 outline translation units compile in
the normal UETD source set; the CMake future option applies only to the later
`src/umath/future/` directory. FFT/windowing is prioritized for spectral PLL
acquisition, harmonic analysis, and diagnostics; PLL loops that use a time-domain
phase detector may not need an FFT in every update. UCON retains PID/integrator
state and anti-windup behavior, while UMATH supplies reusable quadrature and
ODE operations for control and model propagation.

## Accepted catalogue-gap scaffold follow-up

The original findings above describe the audit baseline. The follow-up adds a
non-selectable future catalogue with contracts and type-level outlines for
transactional persistence, driver-owned calibration envelopes, binary
blackbox capacity/loss semantics, PLC process-image authority and freshness,
industrial Modbus/CANopen profiles, UART half-duplex, capture publication,
synchronized I/O, USB/Ethernet, signed update, retained crash history,
external FPGA/PLC artifacts, and qualification evidence. The `.uef`/`.uefs`
type gap is documented across `Option`/`Result`, tagged variants, typed flags,
fixed point, bounded bytes/text, and explicit shared-memory, wire and durable
encodings. These entries are design-only and cannot be selected for generation.

Concrete current-runtime follow-up implements bounded logger and recoverable
fault queues, health freshness supervision, and a lease-gated watchdog refresh
path. That watchdog gate still relies on each target/application routing every
refresh through its injected callback; the generic API cannot prevent an
uncoordinated direct hardware feed. The future security, storage and evidence
contracts remain proposals, not implementation or assurance claims. This
follow-up did not run a build or tests.
