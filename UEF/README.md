# Unified Embedded Framework (UEF)

UEF is an allocation-conscious C11 framework for embedded applications. It provides portable core APIs, hardware and peripheral boundaries, middleware, protocols, application lifecycle services, and reusable control algorithms. Firmware can call the C APIs directly or use `uef-gen` to assemble a project and render UEF-owned per-instance templates. NEXUS authors the application graph and sends its serialized ControlIR to `uef-gen`; neither NEXUS nor Python runs on the firmware target.

## Source layers

| Layer | Responsibility | Main paths |
|---|---|---|
| UCORE | Shared scalar storage types, status, assertions, limits, and time API | `include/uef/ucore`, `src/ucore` |
| UMATH | Shared scalar, fixed-point, vector, complex, matrix, rotation, geometry, and numerical APIs | `include/uef/umath`, `src/umath` |
| UHAL | CPU, GPIO, IRQ, memory, target boundaries, and the selected implementation of UCORE time | `include/uef/uhal`, `src/uhal` |
| UPAL | Peripheral APIs and drivers | `include/uef/upal`, `src/upal` |
| UOS | Bare-metal and FreeRTOS Kernel abstraction | `include/uef/uos`, `src/uos` |
| UMID | Sensor-facing APIs, logging, health, storage adapters | `include/uef/umid`, `src/umid` |
| UPROTO | Reusable protocol logic and generated instance wrappers | `include/uef/uproto`, `src/uproto`, `templates/uproto` |
| UAPP | Component lifecycle, state machines, supervision, faults | `include/uef/uapp`, `src/uapp` |
| UCON | Reusable control/estimation algorithms and their generation templates | `include/uef/ucon`, `src/ucon`, `templates/ucon` |

Every tileable catalogue lives under `registry/` in one consistent layout: a `registry.json` index, a sibling `manifest/` detail tree, and catalogue schemas. The family, exact-chip, module, and ControlIR catalogues all follow this pattern, so adding a record means adding one detail file and one index entry. `uef_api.json` declares the canonical index paths. `uef-gen` reads those declarations from the live UEF checkout and carries no duplicate UEF data.

## Target catalogue and initial profiles

The UHAL C registry contains six Phase 0 family profiles (STM32G4, STM32F4,
single-core STM32H7, STM32F7, STM32G0, and nRF52840) plus a Phase 1 STM32L4+
profile for the SDMMC roadmap. Each is explicitly incomplete: family notes only,
with no verified package pins, clocks, DMA, memory, startup, or linker data.

`registry/targets/families/registry.json` is the canonical UEF-owned family index; its
65 family records live separately under `registry/targets/families/manifest/`. It
covers 65 entries spanning STM32, Espressif, Nordic, NXP, Microchip, Raspberry Pi, TI, WCH,
GigaDevice, and Renesas family entries. It includes later families from the
specification and broad series coverage for planning. Representative part
names are examples only. The separate `registry/targets/chips/registry.json` index points
to one exact-part manifest per chip under `registry/targets/chips/manifest/` and adds
38 vendor-sourced exact-part base records across STM32, Nordic nRF, Microchip
SAM, NXP i.MX RT/LPC, Renesas RA, TI C2000, WCH, GigaDevice, Espressif ESP32,
and RP2040. They record part identity, core, package, headline memory, peripheral
groups, and source links. Multi-core and flashless records call out their
memory/core boundaries. They do not include complete pin/AF, clock, DMA, interrupt,
memory-region, startup, linker, or board data, so generation_available is
false for every record. uef-gen reads both catalogues through paths published
by uef_api.json; it does not carry duplicate chip or family lists. Use
uef-gen targets list --include-scaffolds or targets show <part> to inspect
the combined records.

To add a family or part, add one `{ "id", "path" }` reference to the matching
registry JSON and create the detail file at that path. A part's `family_id`
owns its family relationship, so no second part list needs updating. Cite
vendor sources in the chip record and leave `generation_available` false until
a complete, reviewed ChipSpec exists.

## UMATH numerical foundation

UMATH owns reusable numerical primitives used across UCON and other UEF modules. The implemented surface includes checked scalar operations, Q-format fixed-point arithmetic, vectors, complex numbers, dense row-major matrices, Cholesky factorization/solve, quaternions, rigid 3D transforms, and polynomial evaluation/calculus. The priority catalogue now places nonlinear-estimator Jacobians/autodiff, numerical integration and ODE steps, FFT/windowing for spectral PLL work, 3D Lie operations, matrix factorizations and eigensystems, probability/statistics, least squares, and trajectory primitives in Phase 0. Planar Lie groups and deterministic random sampling are Phase 1; sparse CSR remains later. [`UMATH_CATALOGUE.md`](UMATH_CATALOGUE.md) explains the project reasons and lists every module.

Each UMATH module has an individual registry record. Generated projects receive only selected, implemented modules, their declared C sources, and each module’s explicit transitive public-header closure. Phase 0/1 entries are priority scaffolds: their source functions remain fail-closed and return `UMATH_NOT_IMPLEMENTED`, and uef-gen blocks them until implementation and review. Only outlines still under `src/umath/future/` are excluded from the default library source set; compiling a scaffold does not make its math usable.

## UCON status

The generic PID and lead-lag implementations are available for direct C use and use UMATH for shared scalar primitives. Their thin UEF-owned wrappers are the only registered UCON generation entries. The full catalogue has separate family-organized algorithm files, including a reviewed set of future roadmap proposals; `FIRST_PASS` and `PLANNED` describe priority, not completion. Each unregistered algorithm now has a module outline with named validation, initialization/reset, and algorithm-specific operation functions, each with its own TODO. These functions remain fail-closed and return `UCON_NOT_IMPLEMENTED` without changing caller data.

`registry/control_ir/registry.json` is the ControlIR dispatch index. It points to one detail manifest per node wrapper, alias, and algorithm under `registry/control_ir/manifest/`; shared policy remains in the index. The module index at `registry/modules/registry.json` similarly points to individual module records. Placeholder modules remain `scaffold_only` and uef-gen cannot select them. Empty feature/numeric/output fields and unresolved model choices are called out in generated TODOs rather than guessed. Read [`UCON_CATALOGUE_AUDIT.md`](UCON_CATALOGUE_AUDIT.md) for the audit findings and [`ROADMAP_EXTENSIONS.md`](ROADMAP_EXTENSIONS.md) for proposed algorithms beyond Part XVI.

To promote an algorithm, first define and review its typed fixed-size configuration/state, dimensions/units, equations, numerical bounds, error behavior, memory/work limits, and reset policy. Implement the generic C behavior, then add an output template only if ControlIR needs structural specialization. Update the manifest and module/API metadata together and regenerate with `tools/generate_ucon_scaffolds.py`. Never set `registered: true` while any function is still a placeholder.

## Other layer status

UCORE provides portable storage types, status, assertions, limits, and the time API; UMATH owns shared mathematical operations. The x86 UHAL
backend is for host simulation; the Cortex-M UHAL source is still a target
outline except for its single-core atomic operations and fail-stop fault path.
No exact board profile has verified startup, clocks, pins, DMA routes, cache
policy, or linker placement. UPAL peripheral APIs, including Phase 1 SDMMC and
FDCAN-FD, are linkable outlines and do not yet drive hardware. The optional
FatFS adapter cannot work until a target SDMMC implementation and a selected
FatFS package are supplied.

UOS contains a FreeRTOS adapter and a bare-metal backend. The repository's
FreeRTOS lock is intentionally unpinned and the upstream source is absent, so
the FreeRTOS configuration cannot be selected yet. A non-selectable SafeRTOS
operation outline records the additional backend work. UMID's byte ring buffer
now uses UHAL atomic index operations under an explicit single-producer/single-
consumer contract; sensor adapters remain scaffolds. Health snapshots are
available for concurrent supervision.

UPROTO's PPM interval decoder is implemented as fixed-memory portable code.
DSHOT, CRSF, SBUS, MAVLink, and UAVCAN/DroneCAN remain fail-closed protocol
outlines. The six Part XV.3 protocol additions have individual operation
outlines under `include/uef/uproto/future/` and `src/uproto/future/`; they are
not exposed through `uproto.h`, `protocol_provides`, or the ControlIR manifest.
`UEF_BUILD_FUTURE_UPROTO_SCAFFOLDS` is OFF by default. Read
[`UEF_SYSTEM_AUDIT.md`](UEF_SYSTEM_AUDIT.md) and
[`UPROTO_ROADMAP.md`](UPROTO_ROADMAP.md) for specific gaps and promotion gates.

## Build

Requirements: CMake 3.20 or newer and a C11 compiler. A host build is useful for portable code and host-simulation APIs; it is not hardware validation. The library links `m` on non-MSVC toolchains for standard math functions; MSVC obtains those functions from its runtime.

```powershell
cmake -S . -B build -DUEF_TARGET=HOST -DUEF_UOS_BACKEND=BAREMETAL
cmake --build build
```

To include the minimal lifecycle example, configure with `-DUEF_BUILD_EXAMPLE=ON`:

```powershell
cmake -S . -B build -DUEF_TARGET=HOST -DUEF_UOS_BACKEND=BAREMETAL -DUEF_BUILD_EXAMPLE=ON
cmake --build build
```

For Cortex-M, set `UEF_TARGET=CORTEX_M` and supply `UEF_CMSIS_INCLUDE_DIR` and `UEF_CMSIS_DEVICE_HEADER`. The FreeRTOS integration intentionally fails closed until its exact upstream source revision, license paths, architecture/compiler profile, and generated `FreeRTOSConfig.h` are present. FatFS remains opt-in through `UEF_ENABLE_FATFS` and `UEF_FATFS_INCLUDE_DIR`.

| CMake option | Values / purpose |
|---|---|
| `UEF_TARGET` | `HOST` or `CORTEX_M`; selects exactly one UHAL implementation |
| `UEF_UOS_BACKEND` | `BAREMETAL` or `FREERTOS` |
| `UEF_FREERTOS_PROFILE` | Exact key from `third_party/freertos-kernel/uef-integration.json` |
| `UEF_FREERTOS_CONFIG_INCLUDE_DIR` | Generated firmware directory containing `FreeRTOSConfig.h` |
| `UEF_BUILD_EXAMPLE` | Build the minimal component-lifecycle example |
| `UEF_BUILD_FUTURE_UMATH_SCAFFOLDS` | Opt in to compile the later UMATH outlines under `src/umath/future/` |
| `UEF_BUILD_FUTURE_UPROTO_SCAFFOLDS` | Opt in to compile non-selectable planned protocol outlines |
| `UEF_ENABLE_FATFS` | Include the optional FatFS diskio adapter |
| `UEF_FATFS_INCLUDE_DIR` | Directory containing consumer-supplied `ff.h` and `diskio.h` |

## Minimal API example

`examples/minimal/` demonstrates the UAPP lifecycle only; it does not configure hardware:

```c
#include "uef/uapp/uapp_lifecycle.h"

static uef_status_t init(void *context)  { (void)context; return UEF_OK; }
static uef_status_t start(void *context) { (void)context; return UEF_OK; }
static void stop(void *context)          { (void)context; }

uapp_component_t component = {
    .state = UAPP_STATE_CREATED,
    .name = "sensor",
    .init = init,
    .start = start,
    .stop = stop,
};

if (uapp_component_init(&component) == UEF_OK &&
    uapp_component_start(&component) == UEF_OK) {
    uapp_component_stop(&component);
}
```

## Read next

- [`ARCHITECTURE.md`](ARCHITECTURE.md) maps APIs, source ownership, and scaffold status.
- [`TODO.md`](TODO.md) is the implementation and hardware-qualification plan.
- [`templates/ucon/README.md`](templates/ucon/README.md) describes algorithm registration and template conventions.
- [`third_party/README.md`](third_party/README.md) describes the UEF-managed FreeRTOS dependency policy.
