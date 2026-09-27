# UEF implementation TODO

This list distinguishes linkable source scaffolding from working behavior. Public runtime APIs now have definitions in their layer source files, but each `UEF_NOT_SUPPORTED` return, neutral getter result, or function-level TODO still marks behavior that must be implemented and reviewed. A function being present in a `.c` file does not mean the driver or algorithm works.

## Phase 0 — portable foundation and initial integration

- [ ] Review the public headers against the updated UEF specification's Parts XIV and file-tree/API examples; keep include paths, status names, time units, and C/C++ inclusion behavior consistent.
- [ ] Select and document the first concrete Cortex-M board, toolchain, CMSIS device header, clock setup, linker script, startup files, and interrupt priority policy.
- [ ] Implement and verify UHAL core timing, clock/reset, GPIO, IRQ, atomics, fault capture, backup registers, MPU, and cache hooks for that board.
- [ ] Replace the Phase 0 SDMMC scaffolds in `include/uef/upal/upal_sdmmc.h` and `src/upal/sdmmc/sdmmc.c` with a board-backed driver: identify SD v1/v2, SDHC/SDXC, MMC/eMMC as supported; read CID/CSD and capacity; negotiate clock and 1-/4-bit bus width; validate overflow-safe block ranges; enforce DMA/cache alignment and ownership; handle card removal; and bound initialization, transfer, erase, and recovery waits.
- [ ] Reconcile the current 32-bit SDMMC block count/address fields with the stated SDXC support ceiling. Before claiming support for the full SDXC capacity range, decide whether the public API needs a versioned 64-bit LBA contract and make FatFS `LBA_t` width/configuration part of that decision.
- [ ] Complete both SDMMC transfer paths: async DMA callbacks must fire exactly once with a documented ISR/task context, while blocking read/write must honor one deadline and guarantee hardware no longer owns caller buffers when returning. Implement controller and DMA handlers with bounded ISR work.
- [ ] Implement the optional UMID FatFS diskio binding in `include/uef/umid/umid_fatfs.h` and `src/umid/fatfs.c`: pin the FatFS version/configuration contract, map `FF_VOLUMES` to registered SDMMC devices, validate LBA/count narrowing and card geometry, handle DMA alignment, and map every diskio command/error precisely.
- [ ] Keep filesystem mounting above UPAL; FatFS is an external dependency used by UMID, not part of the raw SDMMC transfer contract. Verify `UEF_ENABLE_FATFS`, `UEF_FATFS_INCLUDE_DIR`, and the optional target against the firmware project's FatFS package.
- [ ] Decide the intended non-RTOS semantics for UOS mutexes, semaphores, queues, and events. The current bare-metal backend returns failure for unsupported synchronization rather than pretending these facilities work.
- [ ] Confirm FreeRTOS kernel version, port, `FreeRTOSConfig.h`, ISR yield macro, `configSTACK_DEPTH_TYPE` compatibility, and static object sizes for the selected MCU.
- [ ] Refresh `uef-gen`'s packaged UEF snapshot and SHA-256 metadata after each deliberate UEF release.

## Phase 1 — peripheral and middleware implementations

- [ ] Implement UPAL DMA first, including source/destination direction, alignment, cache maintenance order, callback context, timeout, and cancellation behavior.
- [ ] Implement UART/SPI/I2C drivers and their board-instance descriptions; validate transfer-size bounds and DMA buffer ownership.
- [ ] Implement ADC/DAC/timer/HRTIM/CAN/USB/QSPI/flash/watchdog/RTC/comparator/op-amp/CORDIC/FMAC modules only on targets that advertise the required capability.
- [ ] Complete hardware validation for Phase 0 SDMMC identification, capacity, async/blocking read/write, erase, card detect, DMA/cache behavior, and FatFS mount/read/write paths, including card removal, failed-transfer recovery, and power-loss behavior.
- [ ] Implement UMID drivers with explicit units, calibration ownership, timestamp source, stale-sample policy, and propagated UPAL errors.
- [ ] Review UMID ring-buffer producer/consumer synchronization on every
  target. Its indices are currently `volatile` and use C11 fences, which is
  not by itself a portable atomic synchronization contract; choose lock-free
  atomics or a documented critical-section policy before relying on
  cross-context safety.
- [ ] Implement UPROTO framing and parsers with bounded buffers, checksum validation before publishing state, and bounded interrupt-side work.
- [ ] Decide whether MAVLink and UAVCAN definitions are vendored, imported, or provided through a documented external package. Keep upstream protocol definitions out of duplicate UEF copies.
- [ ] Define thread/ISR ownership for UMID logging and health registry access on each backend.

## Phase 2 — UCON template library and generation

- [ ] Define and place UCON in UEF: add reviewed algorithm bodies and reusable assets under the UEF tree only after the designs are ready; a family is implemented only when its algorithm body, public types, metadata, selection/assembly path, and numerical bounds have been reviewed together. Until then, keep the catalogue clearly marked as planned and treat the uef-gen UCON code as transitional rather than copying it as a second implementation.
- [ ] Preserve numerical type variants (float32/float64/Q15/Q31), fixed dimensions, saturation rules, and static-memory guarantees in generated artifacts.
- [ ] Add template metadata for supported parameters, generated files, required modules/capabilities, and compile-time shape constraints.
- [ ] Implement the uef-gen adapter that selects and assembles UEF-owned UCON assets from the versioned UEF contract. Retire or narrow generator-owned `TemplateSet` extension duties so they cannot become the source of truth for algorithm bodies.
- [ ] Complete the UCON signal-binding API database as concrete UPAL methods become implemented; require an explicit operation where one peripheral exposes multiple candidate reads/writes.
- [ ] Define UEF-owned UCON public scalar/state types and verify that any generated HAL bindings, ISR/task execution wrappers, and project configuration use the same names and target arithmetic.

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

- The updated Part XV §15.2 places SDMMC in Phase 0 and the file tree includes the FatFS UMID binding. The source driver and binding are still implementation scaffolds.
- The provided module-manifest excerpt does not fully cover repository-only host simulation and module aliases. The checked-in manifest records those requirements plus the optional external FatFS dependency.
- The manifest excerpt lists FreeRTOS kernel paths beneath `third_party`; this UEF checkout does not vendor the kernel. The local manifest treats it as a firmware-project dependency.
- UEF is the intended owner of UCON, but the algorithm designs and source transfer are not complete; `templates/ucon` and `include/uef/ucon` remain documentation-only while the public types and uef-gen handoff are being defined.
