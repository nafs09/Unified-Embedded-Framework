# UEF source architecture and readiness

This document maps UEF's C11 layers, generation boundary, and current implementation state. The architecture contract is in the UEF specification; this file describes the checked-out code and distinguishes usable generic behavior from target scaffolds.

## Ownership and dependency direction

```text
UCORE (shared storage types and status)
  ├── UMATH (shared numerical primitives)
  └── UHAL (one selected target backend)
       ├── UPAL (peripheral drivers)
       ├── UOS (one selected scheduler backend)
       └── UMID → UPROTO → UAPP
                    └────────── UCON consumes UMATH and may use other public UEF services

NEXUS ControlGraph → ControlIR JSON → uef-gen bridge → UEF registries/manifests/templates
```

UEF owns reusable firmware behavior, algorithm definitions, public types, module/API metadata, and templates. NEXUS owns graph authoring and serialization. `uef-gen` validates the ControlIR transport envelope, traverses nodes, resolves them through UEF's `registry/control_ir/registry.json` index and per-entry manifests, plans UEF module closure from `registry/modules/registry.json`, and renders only UEF-declared templates. Algorithm meaning is not reconstructed inside the Python bridge. Generated firmware does not depend on NEXUS or Python.

Portable source must not include vendor device headers. `UEF_TARGET` selects one UHAL backend; `UEF_UOS_BACKEND` selects one operating-system backend. Concrete part, package, board, and clock details belong in target data and generated board configuration, not in generic middleware or algorithm code.

## Main source map

| Concern | Public surface | Implementation | Current state |
|---|---|---|---|
| Shared types/status/time | `include/uef/ucore/` | `src/ucore/` | Portable base APIs exist; host execution is not MCU qualification. |
| Shared mathematics | `include/uef/umath/` | `src/umath/` | Implemented scalar/vector/complex/matrix/rotation primitives plus phase-indexed fail-closed scaffolds; only later catalogue items remain below `future/`. |
| UHAL | `include/uef/uhal/` | `src/uhal/x86/`, `src/uhal/arm_cm/` | Generic contracts and backends exist; exact board startup and target-family profile data remain incomplete. |
| Phase 0/1 profile registry | include/uef/uhal/target_profiles.h and per-family profile.h files | src/uhal/target_profiles.c and per-family profile.c files | Six Phase 0 and one Phase 1 profile records are queryable and incomplete; no exact part is certified. |
| Target-family roadmap | `uef_api.json` declaration | `registry/targets/families/registry.json` plus `registry/targets/families/manifest/` | Canonical 65-entry index with one detail file per family. Later targets are roadmap metadata, never exact-part support or generation targets. |
| Chip-base records | `uef_api.json` declaration | `registry/targets/chips/registry.json` plus `registry/targets/chips/manifest/` | 38 vendor-sourced exact-part index entries; not complete ChipSpecs and never generation targets until full maps exist. |
| UEF modules | `uef_api.json` declaration | `registry/modules/registry.json` plus `registry/modules/manifest/` | Module dependency and selection metadata is indexed by name; each module detail is in its own file. |
| ControlIR dispatch | `uef_api.json` declaration | `registry/control_ir/registry.json` plus `registry/control_ir/manifest/` | Shared UCON/UPROTO dispatch policy with per-entry node, alias, and algorithm manifests. |
| UPAL | `include/uef/upal/` | `src/upal/<peripheral>/` | APIs are grouped by peripheral. Most hardware operations remain unsupported until a target backend is implemented. |
| UOS | `include/uef/uos/` | `src/uos/baremetal/`, `src/uos/freertos/` | Backend boundary exists. FreeRTOS source/profile pinning is incomplete and fails closed. |
| UMID | `include/uef/umid/` | `src/umid/` | Middleware APIs exist; hardware availability follows the selected UPAL driver and optional dependencies. |
| UPROTO | `include/uef/uproto/` | `src/uproto/`, `templates/uproto/` | Initial protocol APIs and wrapper templates exist; transport, parser, upstream dialect/stack, and hardware work remains declared/manual where applicable. |
| UAPP | `include/uef/uapp/` | `src/uapp/` | Lifecycle, state-machine, supervisor, and fault APIs are split into focused modules. |
| UCON shared core | `include/uef/ucon/{ucon_status,ucon_types}.h` | `src/ucon/status.c` | UCON status and a compatibility scalar alias; common numerical behavior belongs to UMATH. |
| UCON direct algorithms | `include/uef/ucon/control/` | `src/ucon/control/` | PID and lead-lag have generic implementations and generated instance wrappers. |
| UCON initial algorithm outlines | `include/uef/ucon/<family>/<algorithm>.h` | `src/ucon/<family>/<algorithm>.c` | One generated pair per `FIRST_PASS` and selected project/common `PLANNED` key; each pair declares/defines named operations with per-function TODOs and fail-closed behavior. |
| UCON future algorithm outlines | `include/uef/ucon/future/<family>/<algorithm>.h` | `src/ucon/future/<family>/<algorithm>.c` | Remaining `PLANNED` and all `DEFERRED` keys; same per-operation TODO outline, fail-closed and omitted from the default UEF library build. |

## Target-family registry and catalogue

The C profile registry contains the six Phase 0 families named in Part XV.1
(STM32G4xx, STM32F4xx, single-core STM32H7xx, STM32F7xx, STM32G0xx, and
nRF52840) and the Phase 1 STM32L4+ scaffold required by the SDMMC plan.
uef_target_phase0_profiles(), uef_target_phase1_profiles(), and
uef_target_profiles() enumerate those C records. uef_target_profile_find()
looks up their canonical family keys. Every record has
profile_complete == false.

The wider family index `registry/targets/families/registry.json` includes all architecture and
roadmap families named by the specification plus series-level STM32 and
Espressif coverage. The separate chip-base index `registry/targets/chips/registry.json` points to 38
vendor-sourced exact-part identity and headline capability records, including
representative devices from other named vendor groups. Neither
catalogue is a generation-ready ChipSpec. A complete target still needs
package pins/alternate functions, peripheral instances, DMA/DMAMUX routing,
clocks/resets, interrupts, memory regions, startup/linker integration, errata,
and a verification record. uef-gen reads both UEF-owned catalogues through
`uef_api.json` and cannot resolve a base record as a physical target. Family
details are stored under `registry/targets/families/manifest/`; exact-part details are
stored under `registry/targets/chips/manifest/`.

`registry/modules/registry.json` holds module dependency and selection metadata and
indexes one detail file per module under `registry/modules/manifest/`.
`registry/control_ir/registry.json` holds shared UCON/UPROTO dispatch policy and
indexes node, alias, and algorithm records under `registry/control_ir/manifest/`. Add
an entry by creating its detail file and adding a path reference to the
corresponding registry. A chip record's `family_id` is its only family link.
`uef_api.json` declares canonical registry/schema paths; entry counts are
derived from the indexes.

## Phase 1 additions

SDMMC and optional FatFS remain in Phase 1. The specification names STM32H7, STM32F7, and STM32L4+ as target families; H7/F7 use their Phase 0 profile records, while L4+ has a new Phase 1 profile. SDMMC exposes raw block/card operations; its current source is a target-facing scaffold. The FatFS diskio adapter sits above UPAL, is consumer-supplied/opt-in, and cannot make an incomplete SDMMC driver usable.

FDCAN-FD has an explicit frame/configuration API in `upal_fdcan_fd.h` and a separate implementation translation unit. Its validation rejects malformed frame identifiers, lengths, and FD/BRS combinations; target timing, message-RAM layout, filters, queues, IRQ/DMA integration, and recovery remain `UEF_NOT_SUPPORTED` scaffold work. The classic CAN API remains separately available through UPAL CAN.

## UMATH shared numerical contract

`umath_scalar_t` is configured by `UEF_UMATH_SCALAR_TYPE` (default `uef_f32_t`) and declared by `uef/umath/config.h`; `uef_types.h` contains only foundational UCORE aliases. `umath_accumulator_t` defaults to the selected scalar and can be widened independently with `UEF_UMATH_ACCUMULATOR_TYPE=uef_f64_t`. `UEF::Core` publishes both settings; the scalar choice affects ABI, while the accumulator choice controls intermediate precision and runtime cost. UMATH APIs use caller-owned storage, contiguous row-major matrix buffers, and documented quaternion/frame conventions. The implemented and priority mathematical systems are catalogued in [`UMATH_CATALOGUE.md`](UMATH_CATALOGUE.md). Phase 0 prioritizes Jacobians/autodiff, quadrature and bounded ODE steps, FFT/windowing, 3D Lie groups, robust matrix methods, probability/statistics, least squares, and trajectory math for the current estimator, PLL, and predictive-control work. Phase 1 contains planar Lie groups and deterministic random sampling. These phase labels express order only: scaffold functions return `UMATH_NOT_IMPLEMENTED`, remain absent from the stable `umath.h` umbrella, and cannot be selected by uef-gen. Each tile has its own module manifest, including its public-header closure. `uef-gen` copies only implemented modules in the selected dependency closure.

## UCON algorithm and generation contract

Direct generic use:

- `umath_scalar_t` and allocation-free numerical helpers are public under `uef/umath/`; `ucon_scalar_t` is a compatibility alias declared by UCON, and `ucon_status_t` is UCON-owned.
- `ucon_pid_*` implements the documented 2-DOF Tustin controller with filtered derivative, output/integral bounds, and the configured anti-windup modes.
- `ucon_lead_lag_*` implements the bilinear first-order recurrence.
- Thin `templates/ucon/wrappers/` templates expose named per-node instances. Templates forward to the generic implementation; they do not duplicate algorithm equations.

Every unregistered catalog entry has an individual `.h`/`.c` pair with named operation declarations and definitions. A family-specific outline covers the algorithm lifecycle (for example estimator predict/correct, MPC prepare/solve/constraint verification, or filter coefficient design/sample processing); every function has its own TODO and returns `UCON_NOT_IMPLEMENTED` without changing state/output data. Family directories keep related algorithms together; the public directory name `estimators/` corresponds to catalog family `estimation/`. First-pass entries and the selected project/common planned set live directly under their normal family paths. Other planned and all deferred entries live under `future/`. The shared type-erased call envelope is a temporary development interface; it must be replaced by reviewed typed fixed-size contracts before promotion. These headers are excluded from `ucon.h` and cannot be selected by uef-gen.

The project-relevant planned set is explicit in `uef_api.json`. A larger curated roadmap in `ROADMAP_EXTENSIONS.md` adds missing online-identification, estimation, DSP, optimization, motor/power-conversion, guidance, and trajectory families. Roadmap entries are future-only and unregistered. A `PLANNED` priority or a generated function outline does not mean an algorithm is implemented or has an assigned release phase. Audit conflicts and open contracts are recorded in `UCON_CATALOGUE_AUDIT.md`.

By default, CMake excludes `src/ucon/future/`. Set `UEF_BUILD_FUTURE_UCON_SCAFFOLDS=ON` only when compiling those fail-closed development entry points is useful. This does not make them selectable in generated firmware.

The manifest's `registered` flag gates generation. Only registered entries may list output templates. An unregistered entry produces an `algorithm_unregistered` diagnostic; a registration flag is not a substitute for reviewing equations, dimensions, units, numeric limits, static memory, output/failure behavior, template parity, and hardware/manual actions.

## Build and dependency selection

`CMakeLists.txt` builds portable layer sources and uses separate globs for peripheral, middleware, protocol, UMATH, and UCON source families. It includes only one UHAL backend and one UOS backend. Phase 0/1 UMATH stubs are compiled as fail-closed linkable outlines; only `src/umath/future/` is excluded unless explicitly enabled. Future UCON stubs are also excluded unless enabled. The host build validates a host configuration; it does not establish embedded timing, electrical behavior, ISR safety, or a supported board.

FreeRTOS source is intended to live under `third_party/freertos-kernel/` and is governed by a lock plus integration manifest. The selected revision/profile, legal text, and configuration header must exist before the FreeRTOS CMake path proceeds. FatFS remains optional and consumer supplied. External templates must continue to obey UEF's source/API and license policies.

## Completion rules

Promote a scaffold only after its concrete target/algorithm contract is reviewed and its implementation is exercised at the appropriate level. For an algorithm, provide typed init/reset/step APIs, dimensions and units, bounds and numerical failure behavior, memory cost, aliasing rules, and matching direct/generated semantics. For a peripheral, provide exact target capability and resource data, bounded operations and recovery, and hardware validation. Update the implementation, public header, manifest/API metadata, module map, README, and TODO together.


## Current implementation boundary

The source tree is structurally populated, but target support is not complete.
The host backend supports portable simulation and the PPM decoder is the only
implemented protocol core outside UCON's PID and lead-lag. Cortex-M target
files, most UPAL drivers, sensor adapters, and the remaining protocol APIs are
fail-closed scaffolds. The planned SafeRTOS and Part XV.3 protocol modules are
recorded as non-selectable outlines. See `UEF_SYSTEM_AUDIT.md` for source/API
coverage, concurrency findings, missing validation assets, and specification
consistency questions.
