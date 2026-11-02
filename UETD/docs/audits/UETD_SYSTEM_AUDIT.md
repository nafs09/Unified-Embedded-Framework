# UETD system-wide implementation status

UETD is the portable C11 library for embedded applications. Its public APIs
cover core utilities, hardware abstraction, peripheral access, operating
systems, math, middleware, protocols, and application services. UETD owns the
registries, manifests, templates, and target contracts consumed by uef-gen.

## Repository map

- Public module APIs are grouped under UETD/include; implementations are
  selected through UETD/src and the CMake module graph.
- api.json locates the module, type, unit, math, runtime, chip, and board
  catalogues. Registry indexes point to the owning manifests and schemas.
- UETD/templates contains target and module scaffolds. A template or registry
  entry does not by itself establish a usable or qualified capability.
- UETD/examples shows application shapes and integration patterns. Example
  source is not evidence of physical-board support.
- UETD/docs/TODO.md and the repository implementation plan track unfinished
  contracts, implementations, integrations, and target qualification.

## UEF-PL and firmware generation

UEF Project Language (UEF-PL) has an implemented frontend in uef-gen. Its
lexer, parser, registry resolution, semantic checks, editor analysis, and
backend-neutral lowering accept .uef projects and .uefs system contracts. The
standalone firmware emitter is a separate incomplete stage; build and flash
routes for language projects therefore remain unavailable. See the
[UEF-PL implementation status](../../../uef-gen/docs/audits/LANGUAGE_AUDIT.md)
for the frontend and generation boundary.

The hardware-configuration route is separate. It resolves UETD data and can
assemble a project scaffold, but a scaffold is not a verified build or
flash-ready image. Exact part, board, toolchain, startup, linker, and deployment
facts remain required for target qualification.

## Capability boundary

- Registry discovery means a name and its metadata can be inspected; it does
  not mean the module has an implemented source path or generation route.
- Scaffold-only modules and incomplete contracts remain blocked from selection
  or generation.
- Family and chip records do not substitute for a complete board profile.
- Portable library builds, target builds, and physical hardware qualification
  are separate capability levels.
- Safety, timing, and fault-handling claims require evidence for the selected
  implementation and target.

## Layer-by-layer state

| Layer | Current state | Remaining capability gaps |
|---|---|---|
| UCORE | Shared storage types, status conversion, time API, assertions, and limits are present. Target-selected UHAL supplies the time implementation. | Integer-overflow, time-source, and assertion policies are not fully qualified across supported compilers and targets. |
| UMATH | Shared scalar/fixed-point, vector, complex, matrix/Cholesky, quaternion, rigid-transform, and polynomial APIs are implemented. The catalogue classifies 20 Phase 0 entries, 3 Phase 1 entries, and one later sparse-CSR entry that remain generation-blocked. Generic-vector and dual-number direct-C functions are implemented within those blocked identities; 73 public math operations still return not-implemented. | The Phase 0 math foundation for nonlinear estimators, spectral PLL work, and predictive control is not yet implemented and numerically qualified. Scaffold operations remain unavailable to generation. |
| UHAL | Host simulation exists. Cortex-M APIs have declarations and target sources; most target functions remain no-op or neutral scaffolds. The registries describe 68 families and seven C family-profile scaffolds; all 16 physical board records remain generation-unavailable. | Clock, GPIO, IRQ/NVIC, cache/MPU, memory, timing, fault-capture, and atomic behavior is not qualified for a selected board. |
| UPAL | Per-peripheral APIs and source files exist, including SDMMC and separate FDCAN-FD. Active portable contracts cover external radio buses, packet netifs, Ethernet MAC/PHY and the generic display facade; IEEE 802.15.4 remains a future frame-provider outline. | Most peripheral operations remain fail-closed. No radio/network provider or exact target has been integrated or qualified; target-backed DMA, serial, timer, conversion, CAN, storage, and watchdog drivers also remain incomplete. |
| UOS | Bare-metal and FreeRTOS source surfaces exist. Bare-metal synchronization calls are unsupported stubs. FreeRTOS Kernel 11.3.1 is staged with an upstream lock and one generic ARM Cortex-M4F GCC source profile; the copied tree has not been checksum-compared with upstream, and no board is qualified. The API still uses ambiguous result forms, backend time epochs differ, and deadline updates are unchecked. A SafeRTOS operation outline exists but is not selectable. | Define status-returning poll/timeout semantics, common monotonic time, transactional deadlines, ISR context, object lifetime/concurrency, and complete target ABI storage checks. The generic profile still requires a matching toolchain and `FreeRTOSConfig.h`; exact startup, clock, linker, interrupt priorities, and hardware evidence are target-specific. See the [UOS API contract](../design/UOS_CONTRACTS.md). |
| UMID | Ring buffer, bounded health registry, coherent snapshots, freshness query, and bounded transport-neutral text logger are implemented. UART is an optional sink adapter; a project may provide a bounded BLE/WiFi/IP sink without adding UPROTO dependencies. Device-specific drivers remain consumer-authored scaffolds. Storage, calibration, binary blackbox, and process-image APIs have first-pass headers, source stubs, and blocked module records. | Device/board drivers, calibration, synchronization, timestamps, stale-data behavior, UPAL error mapping, and SDMMC/FatFS integration remain incomplete. First-pass data services still need committed implementations, integrity/recovery rules, and backend semantics. |
| UPROTO | PPM is a fixed-memory interval decoder. DSHOT, CRSF, SBUS, MAVLink, and UAVCAN/DroneCAN APIs remain fail-closed. Active typed provider APIs cover WiFi, Bluetooth LE/Classic SPP, and IP/DHCP/DNS/sockets; TLS/DTLS, MQTT, HTTP, Modbus and CANopen CC are first-pass outlines. PPP, CoAP, Thread/Zigbee and separate CANopen FD/USDO remain later outlines. Myovena MEDI CAN is a separate fail-closed protocol-extension scaffold with planned reliable delivery and execution gating. | Active APIs do not supply vendor radios or network/security stacks. No provider or exact target is integrated or qualified; current UHAL target selection does not include ESP-IDF. Existing protocol cores and target integrations remain incomplete; provider memory/timing/security contracts still require exact-board qualification. |
| UAPP | Transitional component lifecycle, bounded run-to-completion event/state machine, periodic status/staleness supervisor, health-lease watchdog gate, bounded recoverable-fault queue, and task-context fault dispatch exist. The FSM is single-owner, flat, non-ISR-safe, and requires static configuration-table lifetimes. Its queue rejects newest events with `UETD_OVERFLOW`; critical-event recovery is application-owned. Fatal backup metadata still has zero PC/LR placeholders; durable crash history and secure update remain design-only first-pass services. | Safe-state cleanup after callback failure, richer hierarchy, target watchdog qualification, and architecture-captured crash context with retained-record handling remain incomplete. |
| UCON | Detailed catalogue and function-level scaffolds are covered by `UCON_CATALOGUE_AUDIT.md`. PID and lead-lag are the only registered implementations. | CKF wording, DEADBEAT_MPC priority conflicts, and typed contracts for additional algorithms remain unresolved. |

## Current implementation contracts

- The ring buffer uses UHAL 32-bit atomics under a single-producer/single-
  consumer contract. Initialization and flush require both sides to be
  quiescent; the Cortex-M critical section is single-core and not NMI-safe.
- Health snapshots copy entries while holding the critical section. The legacy
  pointer API is a borrowed view for serialized callers.
- The ring buffer, health registry, and application fault path declare their
  direct UHAL dependencies. Protocol parsing and publication remain constrained
  to bounded task or deferred-interrupt contexts.
- Recoverable faults enter a bounded RAM queue with copied detail, monotonic
  timestamps, a dropped counter, and task-context dispatch. Fatal reset metadata
  still uses placeholder PC/LR values; retained crash history remains a
  separate first-pass service.
- First-pass and later protocol outlines and target profiles remain
  fail-closed and outside normal selectable capabilities. Scope placement
  does not change their implementation or qualification status.

## Target and release gaps

1. **Target coverage:** the specification names broader target tiers than the
   current HOST and generic CORTEX_M backends support. Cortex-M sources are not
   board-qualified, and TI C2000 and RISC-V backends are absent.
2. **SafeRTOS:** no selectable backend or external package contract is present.
   The source outline does not establish an implemented or licensed port.
3. **SDMMC and FDCAN-FD:** source files and metadata exist, but their operations
   are not target implementations. FatFS cannot complete I/O without a working
   SDMMC route.
4. **Protocol context:** the specification mixes transport-independent framing
   with UART-bound CRSF/SBUS APIs and ISR parsing with deferred task processing.
   The public transport and execution-context contract remains unresolved.
5. **Fault API:** recoverable faults are queued in RAM; fatal reports still use
   placeholder PC/LR values. Retained crash context and reset-safe persistence
   are separate first-pass contracts.
6. **Concurrency:** ring-buffer, logger, health-snapshot, and critical-section
   ownership is bounded to documented single-core patterns. CRSF/SBUS frame
   freshness, parser context, and host multi-threaded behavior need explicit
   contracts.
7. **FreeRTOS packaging:** the repository contains the 11.3.1 source, license,
   upstream identity metadata, and a generic ARM Cortex-M4F GCC profile. That
   profile still needs a generated FreeRTOSConfig.h and matching toolchain; it
   does not qualify a board.
8. **Distribution:** CMake installs the library and headers, while a package
   configuration plus UETD metadata/templates for downstream uef-gen use is
   not yet defined.
9. **UCON catalogue:** the separate catalogue review records unresolved CKF
   wording and DEADBEAT_MPC priority conflicts. These are specification issues,
   separate from algorithm implementation.

## UPROTO future operation outlines

The future protocol modules use `uproto_future_call_t` only as a development
work envelope. Each operation has a named declaration, definition, and
protocol-specific TODO. No future module is exposed through the common
`uproto.h` or module provider map.

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

## UMATH implementation priorities

The catalogue was reclassified after checking it against the active NEXUS work:
nonlinear ES-EKF/InEKF/UKF, predictive control, PLL/spectral control, trajectory
generation, and the planned medical estimator. Phase 0 now contains 20
generation-blocked entries, including Jacobians and forward-mode autodiff, quadrature/ODE steps,
FFT/windows, SO(3)/SE(3), LU/LDLᵀ/QR/SVD/eigensystems, probability/statistics,
least squares, and trajectory math. Phase 1 contains SO(2), SE(2), and seeded
random streams. Sparse CSR is the sole later UMATH scaffold.

The refreshed inventory contains 35 UMATH module records and 331 public function declarations. All declared source/header paths resolve. Of these declarations, 258 have source or inline bodies without a not-implemented return and 73 remain fail-closed. The 24 blocked module identities include implemented direct-C generic vectors and dual numbers awaiting binding review.

These phase labels change implementation order only. The unimplemented scaffold APIs remain outside the stable umbrella, carry `available_for_generation: false`, and return `UMATH_NOT_IMPLEMENTED`. Direct-C implementations in blocked module identities must be distinguished from those stubs. Phase 0/1 outline translation units compile in
the normal UETD source set; the CMake future option applies only to the later
`src/umath/matrix/sparse_csr.c` file (excluded from the default library). FFT/windowing is prioritized for spectral PLL
acquisition, harmonic analysis, and diagnostics; PLL loops that use a time-domain
phase detector may not need an FFT in every update. UCON retains PID/integrator
state and anti-windup behavior, while UMATH supplies reusable quadrature and
ODE operations for control and model propagation.

## Future catalogue boundary

The machine-readable future catalogue is available for roadmap inspection and
reference integrity. Its design-only status does not grant module selection or generation. Some future capabilities also have explicitly blocked module records for source discovery; cross-index presence does not grant readiness. Future entries
become capabilities only after their contracts, implementations, target
bindings, and qualification evidence are complete.

The watchdog gate supervises refreshes routed through its injected callback;
direct hardware feeds outside that route remain outside the generic API's
control. First-pass security/storage interfaces now have typed unsupported
scaffolds, while host evidence obligations have a separate blocked generation
catalogue. Neither establishes completed runtime or source-to-image behavior.
