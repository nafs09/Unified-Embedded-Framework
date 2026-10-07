# UEF system-wide audit

Audit scope: the UEF source tree, public headers, CMake source selection,
the UEF module/target/ControlIR registries and their detail manifests,
`registry/modules/registry.json`, `uef_api.json`, examples,
and the current `UEF_Specification.md` (V1.2). This is a static source and
contract review. No build or tests were run.

## Structural checks

- The public C header scan found **1,589** API-like declarations after adding
  the health snapshot API and future protocol operation outlines; each has a
  source definition. Header-only inline functions and target-conditional
  declarations are also present.
- All **25** `c_function` symbols recorded in `uef_api.json` resolve to a
  declaration/definition in the tree. The module manifest's referenced source
  paths resolve. The API registry remains intentionally focused on the
  generator-facing UCON/UPAL/UMID/UHAL operations described in Part XIII.
- The shared ControlIR manifest contains eight selectable UPROTO nodes and 211
  UCON catalogue rows. Only PID and lead-lag are registered UCON implementations.
- Part XIV lists host and hardware test trees, but there is currently no
  `tests/` directory. Examples exist, though hardware examples are not evidence
  of target support. No tests were added or run for this audit.

## Layer-by-layer state

| Layer | Current state | Main work before release |
|---|---|---|
| UCORE | Shared storage types, status conversion, time API, assertions, and limits are present. Target-selected UHAL supplies the time implementation. | Review integer overflow, time-source, and assertion policy against each supported compiler/target. |
| UMATH | Shared scalar/fixed-point, vector, complex, matrix/Cholesky, quaternion, rigid-transform, and polynomial APIs are implemented. The catalogue now classifies 19 Phase 0 priority scaffolds, 3 Phase 1 scaffolds, and one later sparse-CSR scaffold; all remain fail-closed and unavailable to generation. | Implement and numerically qualify the Phase 0 math foundation needed by nonlinear estimators, spectral PLL work, and predictive control; keep every scaffold blocked from generation until reviewed. |
| UHAL | Host simulation exists. Cortex-M APIs have declarations and target sources; most target functions remain no-op or neutral scaffolds. Target-family records describe six families but are not exact-part board profiles. | Implement and verify clocks, GPIO, IRQ/NVIC, cache/MPU, memory, timing, fault capture, and atomic semantics for a selected board. |
| UPAL | Per-peripheral APIs and source files exist, including SDMMC and separate FDCAN-FD. Most operations return `UEF_NOT_SUPPORTED` or are no-op scaffolds. | Implement target-backed DMA, serial, timer, conversion, CAN, storage, watchdog, and other drivers; Phase 1 SDMMC/FDCAN are not hardware-ready. |
| UOS | Bare-metal and FreeRTOS source surfaces exist. FreeRTOS source/lock is not release-ready. A SafeRTOS API-operation outline now exists, but is not selectable. | Pin the FreeRTOS commit and port profile; decide timeout, ISR, storage, and scheduler failure semantics. Add the licensed SafeRTOS adapter only with a consumer-provided package and verified port contract. |
| UMID | Ring buffer and health registry logic exist; sensor, logging, and storage adapters are largely stubs. A coherent health snapshot API is now used by the supervisor. | Implement device/board adapters, units/calibration, synchronization, timestamps, stale-data behavior, and SDMMC/FatFS error mapping. |
| UPROTO | PPM is a fixed-memory interval decoder. DSHOT, CRSF, SBUS, MAVLink, and UAVCAN/DroneCAN APIs remain fail-closed. Six Part XV.3 future modules now have operation-level outlines but no stable typed contract, output template, or dispatch entry. | Implement/validate current protocol cores, resolve the transport and deferred-processing boundary, then promote future modules one by one. |
| UAPP | Lifecycle, table-driven state machine, and periodic supervisor behavior exist. `uapp_fault_report` is intentionally a no-op; fatal reporting has incomplete context semantics. | Define a bounded recoverable-fault sink, reset/fault capture policy, and concurrent supervisor snapshot semantics. |
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
5. **Missing planned outlines:** added non-selectable, fail-closed source and
   header outlines for MSP, ExpressLRS, UAVCAN v1, LIN, J1939, and IEC 60870-5,
   plus all 23 operations in the existing UOS API for the named SafeRTOS
   backend. These are excluded from normal CMake builds and are not advertised
   by `protocol_provides` or the ControlIR manifest.
6. **Spec revision comments:** updated 89 public-header/source comments from
   the stale V1.1 reference to the current V1.2 specification.
7. **Fault reporting honesty:** the nonfatal fault stub now states why it does
   not claim that the event was recorded and lists the sink, lifetime, and
   interrupt-context requirements for implementation.
8. **UCORE/UHAL ownership:** moved `uef_time_now_us` and delay implementations
   into the selected UHAL backend and removed the UHAL include from UCORE's
   assertion source. The module graph no longer needs an impossible
   `ucore`↔`uhal` dependency for the portable time source.
9. **Ignored build artifacts:** 47 tracked paths under UEF `.vs/` and `out/`
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
5. **Fault API:** Part XI describes recoverable fault logging and fatal fault
   capture. `uapp_fault_report` currently has no output sink. The public header
   says fatal handling asserts IWDG, while the source stores zero PC/LR
   placeholders and requests a software reset without starting the watchdog.
   Define whether the platform exception handler owns context capture, how
   recoverable reports are delivered, and which reset path is normative; then
   update the header/source together.
6. **Concurrency:** the new ring buffer has an explicit SPSC contract. `flush`
   and initialization require both sides stopped, and multi-producer/multi-
   consumer access is unsupported. `umid_health_entry_copy` is the safe
   snapshot path. CRSF/SBUS frame freshness and publication, UPROTO parser
   context, and host multi-threaded critical-section behavior still need
   contracts. Cortex-M UHAL atomics briefly mask all maskable interrupts and
   are not suitable for NMI or multicore use.
7. **FreeRTOS packaging:** `third_party/freertos-kernel.lock.json` has an empty
   revision and license-file list, and the upstream kernel source is absent.
   CMake correctly rejects that backend. Pin and add the reviewed dependency
   before claiming FreeRTOS build support.
8. **Tests and installed package:** the spec's Part XIV test tree is absent.
   CMake installs the library and headers but does not yet install a package
   config plus UEF metadata/templates for downstream `uef-gen` use. Decide
   whether the supported distribution is a source checkout or an installed
   SDK, then add that packaging contract.
9. **V1.2 catalogue conflicts:** the separate UCON audit still records the CKF
   third-degree spherical-radial versus stale Gauss-Hermite wording and the
   DEADBEAT_MPC priority inconsistency. These are specification edits, not
   algorithm implementations.

## Added UPROTO operation outline groups

The future protocol modules use `uproto_future_call_t` only as a development
work envelope. Each operation has a named declaration, definition, and
protocol-specific TODO. No future module is exposed through the common
`uproto.h`, module provider map, ControlIR dispatch, or generated templates.

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
library/license inputs, bounded memory/work, and UEF-owned wrapper templates.

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
the normal UEF source set; the CMake future option applies only to the later
`src/umath/future/` directory. FFT/windowing is prioritized for spectral PLL
acquisition, harmonic analysis, and diagnostics; PLL loops that use a time-domain
phase detector may not need an FFT in every update. UCON retains PID/integrator
state and anti-windup behavior, while UMATH supplies reusable quadrature and
ODE operations for control and model propagation.
