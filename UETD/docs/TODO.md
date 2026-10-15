# UETD implementation TODO

This list distinguishes linkable source scaffolding from working behavior. Public runtime APIs now have definitions in their layer source files, but each `UETD_NOT_SUPPORTED` return, neutral getter result, or function-level TODO still marks behavior that must be implemented and reviewed. A function being present in a `.c` file does not mean the driver or algorithm works.


## Source map — where to continue implementation


Read a module's public header for its API and its paired `src/.../*.c` for
implementation progress. Do not promote a `SCAFFOLD_ONLY` registry/module entry
just because a source file exists; update its manifest readiness only after
the behavior and target dependencies are implemented and reviewed.

## Recommended implementation order and exit gates

Audit and prioritize the cross-layer portable backlog before choosing the
first hardware set. Resolve shared contracts and host-check the portable work
that determines first-pass requirements; the broader catalogue can remain
sequenced in the backlog. Choose multiple targets once their required
capability coverage and target-specific work are understood. Host/x86 work can
validate portable logic and configuration, but it is not evidence of embedded
timing or peripheral correctness.

### 0. Inventory work across layers and define the first-pass scope

1. Audit UCORE, UHAL, UOS, UMATH, UPAL, UMID, UPROTO, UAPP, UCON, uef-gen,
   build/package tooling, and documentation. Classify each item as portable,
   target-specific, dependency-blocked, scaffold-only, or qualification work.
2. Order the portable items by dependencies and project value; identify which
   hardware capabilities and behaviors the first-pass target set must cover.
3. Keep API presence, scaffold presence, implementation, and verified behavior
   as separate states in the inventory.

**Exit gate:** the backlog shows the work that can proceed without boards, the
work blocked on concrete hardware, and the capability coverage needed from a
multi-target first pass.

### 1. Stabilize and implement shared portable contracts

1. Reconcile public status/error behavior, integer widths, overflow rules,
   units, callback contexts, ownership, timeout semantics, and C/C++ inclusion
   across all public headers.
2. Resolve UOS clock, deadline, blocking, error-reporting, static-storage, and
   duration-conversion contracts in the detailed checklist under Phase 0.
3. Implement priority board-independent UMATH slices and typed UCON contracts
   or generic C algorithms where specifications are ready. Keep the broader
   catalogue sequenced by project need and unresolved algorithms fail-closed
   and unavailable to generation.
4. Complete portable UPROTO framing/parsing and other board-independent
   middleware behavior; keep transport binding and electrical behavior
   target-specific.
5. Add host/x86 checks for portable behavior, overflow boundaries, generated
   contracts, and C/C++ header compatibility. Do not use host results as
   evidence of hardware timing or peripheral behavior.

**Exit gate:** shared contracts and the portable work needed to scope the
first-pass targets have targeted host checks; remaining work is prioritized
and assigned to dependencies, hardware, or design decisions.

### 2. Select and verify a multi-target first pass

1. Select at least two concrete MCU/board configurations rather than treating
   one board as the entire first pass. Choose them to cover the required core,
   floating-point, memory/cache, DMA, peripheral, and toolchain differences.
2. Record the exact part/package, board revision, reference manual, datasheet,
   errata, CMSIS device headers, compiler/toolchain, startup files, linker
   script, clock tree, interrupt priorities, and physical test access for each.
3. Complete the selected exact-part chip records in place with verified
   resources, and bind each to a development-board record; leave incomplete
   and later-roadmap records registered but generation-unavailable.

**Exit gate:** every first-pass target has traceable source data and enough
verified resource information to implement and validate its declared scope.

### 3. Implement target foundations for the selected set

1. Implement UHAL clock/reset, monotonic timing, GPIO, interrupt control,
   atomics/critical sections, fault capture, and memory/cache hooks per target.
2. Implement each selected UOS backend against the standardized time and
   synchronization contract. Pin FreeRTOS source/profile data before enabling
   it; keep unverified or unsupported backends unavailable.
3. Verify static storage sizes/alignment, stack units, ISR rules, and capability
   macros against every selected RTOS port and architecture.

**Exit gate:** each selected target initializes and reports failures through
verified target operations; unsupported capabilities remain explicit.

### 4. Bring up peripheral paths and board-backed drivers

1. Define DMA ownership, alignment, cache maintenance, cancellation, callback
   context, and timeout semantics before enabling DMA-backed drivers.
2. Bring up a small board-backed smoke path on each target that claims it, then
   add peripheral drivers by verified capability and hardware family.
3. Complete SDMMC/FatFS and FDCAN-FD only on target profiles that declare the
   required resources, dependencies, and validation equipment.

**Exit gate:** every enabled driver has a target record, bounded failure
behavior, buffer/IRQ ownership rules, and hardware validation evidence.

### 5. Complete middleware, protocols, UCON generation, and applications

1. Complete sensor and registry synchronization, protocol state publication,
   bounded interrupt/deferred processing, and UAPP fault/recovery behavior.
2. Reconcile UCON catalogue conflicts, then promote typed algorithms and
   templates only after direct/generated semantics match.
3. Keep generated wrappers thin and record manual board/transport actions in
   UETD manifests consumed by uef-gen.

**Exit gate:** cross-layer errors reach callers; APIs, templates, and manifests
agree on execution context, ownership, timeout, and failure behavior.

### 6. Package and qualify supported configurations

1. Keep examples, CMake exports, module manifests, optional dependencies, and
   generated-project metadata synchronized.
2. Record memory use, execution time, timer resolution, interrupt latency,
   clock assumptions, and cache/DMA rules for each target/toolchain/backend.
3. Promote targets to supported only after their documented build and hardware
   validation procedures pass.

**Exit gate:** the release names the supported multi-target configurations and
separates host checks, compile-only examples, and hardware-validated behavior.

## Phase 0 — portable foundation and initial integration

### Cross-layer readiness before target selection

- [ ] Audit UCORE, UHAL, UOS, UMATH, UPAL, UMID, UPROTO, UAPP, UCON,
  uef-gen, build/package tooling, and documentation. For each item record
  whether it is portable, target-specific, dependency-blocked, scaffold-only,
  or awaiting verification; keep API presence separate from working behavior.
- [ ] Prioritize board-independent contracts and first-pass-relevant
  implementations before selecting the initial hardware set. Record the target
  capabilities needed to validate remaining hardware-specific work; keep the
  broader portable catalogue ordered by project need.
- [ ] Establish host/x86 checks for portable algorithms, configuration,
  overflow handling, and C/C++ public-header compatibility. Keep host results
  separate from embedded timing and peripheral qualification.
- [ ] Define first-pass UPAL API contracts for units, ranges, status reporting,
  buffer ownership, callback context, timeout, and cancellation. Keep pin,
  DMA-route, IRQ, cache, and peripheral-instance validation tied to selected
  targets.
- [ ] Complete the portable UPROTO codec/parser contracts for frame bounds,
  checksum and malformed-frame handling, and coherent state publication.
  Define bounded ISR/deferred-processing expectations separately from
  transport and board bindings.
- [ ] Prioritize the first-pass UCON algorithms and settle their equations,
  state/reset behavior, bounds, numeric-failure handling, and direct/generated
  parity before enabling their templates. Keep unresolved catalogue entries
  fail-closed.

### UOS timing, deadline, blocking, and storage contracts

- [ ] Make `uos_task_delay_until()` deadline updates transactional in every
  backend. Check `*next_us + period_us`, missed-period count increment, and
  missed-period multiplication before writing `*next_us`. Define what happens
  on unrecoverable overflow, leave the stored deadline unchanged on failure,
  and make failure observable through the API or a documented diagnostic path.
- [ ] Document how callers initialize the absolute `next_us` deadline, what a
  call does when the deadline is already due, and how missed periods are
  skipped. The resulting deadline must follow one drift-free policy across
  bare-metal and FreeRTOS implementations.
- [ ] Standardize the `uos_time_now()` monotonic epoch. The header currently
  says “since scheduler start,” bare-metal has no scheduler, and its backend
  returns `uetd_time_now_us()` directly. Specify the common epoch and its clock
  initialization/reset behavior, then make each backend conform.
- [ ] Define and document result semantics for every synchronization call.
  Distinguish unsupported operations from timeout, empty/full, invalid object,
  and success; review the current `bool`, zero-value, and `void` APIs and decide
  which need status returns or an explicit capability/error-reporting path.
  Document that callers must check object creation and operation results.
- [ ] Specify `UOS_WAIT_FOREVER` per duration-taking API and backend. Preserve
  zero-duration polling semantics, allow unbounded waits only where supported,
  and never forward the sentinel to a busy-wait delay or periodic deadline.
- [ ] Verify each opaque UOS storage size and alignment against the concrete
  task, mutex, semaphore, queue, and event control blocks for every supported
  RTOS port and target ABI. Ensure compile-time size/alignment checks cover all
  objects, including task storage, and keep unsupported backend combinations
  unavailable.
- [ ] Define overflow and argument rules for `UOS_US`, `UOS_MS`, and `UOS_S`.
  Their multiplications can overflow before the final cast; either provide
  checked conversion APIs or document/enforce valid input ranges.
- [ ] Verify `UETD_ALIGNAS` and all UOS public storage declarations in the
  supported C and C++ compilers/standards. Check the actual alignment of the
  resulting objects, not only macro expansion or header inclusion.
- [ ] Add UOS host unit tests for deadline calculations near `UINT64_MAX`,
  missed-period skipping and overflow preservation, clock initialization and
  epoch consistency, zero/finite/forever waits, duration-conversion boundaries,
  and storage size/alignment. Exercise shared logic on host and backend-specific
  behavior through appropriate backend tests.

### UMATH type configuration and geometry boundaries

- [x] Keep `types.h` limited to foundational UCORE types. UMATH scalar and
  accumulator policy lives in `include/umath/config.h`, with
  `umath_scalar_t` and independently configurable `umath_accumulator_t`.
- [x] Provide UETD CMake cache settings for the public scalar and accumulator;
  publish the selected compile definitions through `UETD::Core` so library and
  consumers compile public UMATH types consistently. The scalar affects ABI;
  widening the accumulator affects intermediate precision and execution cost.
- [x] Keep the existing `uetd_*f` limit helpers explicitly 32-bit and provide
  UMATH-typed scalar helpers. Implemented UMATH kernels use type-matched libm
  dispatch and accumulator-typed reductions/intermediates instead of silently
  routing configured values through 32-bit helpers.
- [x] Keep rigid-transform geometry separate from Lie algebra: `transform3.h`
  depends on quaternion/vector APIs, while SO(3)/SE(3) tangent operations live
  under `umath/lie/`. The SE(3) layer may depend on rigid transforms; the
  transform layer does not need to include Lie headers.
- [ ] Audit every implemented UMATH source and consumer for precision leaks:
  explicit 32-bit helper calls on configured values, implicit double promotion
  in float32 paths, narrowing before checks, and constants that bypass the
  scalar/accumulator conversion macros. Apply the same policy when statistics
  and other scaffold modules are implemented.
- [ ] Qualify the compile-time configuration combinations: float32 scalar with
  scalar-width accumulation, float32 scalar with float64 accumulation, and
  float64 scalar/accumulation. Verify that UETD library and consumer translation
  units receive identical definitions; record host/x86 and target-specific
  Cortex-M cost/precision results before recommending a configuration.
- [ ] Expose first-class UMATH scalar/accumulator settings in generated CMake
  projects, using the UETD cache names `UETD_UMATH_SCALAR_TYPE` and
  `UETD_UMATH_ACCUMULATOR_TYPE`. The current uef-gen `defines` configuration can
  already emit `UMATH_ACCUMULATOR_TYPE=uetd_f64_t`, but a bare CMake cache entry
  does not become a compiler definition; track the CMake mapping in
  [`uef-gen/TODO.md`](../../uef-gen/docs/TODO.md#3-project-assembly-and-provenance).

- [x] Promote the project-critical UMATH catalogue to Phase 0: automatic
  differentiation/Jacobians, quadrature and bounded ODE steps, FFT/windowing,
  SO(3)/SE(3), LU/LDLᵀ/QR/SVD/symmetric eigensystems, probability/statistics,
  least squares, splines, polynomial roots, and special functions. Individual
  headers, sources, and manifests exist; each remains fail-closed and unavailable
  to uef-gen.
- [ ] Implement the Phase 0 UMATH contracts in
  [`catalogues/UMATH_CATALOGUE.md`](catalogues/UMATH_CATALOGUE.md), starting with derivative/Jacobian
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
- [ ] Review the public headers against the updated UETD specification's Parts XIV and file-tree/API examples; keep include paths, status names, time units, and C/C++ inclusion behavior consistent.
- [x] Add six Phase 0 family-profile records for STM32G4xx, STM32F4xx, single-core STM32H7xx, STM32F7xx, STM32G0xx, and nRF52840; all remain incomplete and contain no invented exact-part data.
- [x] Add the Phase 1 STM32L4+ profile for the SDMMC roadmap. Keep the Phase 0 H7/F7 profiles marked as Phase 1 SDMMC target families, and expose Phase 0, Phase 1, and complete registered-profile queries.
- [x] Publish the UETD-owned 66-roadmap-family catalogue through api.json, including later spec targets and broader STM32/Espressif series as non-generatable, unverified roadmap metadata.
- [x] Add vendor-sourced exact-part chip records across the specification's named vendor groups and expose them to uef-gen list/show queries. Records can progress from identity and headline facts to complete per-chip resources; readiness is declared per record.
- [x] Remove duplicate Phase 0 family data from uef-gen; its target listing reads UETD’s API-declared registry and expands per-family/per-chip manifests. Snapshots fingerprint the registry/detail trees and include them in provenance.
- [ ] After the cross-layer portable backlog is prioritized, select a first-pass set of at least two concrete MCU/package/board configurations. Record each toolchain, CMSIS device header, clock setup, linker script, startup files, interrupt priorities, and capability coverage.
- [ ] Complete selected chip records in place with package pinouts/AFs, peripheral
instances, clocks, DMA/IRQ routes, memory regions, startup/linker data, and
errata; add a separate physical development-board record with verified build
and flash settings. Keep incomplete and later-roadmap records generation-unavailable.
- [ ] Implement and verify UHAL core timing, clock/reset, GPIO, IRQ, atomics, fault capture, backup registers, MPU, and cache hooks for each selected target.
- [ ] Resolve UOS bare-metal and RTOS semantics using the detailed UOS timing/storage checklist above. The current bare-metal backend fails or returns neutral values for unsupported synchronization rather than providing RTOS-style blocking.
- [ ] Checksum-compare the vendored FreeRTOS Kernel 11.3.1 source against upstream commit `3a22924e0a9ddbbc8b0758881c33b3422a5cc20d`; then add and validate only the port profiles for explicitly selected MCU/board targets. The lock and generic ARM Cortex-M4F GCC source profile are now present under `third_party/FreeRTOS-Kernel-11.3.1/`.
- [ ] For every supported FreeRTOS profile, verify `FreeRTOSConfig.h`, ISR yield behavior, `configSTACK_DEPTH_TYPE`, static allocation sizes/alignment, task stack units, tick conversion, and interrupt-priority constraints on each selected MCU.
- [x] Resolve the live UETD checkout at generation time and fingerprint the selected source/template/manifest tree; do not maintain a packaged second UETD snapshot in `uef-gen`.

## Phase 1 — initial-release peripherals, storage, and middleware

- [ ] Implement UPAL DMA first, including source/destination direction, alignment, cache maintenance order, callback context, timeout, and cancellation behavior.
- [ ] Implement UART/SPI/I2C drivers and their board-instance descriptions; validate transfer-size bounds and DMA buffer ownership.
- [ ] Implement ADC/DAC/timer/HRTIM/CAN/USB/QSPI/flash/watchdog/RTC/comparator/op-amp/CORDIC/FMAC modules only on targets that advertise the required capability.
- [ ] Implement the Phase 1 UPAL SDMMC driver on each first-pass board selected for storage support: validate controller/DMA/pin configuration, identify and initialize supported SD/MMC card types, read CID/CSD and capacity, negotiate clock and bus width, and bound every initialization and recovery wait.
- [x] Keep SDMMC in the Phase 1 module/API map and preserve its public card, block-transfer, erase, and interrupt entry points as a target implementation scaffold.
- [ ] Reconcile the current 32-bit SDMMC block count/address fields with the stated SDXC support ceiling. Decide whether the initial release constrains supported card capacity or needs a versioned 64-bit LBA contract; make the FatFS `LBA_t` width/configuration part of the same decision.
- [ ] Complete SDMMC range, card-presence, alignment, DMA ownership, and cache checks. Implement asynchronous callbacks exactly once with documented ISR/task context, plus blocking read/write and erase paths that honor a single deadline and never return while hardware still owns caller buffers.
- [ ] Implement the optional Phase 1 UMID FatFS diskio adapter in `include/umid/fatfs.h` and `src/umid/fatfs.c`: pin the FatFS version/configuration contract, map `FF_VOLUMES` to registered SDMMC devices, validate LBA/count narrowing and card geometry, handle DMA alignment, and map every diskio command/error precisely.
- [x] Keep the FatFS adapter optional and consumer supplied, above UPAL SDMMC, with its module dependency and CMake opt-in declared.
- [x] Add an explicit Phase 1 FDCAN-FD API/header/source module, separate from classic CAN. The current implementation validates basic config/frame shapes then returns `UETD_NOT_SUPPORTED` without claiming hardware initialization.
- [ ] Keep filesystem mounting above UPAL. FatFS remains consumer-supplied and optional; verify `UETD_ENABLE_FATFS`, `UETD_FATFS_INCLUDE_DIR`, and the optional target against the firmware project package before enabling the adapter.
- [ ] Complete hardware validation for Phase 1 SDMMC identification, capacity, async/blocking read/write, erase, card detect, DMA/cache behavior, and FatFS mount/read/write paths, including card removal, failed-transfer recovery, and power-loss behavior.
- [ ] Implement the UMID driver scaffolds under `include/umid/drivers/` and `src/umid/drivers/` with explicit units, calibration ownership, timestamp source, stale-sample policy, and propagated UPAL errors. Add concrete device drivers by hand; do not build a per-device registry or claim unimplemented devices as supported.
- [ ] Qualify UMID ring-buffer producer/consumer synchronization on every
  target. The implementation uses UHAL atomic index loads/stores and assumes
  one producer and one consumer; verify ordering for ISR/task and host use, and
  require quiescence during initialization, reset, and flush. Do not infer
  multi-producer safety from the SPSC contract.
- [ ] Complete the generic UPROTO C APIs for DSHOT, CRSF, SBUS, PPM, MAVLink transport, and UAVCAN/DroneCAN transport; validate malformed/checksum-failed frames before publishing state and keep interrupt work bounded.
- [ ] Decide whether MAVLink and UAVCAN definitions are vendored, imported, or provided through a documented external package. Keep upstream protocol definitions out of duplicate UETD copies.
- [x] Define thread/ISR ownership for UMID logging and health registry access: health reports/snapshots use UHAL critical sections; UMID_LOG is task/main-only, serializes same-core producers, and has one task-context drain consumer. Cross-core logging requires per-core instances or a future shared queue.

## Phase 2 — UCON algorithm contracts and generated specialization

Algorithm contract work and board-independent generic C implementations can
proceed during the portable host phase; do not wait for board selection for
work that has no hardware dependency. Defer signal bindings, target capability
requirements, and hardware-specific template actions until their UPAL/target
contracts exist.

- [x] Add UCON status/numeric foundations and generic PID and lead-lag implementations with thin registered UETD-owned templates.
- [x] Keep all `FIRST_PASS` entries plus planned common/project-required filters, drone estimation/control, QCWDRSSTC predictive-control, and PK/PD medical entries in the initial tree; keep other planned and all deferred entries under `future/`.
- [x] Register every placeholder as a scaffold-only UETD module and prevent uef-gen from selecting it for generated firmware.
- [x] Add per-algorithm `algorithm_unregistered`/direct-only/ambiguous-alias diagnostics to the generic uef-gen bridge; keep UCON equations and catalog ownership in UETD.
- [ ] Preserve supported numerical type variants (float32/float64/Q15/Q31), fixed dimensions, saturation rules, and static-memory guarantees in completed direct/generated implementations.
- [ ] For each catalogue entry, use its operation outline to define the typed API, then replace the type-erased scaffold with a reviewed implementation before setting `registered: true` or adding renderable outputs. Fill features, numeric dependencies, module dependencies, and output metadata only when the contract is defined; do not guess.
- [ ] Complete signal binding metadata as concrete UPAL methods become available; require explicit operation mappings wherever a peripheral has multiple possible reads/writes.
- [ ] Compare direct C and generated behavior for each registered algorithm, including reset, saturation, bounds, numeric failures, and state preservation.

## Phase 3 — packaging, examples, and hardware verification

- [ ] Add the examples listed in Part XIV as each prerequisite driver becomes real: UART DMA RX, SPI IMU, triggered ADC/HRTIM, DSHOT, CRSF, CAN/DroneCAN, and the integrated motor-control example.
- [ ] Maintain host tests for portable math, UOS time/deadline/overflow/storage contracts, CRC, ring buffer, state machine, status strings, and generation manifests; keep target timing/peripheral checks in board-specific validation.
- [ ] Define host-simulation GPIO registry exhaustion behavior and either
  enforce single-threaded access or synchronize it before concurrent tests use
  simulated pins; add an explicit fault-injection hook if tests need to exercise
  UAPP recovery without crashing the process.
- [ ] Add hardware verification for UART loopback, ADC linearity, HRTIM dead-time, CAN loopback, and M7 cache coherency on the corresponding boards.
- [ ] Record memory budgets, worst-case execution time, timer resolution, interrupt latency, and DMA/cache assumptions for released targets.
- [ ] Add install/package rules for firmware projects separately from the NEXUS desktop installer; a generated firmware image must not assume Python is present on the target.

## Specification alignment notes

- Part XV §15.1 Phase 0 families and the Phase 1 STM32L4+ profile are incomplete family scaffolds only. The family index has 66 roadmap entries plus catalogue-only RP2350 and an internal pipeline fixture (68 total); none proves exact-part support.
- Part XV §15.2 places SDMMC, optional FatFS, and explicit FDCAN-FD in Phase 1. APIs/module surfaces exist; SDMMC, FatFS binding behavior, and FDCAN-FD target operations still require implementation and board validation.
- The provided module-manifest excerpt does not fully cover repository-only host simulation and module aliases. The checked-in manifest records those requirements plus the optional external FatFS dependency.
- The UETD module manifest declares FreeRTOS Kernel as a UETD-managed bundled dependency. Version 11.3.1 source, upstream tag/commit lock, license path, and a generic ARM Cortex-M4F GCC source profile are now staged. The source copy is not checksum-verified, and `uef-gen` still fails closed because no exact target is generation-available.


## UCON catalogue audit follow-up

- [ ] Reconcile CKF's third-degree spherical-radial rule with the stale Gauss-Hermite row and sketch in Part XVI.
- [ ] Resolve whether DEADBEAT_MPC is deferred or project-selected planned in Part 15.4 versus 16.6.
- [ ] Decide whether PID deadband/rate limiting are fields of the generic PID contract or separate graph blocks; update the registered API/templates accordingly.
- [ ] Define or explicitly defer the LLC resonant EKF plant, state/parameter vector, measurements, and operating envelope. The key currently gives no model.
- [ ] Review every generated operation list against its typed algorithm contract when implementation begins. Do not preserve outline functions that do not apply, and do not omit required functions simply because the scaffold currently lacks their inputs.
- [ ] Work through `roadmap/ROADMAP_EXTENSIONS.md` by project need; a proposal remains unregistered until its UETD specification and numerical contract are complete.
- [ ] Read `audits/UCON_CATALOGUE_AUDIT.md` before promoting any algorithm; it contains taxonomy corrections, implementation review, and the remaining specification conflicts.


## Full-framework audit follow-up

The implementation-status review is recorded in `audits/UETD_SYSTEM_AUDIT.md`; the
broader capability/readiness and source-equivalence review is in
`audits/UETD_EQUIVALENCE_AND_CATALOGUE_GAP_AUDIT.md`. Work in this order before
describing UETD as a supported embedded release:

1. After completing the portable cross-layer audit, select and qualify an
   initial set of at least two exact Cortex-M board/MCU configurations.
   Implement target UHAL operations and UPAL drivers against each target's
   reference manual, startup, clock, linker, DMA, interrupt, and cache
   contracts. The present family profiles are discovery scaffolds, not
   verified targets.
2. Verify the vendored FreeRTOS Kernel 11.3.1 tree against its locked upstream
   commit; add only board-backed profiles with validated configuration, startup,
   interrupt, and linker contracts. The SafeRTOS operation outline remains
   unselectable until its consumer license and concrete vendor API are available.
3. Complete UPAL SDMMC and FDCAN-FD before marking Phase 1 hardware support
   ready; then validate FatFS diskio buffer alignment, cache maintenance,
   transfer lifetime, timeouts, card removal, and LBA range behavior.
4. Resolve the UMID and protocol concurrency contracts: the ring buffer
   supports one producer and one consumer; init/flush need quiescence. Use
   `umid_health_entry_copy` for concurrent reads; supervisor stale reporting
   now uses timestamps. Specify coherent publication/freshness for CRSF and
   SBUS state and qualify critical-section behavior on each target/core.
5. Implement the registered UPROTO APIs. PPM is the only completed protocol
   core; generated wrappers do not make the other transports functional. The
   future protocol outlines are documented but not selectable until typed
   profiles, transport requirements, and UETD-owned templates are reviewed.
6. Recoverable fault reports now enter a fixed RAM queue and can be dispatched
   from task context. Implement the retained crash-history contract in
   `include/uapp/future/FAULT_HISTORY.md`; do not write filesystems from a
   fault handler.
7. Add the host and hardware validation suites listed in Part XIV, and add an
   installable CMake package/configuration that includes the metadata and
   templates required by downstream `uef-gen` use.
8. Reconcile the specification gaps called out in `audits/UETD_SYSTEM_AUDIT.md`,
   especially the claimed complete architecture tiers, SafeRTOS backend, and
   UPROTO ISR-versus-deferred processing wording.

## Accepted catalogue expansion backlog

`registry/future/registry.json` is the UETD-owned index for accepted design work
that is not yet selectable. Its entries link to a human-readable contract and,
where useful, a type-level C outline. `uef-gen.registry.load_future_catalogue`
loads the same index for planning and validates that linked files exist. Do not
copy these items into generation-ready module selection until the APIs,
backends and evidence are promoted.

- [ ] Promote the UPAL operation extensions from `include/upal/future/`: UART framing/half-duplex, timer capture delivery, synchronized ADC/PWM groups, USB CDC/HID, and Ethernet MAC/PHY.
- [ ] Implement UMID storage, calibration envelope, binary blackbox, and PLC process-image services from `include/umid/future/`; keep sensor-specific driver algorithms user-authored.
- [ ] Implement retained crash history and secure-update profiles from `include/uapp/future/` with exact target boot/retention capabilities.
- [ ] Promote Modbus RTU/TCP and CANopen only after their bounded profiles and PLC process-image mappings are reviewed; see `roadmap/UPROTO_ROADMAP.md` and the future catalogue.
- [ ] Complete `.uef`/`.uefs` type support from `../uef-gen/src/uef_gen/language/frontends/project/TYPE_INTERCHANGE.md` end to end through parser, validation, lowering, generated C and editor metadata.
- [ ] Implement system external-artifact and qualification evidence records using the schemas in `registry/future/` and the system-language contract under uef-gen.
- [ ] Add authenticated signing/update orchestration only with matching bootloader capabilities and the power-loss/rollback evidence required by the secure-update contracts.
