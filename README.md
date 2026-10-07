# UEF and uef-gen

This repository contains two separately maintained projects that cooperate to assemble embedded firmware:

- [`UEF/`](UEF/) is the C11 framework. It owns portable runtime source, public APIs, module/API metadata, UCON algorithms, and all reusable templates.
- [`uef-gen/`](uef-gen/) is the Python host tool. It resolves hardware configuration, selects UEF modules, builds a project scaffold, consumes UEF's live manifest, and renders declared templates.

NEXUS owns graph editing and serializes its ControlGraph as ControlIR JSON. uef-gen validates and transports that JSON to templates owned by UEF. Generated firmware does not depend on NEXUS or Python at runtime.

## UCON ownership and current coverage

1. NEXUS authors the control graph and serializes graph details into ControlIR.
2. UEF owns UCON public types, generic implementations, algorithm metadata, and templates.
3. uef-gen reads the live UEF checkout and remains algorithm agnostic; it does not contain UCON formulas, algorithm classes, or a local template registry.

UEF currently provides direct C implementations and registered thin generated wrappers for PID and lead-lag. Every other Part XVI catalog algorithm has an individual fail-closed header/source pair nested in its family folder. First-pass entries and selected planned algorithms relevant to common filtering, the drone, QCWDRSSTC, and medical dosing live beside the implemented APIs; remaining planned and all deferred entries live under `UEF/include/uef/ucon/future/` and `UEF/src/ucon/future/`. The shared manifest and module map keep every placeholder unavailable to generated firmware until typed APIs and behavior are implemented and reviewed. See [`UEF/templates/ucon/README.md`](UEF/templates/ucon/README.md).

## Phase 0 and Phase 1 scaffold coverage

UEF has incomplete target-family profile records for STM32G4xx, STM32F4xx, single-core STM32H7xx, STM32F7xx, STM32G0xx, and nRF52840. uef-gen exposes a matching roadmap separately from exact chip records. These names do not include verified package pins, clocks, DMA routes, memory maps, startup files, or linker scripts and cannot be used as board-ready targets.

Phase 1 source/API scaffolds cover raw SDMMC access, the optional consumer-supplied FatFS diskio layer, and a separate FDCAN-FD API. Their target operations still fail closed. The UPROTO set includes DSHOT, CRSF, SBUS, PPM, MAVLink transport, and UAVCAN/DroneCAN transport; module/template presence does not claim target readiness.

## Build UEF

Requirements: CMake 3.20+ and a C11 compiler. A host build exercises portable code and host simulation; it is not MCU qualification.

```powershell
cmake -S UEF -B UEF/build -DUEF_TARGET=HOST -DUEF_UOS_BACKEND=BAREMETAL
cmake --build UEF/build
```

To build the lifecycle example, add `-DUEF_BUILD_EXAMPLE=ON` during configuration. A Cortex-M build requires the selected CMSIS/device include directory and device header. FreeRTOS fails closed until its exact source pin, license files, architecture/compiler profile, and project `FreeRTOSConfig.h` are supplied. FatFS is optional and consumer supplied.

## Install and use uef-gen

Python 3.11 or newer is required. Install with template support:

```powershell
Push-Location .\uef-gen
python -m pip install -e ".[templates]"
Pop-Location
$env:UEF_PATH = (Resolve-Path .\UEF).Path
```

Inspect target records and the Phase 0 family roadmap:

```powershell
uef-gen targets list
uef-gen targets list --include-scaffolds
```

The roadmap entries are not exact `ChipSpec` targets. Validate or inspect the hardware-only example:

```powershell
uef-gen validate .\uef-gen\examples\project.yaml
uef-gen generate .\uef-gen\examples\project.yaml --output .\build\firmware --dry-run
```

Generate a project with NEXUS ControlIR when the selected node subtypes are registered in UEF's live `registry/control_ir/registry.json` index and its referenced detail manifests:

```powershell
uef-gen generate .\uef-gen\examples\project.yaml `
  --control-ir .\build\control-ir.json `
  --output .\build\firmware `
  --work .\build\uef-gen-work
```

Currently, PID and lead-lag UCON nodes and the initial UPROTO entries are registered. Other UCON keys return `algorithm_unregistered` and do not publish output. UEF-declared manual actions are written to the response and generated provenance.

## Read next

- [`UEF/README.md`](UEF/README.md), [`UEF/ARCHITECTURE.md`](UEF/ARCHITECTURE.md), and [`UEF/TODO.md`](UEF/TODO.md) describe C APIs, readiness, and implementation work.
- [`uef-gen/README.md`](uef-gen/README.md), [`uef-gen/ARCHITECTURE.md`](uef-gen/ARCHITECTURE.md), and [`uef-gen/TODO.md`](uef-gen/TODO.md) describe the JSON bridge, project assembly, and incomplete build/deployment work.
