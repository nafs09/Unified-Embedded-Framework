# Unified Embedded Framework (UEF)

UEF is the standalone, C-first embedded runtime described by the **UEF Architecture and API Specification V1.1**. It has no NEXUS or Python runtime dependency. `uef-gen` is a separate project-construction tool; NEXUS can invoke it over the documented JSON subprocess contract.

Every function declared by the current public runtime headers has a matching source definition, so the tree has a linkable starting point for each module. Many definitions are deliberately nonfunctional scaffolds: status-returning functions return `UEF_NOT_SUPPORTED`, value-returning functions return a neutral value, and each body has a concrete implementation TODO. Treat those as unimplemented behavior, not as working drivers. The portable UCORE routines, host-simulation UHAL pieces, lifecycle/supervisor pieces, basic ring-buffer byte-copy operations, and selected UOS services have behavior; the ring buffer's cross-context memory-ordering contract still needs review. The TODO and architecture-status documents distinguish working behavior from placeholders.

## Layer map

| Layer | Owns | Public headers |
|---|---|---|
| UCORE | Scalar types, status, time, assertions, limits, math | `include/uef/ucore` |
| UHAL | CPU, clock, GPIO, IRQ, atomics, cache, faults, memory | `include/uef/uhal` |
| UPAL | DMA and peripheral drivers, including the Phase 0 raw SDMMC block boundary | `include/uef/upal` |
| UOS | Microsecond time, static task and synchronization APIs | `include/uef/uos` |
| UMID | Sensors, logging, health, ring buffer, and optional FatFS diskio binding | `include/uef/umid` |
| UPROTO | DSHOT, CRSF, SBUS, MAVLink, and UAVCAN bindings | `include/uef/uproto` |
| UAPP | Component lifecycle, state machine, supervisor, fault handling | `include/uef/uapp` |
| UCON | Intended UEF-owned control and estimation algorithms/assets; uef-gen will select and assemble the needed UEF content | The current catalogue is intentionally sparse; algorithms and the generator handoff are not yet specified |

`include/uef/uef.h` is the public umbrella for the current runtime scaffold. Its UCON surface will be defined with the UEF-owned algorithm designs and handoff contract.

## Source organization

- `src/ucore`: shared status, timing, assertion, and math implementations.
- `src/uhal/arm_cm`: Cortex-M target work isolated from portable modules.
- `src/uhal/x86`: host simulation behavior; host clocks and GPIO are not hardware accurate.
- `src/upal/<peripheral>`: one implementation file per peripheral contract.
- `src/uos/{baremetal,freertos}`: mutually selected scheduler backends.
- `src/umid`, `src/uproto`, `src/uapp`: one source file per named module.
- `templates/ucon` and `include/uef/ucon`: currently documentation-only placeholders. UEF is the intended home for UCON algorithms and reusable assets, but the transfer/design work and uef-gen consumption contract are not complete. Treat generator-side UCON code as transitional; do not copy it into UEF as a substitute for the algorithm designs.

The source files define the public API even when hardware or algorithm behavior is pending. A status of `UEF_NOT_SUPPORTED` or a neutral getter value means the implementation has not been connected; do not treat it as a successful no-op. Cortex-M fault handlers fail-stop until exception-frame capture and persistent fault reporting are implemented.

## Build

Host simulation with the single-loop bare-metal UOS backend:

```powershell
cmake -S . -B out/build/host -DUEF_TARGET=HOST -DUEF_UOS_BACKEND=BAREMETAL -DUEF_BUILD_EXAMPLE=ON
cmake --build out/build/host
```

For a Cortex-M build, configure `UEF_TARGET=CORTEX_M`, `UEF_CMSIS_INCLUDE_DIR`, and `UEF_CMSIS_DEVICE_HEADER` for the selected vendor/device package. The target modules still need a verified board implementation and linker/startup integration before they can drive hardware.

For FreeRTOS, set `UEF_UOS_BACKEND=FREERTOS`, `UEF_FREERTOS_INCLUDE_DIR`, and, when available, `UEF_FREERTOS_TARGET` to the target that supplies the kernel and port. UEF does not vendor the FreeRTOS kernel here.

## SDMMC Phase 0 and FatFS boundary

`upal_sdmmc` is the Phase 0 hardware transfer boundary: it describes card identification, capacity/CID, 512-byte block transfers, DMA callbacks, blocking operations, erase, and controller/DMA handlers. The public block address/count types currently follow the specification's 32-bit contract. The functions are scaffolds until a selected board's controller and DMA/cache behavior are implemented. Filesystem operations stay above UPAL; the optional UMID `umid_fatfs` adapter supplies FatFS diskio callbacks without vendoring FatFS itself.

Enable the binding with `UEF_ENABLE_FATFS=ON` and set `UEF_FATFS_INCLUDE_DIR` to the external FatFS headers. The firmware project supplies the FatFS version, `ffconf.h`, and any FatFS target/library. `UEF_FATFS_TARGET` can name an existing CMake target when one is available.

## Manifests and generated snapshots

`uef_modules.json` describes granular source dependencies for `uef-gen`, including host/ARM UHAL selection, Phase 0 SDMMC, and the optional FatFS binding's external dependency. `uef_api.json` contains the UPAL signal-binding operations listed by the specification. Keep both synchronized with the headers.

UEF is the source of truth. `uef-gen/src/uef_gen/uef_source` is a content-verified release snapshot, not a second implementation. Refresh it and its SHA-256 metadata together when publishing a UEF version.
