# UETD source architecture and readiness

This document maps UETD's C11 layers, generation boundary, and current implementation state. The architecture contract is in the UETD specification; this file describes the checked-out code and distinguishes usable generic behavior from target scaffolds.

## Ownership and dependency direction

```text
UCORE (shared storage types and status)
  ├── UMATH (shared numerical primitives)
  └── UHAL (one selected target backend)
       ├── UPAL (peripheral drivers)
       ├── UOS (one selected scheduler backend)
       └── UMID → UPROTO → UAPP
                    └────────── UCON consumes UMATH and may use other public UETD services
```

UETD owns reusable firmware behavior, algorithm definitions, public types, module/API metadata, and target data. `uef-gen` reads the live UETD registries and manifests to resolve hardware and module dependencies and assemble a project scaffold. Generated firmware does not depend on Python.

Portable source must not include vendor device headers. `UETD_TARGET` selects one UHAL backend; `UETD_UOS_BACKEND` selects one operating-system backend. Concrete part, package, board, and clock details belong in target data and generated board configuration, not in generic middleware or algorithm code.

## Main source map

| Concern | Public surface | Implementation | Current state |
|---|---|---|---|
| Shared types/status/time | `include/ucore/` | `src/ucore/` | Portable base APIs exist; host execution is not MCU qualification. |
| Shared mathematics | `include/umath/` | `src/umath/` | Shared scalar/configuration headers stay at the root; vectors, geometry, matrix core/factorizations, polynomial core/roots, and derivative methods are grouped by domain. Only later catalogue items remain below `future/`. |
| UHAL | `include/uhal/` | `src/uhal/x86/`, `src/uhal/arm_cm/` | Generic contracts and backends exist; exact board startup and target-family profile data remain incomplete. |
| Phase 0/1 profile registry | `include/uhal/target_profiles.h` and per-family `profile.h` files | `src/uhal/target_profiles.c` and per-family `profile.c` files | Six Phase 0 and one Phase 1 profile records are queryable and incomplete; no exact part is certified. |
| Target-family roadmap | `api.json` declaration | `registry/targets/families/registry.json` plus `manifest/` | Canonical 68-entry index: 66 roadmap families, catalogue-only RP2350, and one internal pipeline fixture. These records do not imply exact-part support or generation readiness. |
| Exact-part chip records | `api.json` declaration | `registry/targets/chips/registry.json` plus `manifest/` | UETD is the source for exact-part facts; `uef-gen` materializes records into its resolver, while `generation_available` gates physical generation until the required maps are ready. |
| Development-board records | `api.json` declaration | `registry/targets/development_boards/registry.json` plus `manifest/` | Board clocks, reserved pins, inline module facts, build settings, and flash settings live with each board record. |
| UETD modules | `api.json` declaration | `registry/modules/registry.json` plus `manifest/` | Module dependency and selection metadata is indexed by name; each module detail is in its own file. |
| UPAL | `include/upal/` | `src/upal/<peripheral>/` | APIs are grouped by peripheral. Most hardware operations remain unsupported until a target backend is implemented. |
| UOS | `include/uos/` | `src/uos/baremetal/`, `src/uos/freertos/` | Backend boundary exists. FreeRTOS source/profile pinning is incomplete and fails closed. |
| UMID | `include/umid/` (`drivers/`) | `src/umid/` (`drivers/`) | Sensor and measurement driver scaffolds are grouped under `drivers/`; physical availability follows UPAL and board integration. Driver implementations are added manually without a per-device registry. |
| UACTUATOR | `registry/modules/manifest/uactuator/` | Target-specific driver sources remain user-authored | The `h_bridge` registry entry is a draft language-facing actuator identity; it supplies no target pin map, gate-driver code, safe-state definition, or generation-ready route. |
| UPROTO | `include/uproto/` | `src/uproto/` | Initial protocol APIs exist; transport, parser, upstream dialect/stack, and hardware work remains declared/manual where applicable. |
| UAPP | `include/uapp/` | `src/uapp/` | Lifecycle, state-machine, supervisor, and fault APIs are split into focused modules. |
| UCON shared core | `include/ucon/{ucon_status,ucon_types}.h` | `src/ucon/status.c` | UCON status and a compatibility scalar alias; common numerical behavior belongs to UMATH. |
| UCON direct algorithms | `include/ucon/control/classical/` | `src/ucon/control/classical/` | PID, lead-lag, IMC, and Smith predictor are grouped as classical control methods; PID and lead-lag have generic implementations. |
| UCON initial algorithm outlines | `include/ucon/<family>/<algorithm>.h` | `src/ucon/<family>/<algorithm>.c` | One pair per `FIRST_PASS` and selected project/common `PLANNED` key; each pair declares/defines named operations with per-function TODOs and fail-closed behavior. |
| UCON future algorithm outlines | `include/ucon/future/<family>/<algorithm>.h` | `src/ucon/future/<family>/<algorithm>.c` | Remaining `PLANNED` and all `DEFERRED` keys; same per-operation TODO outline, fail-closed and omitted from the default UETD library build. |

## Target-family registry and catalogue

The C profile registry contains the six Phase 0 families named in Part XV.1 and the Phase 1 STM32L4+ scaffold required by the SDMMC plan. `uetd_target_phase0_profiles()`, `uetd_target_phase1_profiles()`, and `uetd_target_profiles()` enumerate those C records. `uetd_target_profile_find()` looks up canonical family keys. Every record has `profile_complete == false`.

The wider family index `registry/targets/families/registry.json` includes all architecture and roadmap families named by the specification plus series-level STM32 and Espressif coverage. Its 66 roadmap entries are joined by catalogue-only RP2350 and one internal pipeline fixture. The exact-part chip index points to sourced device facts and completed resource maps. `uef-gen` reads those records from UETD and materializes them into its resolver; there is no parallel Python target database. The development-board index stores carrier-board details and may keep ESP-WROOM module facts inline. Physical generation remains blocked until chip package pins/alternate functions, peripheral instances, DMA/DMAMUX routing, clocks/resets, interrupts, memory regions, startup/linker integration, and errata review are complete, and the board has complete build/flash settings.

`registry/modules/registry.json` holds module dependency and selection metadata and indexes one detail file per module under `registry/modules/manifest/`. Add a module by creating its detail file and adding its path to the index. `api.json` declares canonical registry/schema paths; entry counts are derived from the indexes. A chip record's `family_id` is its family link.

## Phase 1 additions

SDMMC and optional FatFS remain in Phase 1. The specification names STM32H7, STM32F7, and STM32L4+ as target families; H7/F7 use their Phase 0 profile records, while L4+ has a new Phase 1 profile. SDMMC exposes raw block/card operations; its current source is a target-facing scaffold. The FatFS diskio adapter sits above UPAL, is consumer-supplied/opt-in, and cannot make an incomplete SDMMC driver usable.

FDCAN-FD has an explicit frame/configuration API in `fdcan_fd.h` and a separate implementation translation unit. Its validation rejects malformed frame identifiers, lengths, and FD/BRS combinations; target timing, message-RAM layout, filters, queues, IRQ/DMA integration, and recovery remain `UETD_NOT_SUPPORTED` scaffold work. The classic CAN API remains separately available through UPAL CAN.

## UMATH shared numerical contract

`umath_scalar_t` is configured by `UETD_UMATH_SCALAR_TYPE` (default `uetd_f32_t`) and declared by `umath/config.h`; `types.h` contains only foundational UCORE aliases. `umath_accumulator_t` defaults to the selected scalar and can be widened independently with `UETD_UMATH_ACCUMULATOR_TYPE=uetd_f64_t`. `UETD::Core` publishes both settings; the scalar choice affects ABI, while the accumulator choice controls intermediate precision and runtime cost. UMATH APIs use caller-owned storage, contiguous row-major matrix buffers, and documented quaternion/frame conventions. The implemented and priority mathematical systems are catalogued in [`UMATH_CATALOGUE.md`](catalogues/UMATH_CATALOGUE.md). Phase 0 prioritizes Jacobians/autodiff, quadrature and bounded ODE steps, FFT/windowing, 3D Lie groups, robust matrix methods, probability/statistics, least squares, and trajectory math; Phase 1 contains planar Lie groups and deterministic random sampling. Phase labels express order only: scaffold functions return `UMATH_NOT_IMPLEMENTED`, remain absent from the stable `umath.h` umbrella, and cannot be selected by uef-gen. Each tile has its own module manifest, including its public-header closure. `uef-gen` copies only implemented modules in the selected dependency closure.

## UCON algorithm status

Direct generic use:

- `umath_scalar_t` and allocation-free numerical helpers are public under `umath/`; `ucon_scalar_t` is a compatibility alias declared by UCON, and `ucon_status_t` is UCON-owned.
- `ucon_pid_*` implements the documented 2-DOF Tustin controller with filtered derivative, output/integral bounds, and configured anti-windup modes.
- `ucon_lead_lag_*` implements the bilinear first-order recurrence.

Every unregistered catalogue entry has an individual `.h`/`.c` pair with named operation declarations and definitions. A family-specific outline covers the algorithm lifecycle (for example estimator predict/correct, MPC prepare/solve/constraint verification, or filter coefficient design/sample processing); every function has its own TODO and returns `UCON_NOT_IMPLEMENTED` without changing state/output data. First-pass entries and the selected project/common planned set live directly under their normal family paths. Other planned and all deferred entries live under `future/`. These headers are excluded from `ucon.h` and cannot be selected by `uef-gen`.

The project-relevant planned set is explicit in `api.json`. A larger curated roadmap in `roadmap/ROADMAP_EXTENSIONS.md` adds missing online-identification, estimation, DSP, optimization, motor/power-conversion, guidance, and trajectory families. Roadmap entries are future-only and unregistered. A `PLANNED` priority or a generated function outline does not mean an algorithm is implemented or has an assigned release phase. Audit conflicts and open contracts are recorded in `audits/UCON_CATALOGUE_AUDIT.md`.

By default, CMake excludes `src/ucon/future/`. Set `UETD_BUILD_FUTURE_UCON_SCAFFOLDS=ON` only when compiling those fail-closed development entry points is useful. This does not make them selectable in generated firmware.

## Build and dependency selection

`CMakeLists.txt` builds portable layer sources and uses separate source groups for peripherals, middleware, protocols, UMATH, and UCON. It includes only one UHAL backend and one UOS backend. Phase 0/1 UMATH stubs are compiled as fail-closed linkable outlines; only `src/umath/future/` is excluded unless explicitly enabled. Future UCON stubs are also excluded unless enabled. A host build validates a host configuration; it does not establish embedded timing, electrical behavior, ISR safety, or a supported board.

FreeRTOS Kernel 11.3.1 is staged under `third_party/FreeRTOS-Kernel-11.3.1/` and described by a lock plus integration manifest. The lock records upstream tag V11.3.1 and its resolved commit; the local copy has not been checksum-compared with upstream. CMake wires common kernel sources and the generic ARM Cortex-M4F GCC port. A generated project must still supply its configuration header, cross-toolchain, ABI/FPU flags, startup/interrupt code, and linker script. The profile does not establish support for a specific board. FatFS remains optional and consumer supplied. External templates must continue to obey UETD's source/API and license policies.

## Completion rules

Promote a scaffold only after its concrete target/algorithm contract is reviewed and its implementation is exercised at the appropriate level. For an algorithm, provide typed init/reset/step APIs, dimensions and units, bounds and numerical failure behavior, memory cost, aliasing rules, and matching direct/generated semantics. For a peripheral, provide exact target capability and resource data, bounded operations and recovery, and hardware validation. Update the implementation, public header, manifest/API metadata, module map, README, and TODO together.

## Current implementation boundary

The source tree is structurally populated, but target support is not complete. The host backend supports portable simulation and the PPM decoder is the only implemented protocol core outside UCON's PID and lead-lag. Cortex-M target files, most UPAL drivers, sensor adapters, and the remaining protocol APIs are fail-closed scaffolds. The planned SafeRTOS and Part XV.3 protocol modules are recorded as non-selectable outlines. See [`UETD_SYSTEM_AUDIT.md`](audits/UETD_SYSTEM_AUDIT.md) for source/API coverage, concurrency findings, missing validation assets, and specification consistency questions.
