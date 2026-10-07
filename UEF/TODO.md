# UEF implementation TODO

This list distinguishes linkable source scaffolding from working behavior. Public runtime APIs now have definitions in their layer source files, but each `UEF_NOT_SUPPORTED` return, neutral getter result, or function-level TODO still marks behavior that must be implemented and reviewed. A function being present in a `.c` file does not mean the driver or algorithm works.

## Recommended implementation order and exit gates

Implement UEF from stable contracts outward. The target, compiler, board and
memory/interrupt assumptions must be known before target-facing code can be
called complete. Keep host simulation useful for portable logic, but do not
use it as evidence of embedded timing or peripheral correctness.

### 0. Freeze the first target and contract baseline

1. Select one concrete Cortex-M board and record the exact part/package,
   reference manual, datasheet, errata, CMSIS device header, compiler/toolchain,
   startup files, linker script, clock tree and interrupt priorities.
2. Reconcile public headers with the selected UEF contract: status values,
   integer widths, time units, callback contexts, buffer ownership, timeout
   semantics, C/C++ inclusion and unsupported-operation behavior.
3. Mark each public function as portable implementation, target implementation,
   optional dependency or explicit unsupported scaffold. Remove ambiguous
   success-shaped placeholders where unsupported behavior can be reported.
4. Establish the module manifest and build selection rules so exactly one UHAL
   target and one UOS backend are selected. Document external dependency and
   license/version ownership.

**Exit gate:** another developer can reproduce the host build and configure
the selected board build from documented inputs, and every exposed contract
has defined units, ownership, error behavior and target assumptions.

### 1. Complete portable core and target foundation

1. Confirm UCORE status/time behavior and integer overflow rules; review UMATH floating-point and fixed-point bounds, precision assumptions, and host simulation limits against supported targets.
2. Implement target startup assumptions and the minimum UHAL services needed
   for deterministic operation: core identity/capabilities, clock/reset,
   timing, GPIO, interrupt control, atomics/critical sections and fault path.
3. Implement cache/MPU/memory-region behavior only where the chosen target
   provides it; define DMA alignment and cache-maintenance ownership at the
   same time.
4. Define UOS bare-metal behavior for every synchronization API. Preserve
   explicit unsupported responses where the backend cannot provide the
   requested semantics. Before enabling the FreeRTOS backend, pin its upstream
   commit and add a verified architecture/compiler port profile plus a
   generated `FreeRTOSConfig.h` contract; never float to a newer kernel.
5. Verify each public UHAL operation against the selected board and document
   which capability macros are authoritative.

**Exit gate:** core and target boundary behavior is implemented or explicitly
unsupported, and the chosen target can initialize, report faults and provide
bounded timing without relying on neutral stub values.

### 2. Bring up one peripheral path end to end

1. Implement DMA ownership/cancel/timeout semantics and the cache rules before
   enabling DMA-backed drivers on cache-bearing targets.
2. Implement a small board-backed smoke path (UART or GPIO according to the
   selected board) so startup and target selection are exercised before
   Phase 1 peripheral work.
3. Add an example with exact board wiring and externally connected component
   revision; keep the example marked compile-only until its behavior is proven.
4. Implement other UPAL drivers by capability and family, grouping shared
   behavior only where hardware semantics actually match.
5. Complete SDMMC as a Phase 1 storage milestone: decide LBA width and device
   capacity contract first, then implement card identification, bounded
   initialization, async and blocking DMA transfers, erase, removal and
   recovery. Add the optional FatFS diskio adapter only after the raw block
   contract and FatFS version/configuration are fixed.

**Exit gate:** each enabled driver has a target/configuration record, explicit
error/timeout behavior, buffer/IRQ ownership rules and a hardware validation
record. Unimplemented capability paths remain unavailable rather than silently
selecting a different device implementation.

### 3. Finish middleware, protocols and application behavior

1. Specify cross-context synchronization for UMID ring buffers and registries;
   `volatile` plus fences alone is not the contract. Choose atomics or a
   documented critical-section policy per supported backend.
2. Implement sensor middleware with units, calibration ownership, timestamp
   source, stale-sample rules and propagation of driver failures.
3. Complete UPROTO's direct-use APIs and project wrappers: implement bounded
   framing/parsing, checksum validation before publishing state, and bounded
   interrupt-side work. Keep reusable protocol code in UEF; let the shared
   manifest declare thin instance templates and explicit manual actions for
   board transport setup or upstream dialect/stack integration.
4. Complete UAPP lifecycle, supervisor and fault behavior with board recovery
   policy, bounded fault logging and restart semantics.

**Exit gate:** every API has defined execution context and synchronization;
protocol and middleware errors reach callers, and lifecycle/fault behavior is
reviewed for the selected backend.

### 4. Implement UCON algorithms and publish reviewed entries

1. Keep algorithm meaning, typed public APIs, generic C implementations,
   numerical helpers, catalog metadata, and generated templates in UEF.
   NEXUS remains the graph author and `uef-gen` remains the generic ControlIR
   bridge and UEF-manifest consumer.
2. The shared UCON status/numeric APIs and generic PID and lead-lag
   implementations now exist. Their wrappers are the only UCON entries
   currently registered for generation. Reconcile their headers and equation
   contracts with Part XVI before extending the registered set. The catalogue audit identified a PID feature-parity question (deadband and rate limiting), a CKF quadrature wording conflict, and a DEADBEAT_MPC priority conflict; resolve the specification issues recorded in `UCON_CATALOGUE_AUDIT.md` before implementing those affected entries.
3. Every other catalog entry has an individual development declaration and
   definition in its algorithm family directory. First-pass entries and the
   planned project/common set sit beside implemented APIs; remaining planned
   and all deferred entries are under `include/uef/ucon/future/` and
   `src/ucon/future/`. These functions return `UCON_NOT_IMPLEMENTED` and stay
   unavailable to direct application code and uef-gen until replaced by typed,
   reviewed implementations.
4. Every unregistered algorithm now has a family-specific operation outline: each declared operation has a separate function heading, source definition, and focused TODO. Treat the outline as a work breakdown, not an API contract; confirm it against the algorithm specification and remove operations that are not required.
5. For each algorithm, specify fixed dimensions, units, state/configuration,
   init/reset behavior, parameter bounds, aliasing, status/failure semantics,
   numerical conditioning, saturation policy, maximum memory, and bounded
   execution before writing the implementation. Use the Part XVI entry or the clearly marked roadmap proposal plus applicable
   ControlIR fields as inputs; do not invent absent design values. A roadmap
   proposal must be specified in UEF before implementation.
6. Implement generic C first. Add a template only when it forwards to the same
   algorithm behavior and specializes explicit structural values. Declare
   output paths, required modules/numerics/features, and manual actions in
   each per-entry file under `registry/control_ir/manifest/` and its reference in
   `registry/control_ir/registry.json`; update `registry/modules/registry.json` and `uef_api.json`
   where the contract requires them.
7. Keep `registered: false` and `outputs: []` until the implementation and
   generated contract exist. The manifest adapter already reports distinct
   diagnostics for unregistered, direct-only, ambiguous-alias, and unknown
   nodes. Never add algorithm-specific IR types, equations, or fallback code
   to uef-gen.
8. Regenerate per-algorithm placeholders and their manifest/module/API paths
   with `tools/generate_ucon_scaffolds.py` after changing catalog family paths,
   keys, priorities, or the planned-project allowlist. Move a completed typed
   implementation into its normal family directory and remove its placeholder
   registration.

**Exit gate:** each advertised algorithm has a typed direct API or registered
template, reviewed numerical and memory bounds, defined failure behavior, and
matching direct/generated semantics.

### 5. Package, document and qualify supported configurations

1. Add examples only when their required drivers and board data are available;
   document wiring, expected signals, build configuration and unsupported
   states.
2. Keep build/install exports, module manifests, public headers and optional
   dependencies synchronized.
3. Record memory use, timing, interrupt latency, clock assumptions and cache/
   DMA rules for each released board/toolchain combination.
4. Promote a target from experimental to supported only after its documented
   build and hardware validation procedure has been completed.

**Exit gate:** a release identifies exact supported board/toolchain/backend
combinations and clearly separates host simulation, compile-only examples and
hardware-validated behavior.

## Phase 0 — portable foundation and initial integration

- [x] Promote the project-critical UMATH catalogue to Phase 0: automatic
  differentiation/Jacobians, quadrature and bounded ODE steps, FFT/windowing,
  SO(3)/SE(3), LU/LDLᵀ/QR/SVD/symmetric eigensystems, probability/statistics,
  least squares, splines, polynomial roots, and special functions. Individual
  headers, sources, and manifests exist; each remains fail-closed and unavailable
  to uef-gen.
- [ ] Implement the Phase 0 UMATH contracts in
  [`UMATH_CATALOGUE.md`](UMATH_CATALOGUE.md), starting with derivative/Jacobian
  support, deterministic fixed-work integration and estimator propagation,
  FFT/window conventions, and matrix solves for EKF/UKF/MPC. Define dimensions,
  workspace, aliasing, finite-value behavior, convergence limits, and scalar
  precision before allowing generation.
- [ ] Keep Jacobian ownership explicit: use analytic derivatives where the
  algorithm contract provides them, and implement bounded finite-difference or
  forward-mode helpers for remaining nonlinear process and measurement models.
  Check derivative accuracy against known functions before estimator integration.
- [ ] Keep controller state semantics in UCON. Implement UMATH quadrature/ODE
  primitives as reusable numerical operations; specify sample timing, integration
  rule, saturation, and anti-windup behavior in each UCON controller that uses
  them.
- [ ] Implement FFT and windows with fixed maximum lengths, caller-owned
  workspaces, documented real/complex layout and scaling, a twiddle-table policy,
  and bounded execution. Validate the selected PLL/spectral use cases and
  frequency/phase conventions.
- [ ] Complete the Phase 0 3D Lie and matrix foundation for ES-EKF/InEKF/UKF and
  MPC. Define perturbation conventions, covariance symmetry/conditioning policy,
  pivot/rank thresholds, and stable behavior near singularities before exposing
  these operations.
- [ ] Keep SO(2), SE(2), and deterministic PRNG at Phase 1. Keep sparse CSR under
  the later catalogue until a concrete project needs and specifies it.
- [ ] Review the public headers against the updated UEF specification's Parts XIV and file-tree/API examples; keep include paths, status names, time units, and C/C++ inclusion behavior consistent.
- [x] Add six Phase 0 family-profile records for STM32G4xx, STM32F4xx, single-core STM32H7xx, STM32F7xx, STM32G0xx, and nRF52840; all remain incomplete and contain no invented exact-part data.
- [x] Add the Phase 1 STM32L4+ profile for the SDMMC roadmap. Keep the Phase 0 H7/F7 profiles marked as Phase 1 SDMMC target families, and expose Phase 0, Phase 1, and complete registered-profile queries.
- [x] Publish the UEF-owned 65-entry family catalogue through uef_api.json, including later spec targets and broader STM32/Espressif series as non-generatable, unverified roadmap metadata.
- [x] Keep family, exact-part, module, and ControlIR indexes beside their per-entry detail files under registry, using one index-plus-manifest layout for each catalogue.
- [x] Add 38 vendor-sourced exact-part chip-base records across the specification's named vendor groups and expose them to uef-gen list/show queries. These records
capture identity and headline facts only; keep every record non-generatable until
its complete resource and board maps are reviewed.
- [x] Remove duplicate Phase 0 family data from uef-gen; its target listing reads UEF’s API-declared registry and expands per-family/per-chip manifests. Snapshots fingerprint the registry/detail trees and include them in provenance.
- [ ] Select and document the first exact MCU/package/board, toolchain, CMSIS device header, clock setup, linker script, startup files, and interrupt priority policy.
- [ ] Promote only selected chip-base records into exact-part target profiles. For
each chosen MCU, verify package pinouts/AFs, peripheral instances, clocks,
DMA/IRQ routes, memory regions, startup/linker data, errata, and a physical board
binding; leave catalogue-only and later-roadmap parts non-generatable.
- [ ] Implement and verify UHAL core timing, clock/reset, GPIO, IRQ, atomics, fault capture, backup registers, MPU, and cache hooks for that board.
- [ ] Decide the intended non-RTOS semantics for UOS mutexes, semaphores, queues, and events. The current bare-metal backend returns failure for unsupported synchronization rather than pretending these facilities work.
- [ ] Pin the exact FreeRTOS Kernel upstream commit and license-file paths in `third_party/freertos-kernel.lock.json`; populate only the reviewed common source list and selected architecture/compiler port profile in `uef-integration.json`.
- [ ] For every supported FreeRTOS profile, verify `FreeRTOSConfig.h`, ISR yield behavior, `configSTACK_DEPTH_TYPE`, static allocation sizes, task stack units, tick conversion, and interrupt-priority constraints on the selected MCU.
- [x] Resolve the live UEF checkout at generation time and fingerprint the selected source/template/manifest tree; do not maintain a packaged second UEF snapshot in `uef-gen`.

## Phase 1 — initial-release peripherals, storage, and middleware

- [ ] Implement UPAL DMA first, including source/destination direction, alignment, cache maintenance order, callback context, timeout, and cancellation behavior.
- [ ] Implement UART/SPI/I2C drivers and their board-instance descriptions; validate transfer-size bounds and DMA buffer ownership.
- [ ] Implement ADC/DAC/timer/HRTIM/CAN/USB/QSPI/flash/watchdog/RTC/comparator/op-amp/CORDIC/FMAC modules only on targets that advertise the required capability.
- [ ] Implement the Phase 1 UPAL SDMMC driver on the selected supported board: validate controller/DMA/pin configuration, identify and initialize supported SD/MMC card types, read CID/CSD and capacity, negotiate clock and bus width, and bound every initialization and recovery wait.
- [x] Keep SDMMC in the Phase 1 module/API map and preserve its public card, block-transfer, erase, and interrupt entry points as a target implementation scaffold.
- [ ] Reconcile the current 32-bit SDMMC block count/address fields with the stated SDXC support ceiling. Decide whether the initial release constrains supported card capacity or needs a versioned 64-bit LBA contract; make the FatFS `LBA_t` width/configuration part of the same decision.
- [ ] Complete SDMMC range, card-presence, alignment, DMA ownership, and cache checks. Implement asynchronous callbacks exactly once with documented ISR/task context, plus blocking read/write and erase paths that honor a single deadline and never return while hardware still owns caller buffers.
- [ ] Implement the optional Phase 1 UMID FatFS diskio adapter in `include/uef/umid/umid_fatfs.h` and `src/umid/fatfs.c`: pin the FatFS version/configuration contract, map `FF_VOLUMES` to registered SDMMC devices, validate LBA/count narrowing and card geometry, handle DMA alignment, and map every diskio command/error precisely.
- [x] Keep the FatFS adapter optional and consumer supplied, above UPAL SDMMC, with its module dependency and CMake opt-in declared.
- [x] Add an explicit Phase 1 FDCAN-FD API/header/source module, separate from classic CAN. The current implementation validates basic config/frame shapes then returns `UEF_NOT_SUPPORTED` without claiming hardware initialization.
- [ ] Keep filesystem mounting above UPAL. FatFS remains consumer-supplied and optional; verify `UEF_ENABLE_FATFS`, `UEF_FATFS_INCLUDE_DIR`, and the optional target against the firmware project package before enabling the adapter.
- [ ] Complete hardware validation for Phase 1 SDMMC identification, capacity, async/blocking read/write, erase, card detect, DMA/cache behavior, and FatFS mount/read/write paths, including card removal, failed-transfer recovery, and power-loss behavior.
- [ ] Implement UMID drivers with explicit units, calibration ownership, timestamp source, stale-sample policy, and propagated UPAL errors.
- [ ] Review UMID ring-buffer producer/consumer synchronization on every
  target. Its indices are currently `volatile` and use C11 fences, which is
  not by itself a portable atomic synchronization contract; choose lock-free
  atomics or a documented critical-section policy before relying on
  cross-context safety.
- [ ] Complete the generic UPROTO C APIs for DSHOT, CRSF, SBUS, PPM, MAVLink transport, and UAVCAN/DroneCAN transport; validate malformed/checksum-failed frames before publishing state and keep interrupt work bounded.
- [ ] Complete and review UEF-owned per-instance templates against the direct-use APIs. Each wrapper must compile from the selected node configuration, expose a clear application hook, and declare every required board action in its `registry/control_ir/manifest/` record indexed by `registry/control_ir/registry.json`.
- [ ] Decide whether MAVLink and UAVCAN definitions are vendored, imported, or provided through a documented external package. Keep upstream protocol definitions out of duplicate UEF copies.
- [ ] Define thread/ISR ownership for UMID logging and health registry access on each backend.

## Phase 2 — UCON algorithm contracts and generated specialization

- [x] Add UCON status/numeric foundations and generic PID and lead-lag implementations with thin registered UEF-owned templates.
- [x] Add the Part XVI catalogue and curated future roadmap proposals to the shared ControlIR manifest. Generate a family-organized include/source pair per unregistered entry with named lifecycle and algorithm operation functions, an individual TODO for each function, and fail-closed behavior.
- [x] Keep all `FIRST_PASS` entries plus planned common/project-required filters, drone estimation/control, QCWDRSSTC predictive-control, and PK/PD medical entries in the initial tree; keep other planned and all deferred entries under `future/`.
- [x] Register every placeholder as a scaffold-only UEF module and prevent uef-gen from selecting it for generated firmware.
- [x] Add per-algorithm `algorithm_unregistered`/direct-only/ambiguous-alias diagnostics to the generic uef-gen bridge; keep UCON equations and catalog ownership in UEF.
- [ ] Preserve supported numerical type variants (float32/float64/Q15/Q31), fixed dimensions, saturation rules, and static-memory guarantees in completed direct/generated implementations.
- [ ] For each catalogue entry, use its operation outline to define the typed API, then replace the type-erased scaffold with a reviewed implementation before setting `registered: true` or adding renderable outputs. Fill features, numeric dependencies, module dependencies, and output metadata only when the contract is defined; do not guess.
- [ ] Add template metadata for accepted ControlIR parameters, outputs, required modules/capabilities/numerics, fixed-shape constraints, and manual actions.
- [ ] Complete signal binding metadata as concrete UPAL methods become available; require explicit operation mappings wherever a peripheral has multiple possible reads/writes.
- [ ] Compare direct C and generated behavior for each registered algorithm, including reset, saturation, bounds, numeric failures, and state preservation.

## Phase 3 — packaging, examples, and hardware verification

- [ ] Add the examples listed in Part XIV as each prerequisite driver becomes real: UART DMA RX, SPI IMU, triggered ADC/HRTIM, DSHOT, CRSF, CAN/DroneCAN, and the integrated motor-control example.
- [ ] Add host checks for pure math, CRC, ring buffer, state machine, status strings, and generation manifests when testing is requested.
- [ ] Define host-simulation GPIO registry exhaustion behavior and either
  enforce single-threaded access or synchronize it before concurrent tests use
  simulated pins; add an explicit fault-injection hook if tests need to exercise
  UAPP recovery without crashing the process.
- [ ] Add hardware verification for UART loopback, ADC linearity, HRTIM dead-time, CAN loopback, and M7 cache coherency on the corresponding boards.
- [ ] Record memory budgets, worst-case execution time, timer resolution, interrupt latency, and DMA/cache assumptions for released targets.
- [ ] Add install/package rules for firmware projects separately from the NEXUS desktop installer; a generated firmware image must not assume Python is present on the target.

## Specification alignment notes

- Part XV §15.1 Phase 0 families and the Phase 1 STM32L4+ profile are incomplete family scaffolds only. The 65-entry roadmap includes later/catalogue-only families; none proves exact-part support.
- Part XV §15.2 places SDMMC, optional FatFS, and explicit FDCAN-FD in Phase 1. APIs/module surfaces exist; SDMMC, FatFS binding behavior, and FDCAN-FD target operations still require implementation and board validation.
- The provided module-manifest excerpt does not fully cover repository-only host simulation and module aliases. The checked-in manifest records those requirements plus the optional external FatFS dependency.
- The UEF module manifest declares FreeRTOS Kernel as a UEF-managed bundled dependency. This checkout contains its lock/profile scaffolds but no kernel source, commit pin, or verified profile yet; `uef-gen` fails closed until those are supplied.
- UEF owns `registry/control_ir/registry.json` as the shared UPROTO/UCON dispatch index. It points to separate node, alias, and algorithm records; PID and lead-lag UCON wrappers are registered. Remaining UCON entries each have an individual fail-closed declaration/definition and no outputs; do not infer implemented algorithm behavior from a catalogue or source file.


## UCON catalogue audit follow-up

- [ ] Reconcile CKF's third-degree spherical-radial rule with the stale Gauss-Hermite row and sketch in Part XVI.
- [ ] Resolve whether DEADBEAT_MPC is deferred or project-selected planned in Part 15.4 versus 16.6.
- [ ] Decide whether PID deadband/rate limiting are fields of the generic PID contract or separate graph blocks; update the registered API/templates accordingly.
- [ ] Define or explicitly defer the LLC resonant EKF plant, state/parameter vector, measurements, and operating envelope. The key currently gives no model.
- [ ] Review every generated operation list against its typed algorithm contract when implementation begins. Do not preserve outline functions that do not apply, and do not omit required functions simply because the scaffold currently lacks their inputs.
- [ ] Work through `ROADMAP_EXTENSIONS.md` by project need; a proposal remains unregistered until its UEF specification and numerical contract are complete.
- [ ] Read `UCON_CATALOGUE_AUDIT.md` before promoting any algorithm; it contains taxonomy corrections, implementation review, and the remaining specification conflicts.


## Full-framework audit follow-up

The repository-wide review is recorded in `UEF_SYSTEM_AUDIT.md`. Work in this
order before describing UEF as a supported embedded release:

1. Select and qualify one exact Cortex-M board. Implement the target UHAL
   operations and UPAL drivers against its reference manual, startup, clock,
   linker, DMA, interrupt, and cache contracts. The present family profiles
   are discovery scaffolds, not verified targets.
2. Pin and populate the FreeRTOS Kernel source/profile, or keep that backend
   unavailable. The lock currently has no revision or license-file list. The
   SafeRTOS operation outline remains unselectable until its consumer license
   and concrete vendor API are available.
3. Complete UPAL SDMMC and FDCAN-FD before marking Phase 1 hardware support
   ready; then validate FatFS diskio buffer alignment, cache maintenance,
   transfer lifetime, timeouts, card removal, and LBA range behavior.
4. Resolve the UMID and protocol concurrency contracts: the new ring buffer
   supports one producer and one consumer; init/flush need quiescence. Replace
   borrowed health-entry reads with `umid_health_entry_copy` in concurrent
   callers. Specify coherent publication/freshness for CRSF and SBUS state.
5. Implement the registered UPROTO APIs. PPM is the only completed protocol
   core; generated wrappers do not make the other transports functional. The
   future protocol outlines are documented but not selectable until typed
   profiles, transport requirements, and UEF-owned templates are reviewed.
6. Define a recoverable-fault sink for `uapp_fault_report`; its current body is
   deliberately a no-op because the API has no callback or storage contract.
7. Add the host and hardware validation suites listed in Part XIV, and add an
   installable CMake package/configuration that includes the metadata and
   templates required by downstream `uef-gen` use.
8. Reconcile the specification gaps called out in `UEF_SYSTEM_AUDIT.md`,
   especially the claimed complete architecture tiers, SafeRTOS backend, and
   UPROTO ISR-versus-deferred processing wording.
