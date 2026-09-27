# Unified Embedded Framework (UEF)

UEF is a C11 framework for embedded systems. It provides portable interfaces and source modules for hardware access, peripheral drivers, middleware, protocols, operating-system integration, and application lifecycle management. Framework modules are compiled into a firmware project; UEF is not an operating system or a ready-made application.

## Layers

| Layer | Role |
|---|---|
| `ucore` | Shared types, status codes, math, time, and assertions |
| `uhal` | CPU, GPIO, interrupts, memory, timing, and target boundary |
| `upal` | Peripheral access such as UART, SPI, I2C, DMA, timers, and storage |
| `uos` | Bare-metal and FreeRTOS operating-system abstraction |
| `umid` | Reusable middleware and sensor-facing APIs |
| `uproto` | Protocol interfaces and implementations |
| `uapp` | Component lifecycle, state machines, supervision, and fault handling |
| `ucon` | Planned home for reusable control algorithms and templates; currently incomplete |

The public headers are under `include/uef/`, with implementations under `src/`. `uef_modules.json` describes modules and dependencies, while `uef_api.json` records peripheral API metadata.

The layer boundaries keep portable code above hardware-specific implementations: UPAL drivers use UHAL interfaces, middleware sits above the peripheral APIs, and application lifecycle helpers coordinate components. Target selection happens at configure time so a build selects one UHAL backend rather than mixing host simulation with a firmware backend.

## Build

Requirements: CMake 3.20 or newer and a C11 compiler. The host target is a simulation/testing boundary, not a substitute for hardware validation.

```powershell
cmake -S . -B build -DUEF_TARGET=HOST -DUEF_UOS_BACKEND=BAREMETAL
cmake --build build
```

To include the minimal lifecycle example, configure with `-DUEF_BUILD_EXAMPLE=ON`:

```powershell
cmake -S . -B build -DUEF_TARGET=HOST -DUEF_UOS_BACKEND=BAREMETAL -DUEF_BUILD_EXAMPLE=ON
cmake --build build
```

For Cortex-M, set `UEF_TARGET=CORTEX_M` and provide `UEF_CMSIS_INCLUDE_DIR` and `UEF_CMSIS_DEVICE_HEADER` for the selected device. FreeRTOS and FatFS are optional external dependencies configured through the corresponding CMake variables; neither is bundled here.

The main build options are:

| CMake option | Values / purpose |
|---|---|
| `UEF_TARGET` | `HOST` for simulation, or `CORTEX_M` for the CMSIS-backed target boundary |
| `UEF_UOS_BACKEND` | `BAREMETAL` or `FREERTOS` |
| `UEF_BUILD_EXAMPLE` | Build the minimal component-lifecycle executable |
| `UEF_ENABLE_FATFS` | Include the optional FatFS diskio adapter; requires an external FatFS package |

## Minimal API example

The checked-in `examples/minimal/` demonstrates the UAPP lifecycle API. A component supplies initialization/start callbacks, then transitions through the framework lifecycle functions:

```c
#include "uef/uapp/uapp_lifecycle.h"

static uef_status_t init(void* ctx)  { (void)ctx; return UEF_OK; }
static uef_status_t start(void* ctx) { (void)ctx; return UEF_OK; }
static void stop(void* ctx)          { (void)ctx; }

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

This demonstrates component state transitions only; it does not configure hardware or show a complete embedded application.

## Examples

`examples/minimal/` exercises the component lifecycle without board-specific peripherals. Other example folders describe hardware scenarios, but their board configuration and driver behavior may be placeholders. Read each example's README before treating it as runnable on a target.

## Current implementation status

The source tree includes useful portable foundations and linkable API surfaces, but many target-dependent drivers are scaffolds. Cortex-M operations still need device-specific implementations and board validation; protocol parsing, sensor behavior, and complete middleware are also unfinished. UCON algorithms, public types, metadata, and their generation contract are still being designed. A declared function or module does not by itself mean its behavior is implemented.

See [`ARCHITECTURE.md`](ARCHITECTURE.md) for the source-to-contract status map and [`TODO.md`](TODO.md) for the implementation sequence.
