# UEF source architecture and implementation status

This document maps the V1.1 public contract to the source tree and calls out what is still a scaffold. The attached `UEF_Specification.md` remains the architecture reference; this file records repository-specific decisions and implementation progress.

## Dependency direction

```text
UCORE
  └── UHAL (one target backend selected per build)
       ├── UPAL (peripheral modules)
       ├── UOS (one scheduler backend selected per build)
       └── UMID → UPROTO → UAPP

UEF is intended to own UCON algorithms and reusable assets. uef-gen will read
the UEF manifests/contracts and select or assemble the UEF content needed by a
project. NEXUS is an authoring host only; firmware does not link NEXUS or Python.
```

Portable code must not include vendor device headers. `include/uef/uhal/target.h` is the boundary for CMSIS types and core intrinsics. Device register mappings belong under `src/uhal/arm_cm` or in a later family-specific target directory.

## Current source status

| Area | Current state | Remaining work |
|---|---|---|
| UCORE | Status strings, CRC/integer helpers, scalar math reference routines, cycle-based time helpers, fail-fast assertion | Replace scalar math with measured target acceleration where useful; validate target timer behavior |
| UHAL host | Host counter, simulated clock/GPIO, atomic operations, backup-register stand-in, no-op interrupt/cache boundary | Host GPIO registry is single-threaded simulation state; no embedded timing claim |
| UHAL Cortex-M | Each public contract has an explicit source definition behind the target-header boundary | Device-specific NVIC, clock, GPIO, cache, fault, MPU, backup-register, and DWT implementation; the current definitions are fail-safe/neutral scaffolds |
| UPAL | Every declared peripheral operation has a linkable definition, including raw SDMMC block operations | Register-level drivers, DMA/cache rules, board configuration, and peripheral-specific error handling; pending status-returning calls return `UEF_NOT_SUPPORTED` |
| UOS bare-metal | Time and startup-delay calls work; unsupported scheduler/locking objects fail explicitly | Decide whether a cooperative scheduler is needed or use FreeRTOS for concurrent services |
| UOS FreeRTOS | Static task, mutex, semaphore, queue, event, tick conversion, and ISR adapter code | Integrate a pinned kernel/port and verify its static object sizes/configuration on each target |
| UMID | Ring buffer and bounded health registry have implementations; each sensor/log API is defined; optional FatFS diskio adapter is scaffolded | Sensor sampling, logging transport, calibration, unit conversion, stale-data policy, and the FatFS callback implementation |
| UPROTO | Per-protocol API functions are defined in separate source files | Framing, checksum, parser, transport, and upstream dialect integration |
| UAPP | Lifecycle, transition table, threshold supervisor, and fault log path have implementations | Board-specific recovery policy, exception PC/LR capture, and restart semantics |
| UCON | Intended UEF-owned control/estimation algorithms and reusable assets; the current catalogue is only a placeholder | Define algorithm bodies, public types, metadata, and the uef-gen selection/assembly contract; do not infer equations, dimensions, or saturation rules from filenames |

## UCON transfer status

The current project direction assigns UCON to UEF, with uef-gen consuming UEF's eventual selection and assembly contract. The attached UEF and uef-gen specifications have not yet been revised to that direction and still describe UCON as generator-owned/project-generated. This checkout therefore keeps `templates/ucon` and `include/uef/ucon` documentation-only while the algorithm designs, public types, metadata, and handoff are defined. Treat the existing generator-side UCON material as transitional; do not claim a family is implemented from its catalogue entry.

## SDMMC and FatFS status

The updated UEF specification places SDMMC in Phase 0 and defines card identification, card information, asynchronous DMA transfers, blocking transfers, erase, and interrupt handlers. The local API now matches that shape, but the function bodies remain explicit `UEF_NOT_SUPPORTED` scaffolds until a board driver is selected. FatFS belongs above UPAL in the optional UMID `umid_fatfs` adapter; FatFS itself remains an external firmware dependency and is enabled through the CMake option.

## Manifest interpretation

The UEF manifest excerpt lists Cortex-M UHAL and granular peripheral modules but does not fully cover repository-only host simulation and module aliases. The checked-in manifest adds those items, records SDMMC and the external FatFS dependency, preserves architecture/backend selection, and uses the capability metadata consumed by `uef-gen`.
