# Unified Embedded Template Directory (UETD)

UETD (the Unified Embedded Template Directory) contains the allocation-conscious
C11 framework for embedded applications: portable core APIs, hardware and
peripheral boundaries, middleware, protocols, lifecycle services, algorithms,
catalogues, and reusable templates. Firmware can call its C APIs directly.
`uef-gen` assembles a hardware-configured project scaffold from UETD's public
hardware and module catalogues; generated firmware does not require Python at
runtime.

The public UETD workflow is through the C APIs and the documented hardware-
configuration path.

## License

UETD and `uef-gen` together make up the Unified Embedded Framework and are
licensed under GNU GPL-3.0-only; see the repository [LICENSE.md](../LICENSE.md).
Dependencies retain their own license terms.

## Source layers

| Layer | Responsibility | Main paths |
|---|---|---|
| UCORE | Shared scalar storage types, status, assertions, limits, and time API | `include/ucore`, `src/ucore` |
| UMATH | Shared scalar, fixed-point, vector, complex, matrix, rotation, geometry, and numerical APIs | `include/umath`, `src/umath` |
| UHAL | CPU, GPIO, IRQ, memory, target boundaries, and the selected implementation of UCORE time | `include/uhal`, `src/uhal` |
| UPAL | Peripheral APIs and drivers | `include/upal`, `src/upal` |
| UOS | Bare-metal and FreeRTOS Kernel abstraction | `include/uos`, `src/uos` |
| UMID | Sensor-facing APIs and manual hardware drivers, logging, health, storage adapters | `include/umid`, `src/umid` |
| UACTUATOR | Target-independent actuator authoring identities; physical drivers remain handwritten per target | `registry/modules/manifest/uactuator` |
| UPROTO | Reusable protocol APIs and portable protocol logic | `include/uproto`, `src/uproto` |
| UAPP | Component lifecycle, state machines, supervision, faults | `include/uapp`, `src/uapp` |
| UCON | Reusable control and estimation APIs and algorithms | `include/ucon`, `src/ucon` |

### File naming

The directory and include path carry the module identity, so module-owned files
use the capability name without repeating the layer prefix: for example,
`include/umid/health.h`, `include/upal/uart.h`, and
`src/uos/freertos/freertos.c`. Keep layer prefixes on public C symbols and
globally unique include guards; this convention changes filesystem names, not
the C API namespace. Retain names whose prefix is part of the capability,
protocol, or algorithm itself, such as `uavcan_v1.h` and `ukf_merwe.h`.

UMID's sensor and measurement driver headers and sources live under
`include/umid/drivers/` and `src/umid/drivers/`. The directory is the
manual extension point for concrete hardware drivers; UETD does not maintain an
exhaustive per-device driver registry or generate chip drivers. The current
driver files are API/source scaffolds. CMake recursively includes UMID source
files, while generated projects use the existing UMID module manifests to
select sources explicitly.

The initial UACTUATOR entry, `h_bridge`, is a draft authoring record only. It
does not supply pin topology, a gate driver, a safe electrical state, or a
generation-ready target binding. Complete that contract against a selected
actuator/board before using it in a generated build.

Application-level sensor simulation is outside UETD's hardware-driver layer.

UETD's public hardware and software catalogues live under `registry/`. Their
indexes point to individual detail records, and `api.json` declares the
canonical registry paths. `uef-gen` reads those declarations from the live UETD
checkout rather than maintaining a duplicate target or module database.

## Target catalogue and initial profiles

The UHAL C registry contains six Phase 0 family profiles (STM32G4, STM32F4,
single-core STM32H7, STM32F7, STM32G0, and nRF52840) plus a Phase 1 STM32L4+
profile for the SDMMC roadmap. Each is explicitly incomplete: family notes only,
with no verified package pins, clocks, DMA, memory, startup, or linker data.

`registry/targets/families/registry.json` is the canonical UETD-owned family index; its
66 roadmap records live under `registry/targets/families/manifest/`, alongside a
catalogue-only RP2350 family and one internal pipeline-fixture family (68 index
entries total). The 66 roadmap records span STM32, Espressif, Nordic, NXP,
Microchip AVR and SAM, Raspberry Pi, TI, WCH, GigaDevice, and Renesas. The
catalogue also covers RP2350, whose architecture is selectable but is not yet
assigned a roadmap phase. Representative part names are examples only. The
separate `registry/targets/chips/registry.json` index points
to one exact-part manifest per chip under `registry/targets/chips/manifest/` and adds
51 vendor-sourced exact-part records across STM32, Nordic nRF, Microchip AVR
and SAM, NXP i.MX RT/LPC, Renesas RA, TI C2000, WCH, GigaDevice, Espressif ESP32,
and Raspberry Pi RP2040/RP2350, plus the non-physical `generic-reference`
generator fixture (52 chip records total). The
physical records currently contain sourced identity, core, package, headline
memory, peripheral groups, and source links. `uef-gen` loads these records into
its resolver directly from the API-declared UETD registry; it does not keep a
second chip database in Python. `generation_available` is per-record, so an
incomplete part remains discoverable while validation blocks generation. Full
chip profiles add resource maps to that same chip record, while board clocks,
pin reservations, and build/flash settings belong in
`registry/targets/development_boards/`. Module details can live inline with a
board record; no separate hardware-module catalogue is needed. Use
`uef-gen targets list --include-scaffolds` or `uef-gen targets show <part>` to
inspect UETD records.

The development-board index includes Arduino UNO R3, Mega 2560 Rev3, Leonardo,
Nano Classic, and Nano Every; Espressif ESP32-DevKitC V4 with an ESP-WROOM-32
module; Raspberry Pi Pico and Pico 2 (non-W); ST NUCLEO-G474RE, NUCLEO-F746ZG,
NUCLEO-H743ZI2, NUCLEO-H745ZI-Q, NUCLEO-H755ZI-Q, and STM32H745I-DISCO; and
the Nordic nRF9160 DK. The separate
STM32F722RET6 entry is a pending carrier placeholder because its board model
was not specified. There are 15 source-backed board bindings plus this pending
placeholder (16 entries total); all remain `generation_available: false` until
chip maps, board revisions, clocks, pin reservations, build profiles, and flash
profiles are completed and reviewed. The STM32F103C8T6 chip is registered
without a generic Blue Pill binding because those boards vary by manufacturer
and revision.

To add a part, create or extend its exact-part JSON and add one `{ "id",
"path" }` reference to `registry/targets/chips/registry.json`. Its `family_id`
links it to the family roadmap; no second part list needs updating. To add a
development board, create its detail JSON and add one index reference to
`registry/targets/development_boards/registry.json`. The board record references
`chip_id` and may inline module details. Cite the datasheet, reference manual,
board schematic, and programming-tool documentation used for each record.
Leave `generation_available` false until the chip resources and the board's
build/flash profiles are complete and reviewed. See
[`registry/targets/README.md`](registry/targets/README.md) for the authoring
shape and separation between chip, board, and inline module facts.

## UMATH numerical foundation

UMATH owns reusable numerical primitives used across UCON and other UETD modules. The implemented surface includes checked scalar operations, Q-format fixed-point arithmetic, vectors, complex numbers, dense row-major matrices, Cholesky factorization/solve, quaternions, rigid 3D transforms, and polynomial evaluation/calculus. The priority catalogue now places nonlinear-estimator Jacobians/autodiff, numerical integration and ODE steps, FFT/windowing for spectral PLL work, 3D Lie operations, matrix factorizations and eigensystems, probability/statistics, least squares, and trajectory primitives in Phase 0. Planar Lie groups and deterministic random sampling are Phase 1; sparse CSR remains later. [UMATH catalogue](docs/catalogues/UMATH_CATALOGUE.md) explains the project reasons and lists every module.

Each UMATH module has an individual registry record. Generated projects receive only selected, implemented modules, their declared C sources, and each module’s explicit transitive public-header closure. Phase 0/1 entries are priority scaffolds: their source functions remain fail-closed and return `UMATH_NOT_IMPLEMENTED`, and uef-gen blocks them until implementation and review. Only outlines still under `src/umath/future/` are excluded from the default library source set; compiling a scaffold does not make its math usable.

Use the [equation checklist](docs/design/UETD_EQUATION_CHECKLIST.md) as the per-file
equation and algorithm worklist for UMATH, UCON, and other equation-bearing
UETD layers.

## UCON status

The generic PID and lead-lag implementations are available for direct C use
and use UMATH for shared scalar primitives. Other catalogue algorithms have
family-organized API/source scaffolds; `FIRST_PASS` and `PLANNED` describe
priority, not completion. Unimplemented functions fail closed with
`UCON_NOT_IMPLEMENTED` and do not change caller data.

The module index at `registry/modules/registry.json` points to individual
module records. Placeholder modules remain `scaffold_only` and are not
selectable for generated firmware. See the [UCON catalogue audit](docs/audits/UCON_CATALOGUE_AUDIT.md)
for coverage findings and [roadmap extensions](docs/roadmap/ROADMAP_EXTENSIONS.md)
for proposed algorithms beyond Part XVI.

To promote an algorithm, first define and review its typed fixed-size
configuration/state, dimensions/units, equations, numerical bounds, error
behavior, memory/work limits, and reset policy. Implement and review its
generic C behavior, then update the public module/API metadata. Never mark an
algorithm available while any function is still a placeholder.

## Other layer status

UCORE provides portable storage types, status, assertions, limits, and the time API; UMATH owns shared mathematical operations. The x86 UHAL
backend is for host simulation; the Cortex-M UHAL source is still a target
outline except for its single-core atomic operations and fail-stop fault path.
No exact board profile has verified startup, clocks, pins, DMA routes, cache
policy, or linker placement. UPAL peripheral APIs, including Phase 1 SDMMC and
FDCAN-FD, are linkable outlines and do not yet drive hardware. The optional
FatFS adapter cannot work until a target SDMMC implementation and a selected
FatFS package are supplied.

UOS contains a FreeRTOS adapter and a bare-metal backend. FreeRTOS Kernel
11.3.1 is staged under `third_party/FreeRTOS-Kernel-11.3.1/`; its lock
records upstream tag `V11.3.1`, commit
`3a22924e0a9ddbbc8b0758881c33b3422a5cc20d`, and `LICENSE.md`. The copied
source has not yet been checksum-compared with that upstream commit. CMake
provides a generic `arm_cm4f_gcc` integration profile, but choosing it still
requires an ARM cross toolchain, project `FreeRTOSConfig.h`, ABI/FPU settings,
and board-specific startup/interrupt setup. This does not make any UETD MCU
target generation-ready. A non-selectable SafeRTOS operation outline records
the additional backend work. UMID's byte ring buffer
now uses UHAL atomic index operations under an explicit single-producer/single-
consumer contract; sensor adapters remain scaffolds. Health snapshots are
available for concurrent supervision.

UPROTO's PPM interval decoder is implemented as fixed-memory portable code.
DSHOT, CRSF, SBUS, MAVLink, and UAVCAN/DroneCAN remain fail-closed protocol
outlines. The six Part XV.3 protocol additions have individual operation
outlines under `include/uproto/future/` and `src/uproto/future/`; they are
not exposed through `uproto.h` or `protocol_provides`.
`UETD_BUILD_FUTURE_UPROTO_SCAFFOLDS` is OFF by default. Read
[UETD system audit](docs/audits/UETD_SYSTEM_AUDIT.md) and
[UPROTO roadmap](docs/roadmap/UPROTO_ROADMAP.md) for specific gaps and promotion gates.

## Build

Requirements: CMake 3.20 or newer and a C11 compiler. A host build is useful for portable code and host-simulation APIs; it is not hardware validation. The library links `m` on non-MSVC toolchains for standard math functions; MSVC obtains those functions from its runtime.

```powershell
cmake -S . -B build -DUETD_TARGET=HOST -DUETD_UOS_BACKEND=BAREMETAL
cmake --build build
```

To include the minimal lifecycle example, configure with `-DUETD_BUILD_EXAMPLE=ON`:

```powershell
cmake -S . -B build -DUETD_TARGET=HOST -DUETD_UOS_BACKEND=BAREMETAL -DUETD_BUILD_EXAMPLE=ON
cmake --build build
```

For Cortex-M, set `UETD_TARGET=CORTEX_M` and supply `UETD_CMSIS_INCLUDE_DIR` and `UETD_CMSIS_DEVICE_HEADER`. To select FreeRTOS, set `UETD_UOS_BACKEND=FREERTOS`, choose a profile key from `third_party/FreeRTOS-Kernel-11.3.1/uetd-integration.json`, and supply the generated project's `FreeRTOSConfig.h` directory. The profile wires kernel and port source files; it does not select a board, compiler installation, ABI/FPU flags, startup code, or linker script. The generic profile is not board qualification. FatFS remains opt-in through `UETD_ENABLE_FATFS` and `UETD_FATFS_INCLUDE_DIR`.

| CMake option | Values / purpose |
|---|---|
| `UETD_TARGET` | `HOST` or `CORTEX_M`; selects exactly one UHAL implementation |
| `UETD_UOS_BACKEND` | `BAREMETAL` or `FREERTOS` |
| `UETD_FREERTOS_PROFILE` | Exact key from `third_party/FreeRTOS-Kernel-11.3.1/uetd-integration.json` (currently `arm_cm4f_gcc`) |
| `UETD_FREERTOS_CONFIG_INCLUDE_DIR` | Generated firmware directory containing `FreeRTOSConfig.h` |
| `UETD_BUILD_EXAMPLE` | Build the minimal component-lifecycle example |
| `UETD_BUILD_FUTURE_UMATH_SCAFFOLDS` | Opt in to compile the later UMATH outlines under `src/umath/future/` |
| `UETD_BUILD_FUTURE_UPROTO_SCAFFOLDS` | Opt in to compile non-selectable planned protocol outlines |
| `UETD_ENABLE_FATFS` | Include the optional FatFS diskio adapter |
| `UETD_FATFS_INCLUDE_DIR` | Directory containing consumer-supplied `ff.h` and `diskio.h` |

## Minimal API example

`examples/minimal/` demonstrates the UAPP lifecycle only; it does not configure hardware:

```c
#include "uapp/lifecycle.h"

static uetd_status_t init(void *context)  { (void)context; return UETD_OK; }
static uetd_status_t start(void *context) { (void)context; return UETD_OK; }
static uetd_status_t stop(void *context)  { (void)context; return UETD_OK; }

uapp_component_t component = {
    .state = UAPP_STATE_CREATED,
    .name = "sensor",
    .init = init,
    .start = start,
    .stop = stop,
};

if (uapp_component_init(&component) == UETD_OK &&
    uapp_component_start(&component) == UETD_OK) {
    (void)uapp_component_stop(&component);
}
```

The bounded state machine copies events into a fixed queue, so callback code
can post follow-up work without recursively changing state:

```c
#include "uapp/statemachine.h"

enum { APP_IDLE, APP_RUNNING };
enum { APP_EVENT_START = 1 };

static const uapp_sm_transition_t transitions[] = {
    { .from = APP_IDLE, .event_id = APP_EVENT_START, .to = APP_RUNNING },
};
static const uapp_sm_state_hooks_t hooks[2] = {{0}};

const uapp_sm_config_t config = {
    .transitions = transitions,
    .transition_count = 1,
    .state_hooks = hooks,
    .state_count = 2,
    .initial_state = APP_IDLE,
};
uapp_statemachine_t machine = UAPP_SM_INITIALIZER;
uapp_sm_event_t event;

if (uapp_sm_init(&machine, &config) == UETD_OK &&
    uapp_sm_make_event(&event, APP_EVENT_START, NULL, 0) == UETD_OK) {
    uetd_status_t posted = uapp_sm_post(&machine, &event);
    if (posted == UETD_OVERFLOW) {
        /* Preserve/re-route critical events; do not silently drop them. */
    } else if (posted != UETD_OK) {
        /* Propagate other status failures to the application owner. */
    } else {
        uetd_status_t processed = uapp_sm_process_one(&machine);
        if (processed == UETD_OK && uapp_sm_is_active(&machine)) {
            /* APP_RUNNING's entry hook completed. */
        }
    }
}
```

One owner task calls the FSM API and executes transitions. Other tasks and ISRs
send events through a platform-safe ingress queue or notification for that owner
to drain; the embedded queue is not synchronized or ISR-safe. Always check
`uapp_sm_post`: overflow rejects the newest event, so critical events need an
application-owned latch/mailbox or independent safety path. Callbacks run
synchronously in the owner task. They may post copied events, but must be
bounded, non-blocking, and must not call `uapp_sm_process_one`. A transition
callback error fault-latches the machine and clears pending events;
`uapp_sm_is_active` and `uapp_sm_is_faulted` distinguish completed state entry
from fault. See [the UAPP state-machine contract](docs/design/UAPP_STATE_MACHINE_DESIGN.md)
for memory, timing, callback-failure, and reset policy.

## UEF Project Language

UETD also owns the component, type, unit, UMATH, and target catalogues used by
the standalone UEF Project Language (`.uef`/`.uefs`) path in `uef-gen`. Those
JSON records are the source of truth for language names and UETD contracts; the
generator discovers them through `api.json` rather than maintaining its own
copy. The standalone language front end and generation path are still in
development. Start with the [UEF-PL user handbook](../docs/guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md)
for the current status and project walkthrough, then use the
[registries and discovery guide](../docs/guides/UEF_PL_REGISTRIES.md) when you
need to understand or change a registry record.

## Read next

- [UEF-PL user handbook](../docs/guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md) explains how a standalone language project is intended to work.
- [UETD registries and discovery](../docs/guides/UEF_PL_REGISTRIES.md) explains the JSON indexes, manifests, schemas, and lookup flow.
- [UEF-PL technical guide](../docs/guides/UEF_PROJECT_LANGUAGE_GUIDE.md) and [generated catalogue](../docs/reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md) document the source rules and registered names.
- [Maintainer architecture notes](docs/ARCHITECTURE.md) map APIs, source ownership, and scaffold status.
- [Implementation inventory](docs/TODO.md) is the maintainer worklist; the repository-level [implementation plan](../docs/roadmap/IMPLEMENTATION_PLAN.md) gives the ordered cross-project checklist.
- [`templates/modules/README.md`](templates/modules/README.md) documents the renderable CMake build route for every catalog module; [`templates/modules/catalog.md`](templates/modules/catalog.md) lists them by module ID.
- [`templates/targets/README.md`](templates/targets/README.md) indexes the parameterized Cortex-M, AVR, Arduino, ESP-IDF, Nordic/Zephyr, RP2040, and i.MX RT templates and their input contracts.
- [`third_party/README.md`](third_party/README.md) describes the UETD-managed FreeRTOS dependency policy.
