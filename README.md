# Unified Embedded Framework (UEF)

The Unified Embedded Framework (UEF) consists of two coordinated components:
UETD (the Unified Embedded Template Directory) and `uef-gen`, its host-side
Python project assembler:

- [`UETD/`](UETD/) contains the C11 framework source, public APIs, module
  metadata, control algorithms, registries, and reusable templates.
- [`uef-gen/`](uef-gen/) resolves hardware configuration, selects UETD modules,
  assembles project scaffolds, reads UETD's live registries, and renders its
  declared templates.

UEF is licensed under GNU GPL-3.0-only; see [`LICENSE.md`](LICENSE.md).
Dependencies retain their own license terms.

Generated firmware does not require the host-side Python tool at runtime.


## Public API and current coverage

UETD owns its public C APIs, implementations, metadata, and reusable templates.
`uef-gen` reads the live UETD checkout and resolves public hardware and module
configuration; it does not maintain a second copy of UETD's catalogues.

PID and lead-lag have generic C implementations available for direct use.
Other catalogue algorithms are scaffolds and must not be treated as implemented
or ready for firmware. See [`UETD/README.md`](UETD/README.md) for framework status
and [`UETD/include/ucon/README.md`](UETD/include/ucon/README.md) for
the direct-use API and scaffold status.

## Phase 0 and Phase 1 scaffold coverage

UETD has incomplete target-family profile records for STM32G4xx, STM32F4xx, single-core STM32H7xx, STM32F7xx, STM32G0xx, and nRF52840. uef-gen exposes a matching roadmap separately from exact chip records. These names do not include verified package pins, clocks, DMA routes, memory maps, startup files, or linker scripts and cannot be used as board-ready targets.

Phase 1 source/API scaffolds cover raw SDMMC access, the optional consumer-supplied FatFS diskio layer, and a separate FDCAN-FD API. Their target operations still fail closed. The UPROTO set includes DSHOT, CRSF, SBUS, PPM, MAVLink transport, and UAVCAN/DroneCAN transport; module/template presence does not claim target readiness.

## Build UETD

Requirements: CMake 3.20+ and a C11 compiler. A host build exercises portable code and host simulation; it is not MCU qualification.

```powershell
cmake -S UETD -B UETD/build -DUETD_TARGET=HOST -DUETD_UOS_BACKEND=BAREMETAL
cmake --build UETD/build
```

To build the lifecycle example, add `-DUETD_BUILD_EXAMPLE=ON` during configuration. A Cortex-M build requires the selected CMSIS/device include directory and device header. FreeRTOS fails closed until its exact source pin, license files, architecture/compiler profile, and project `FreeRTOSConfig.h` are supplied. FatFS is optional and consumer supplied.

## Install and use uef-gen

Python 3.11 or newer is required. Install with template support:

```powershell
Push-Location .\uef-gen
python -m pip install -e ".[templates]"
Pop-Location
$env:UETD_PATH = (Resolve-Path .\UETD).Path
```

Inspect target records and the Phase 0 family roadmap:

```powershell
uef-gen targets list
uef-gen targets list --include-scaffolds
```

The roadmap entries are not exact `ChipSpec` targets. Inspect available
records, then validate a hardware configuration file that you provide (the
repository does not currently include a hardware-only YAML example):

```powershell
uef-gen targets show <chip-id>
uef-gen validate .\path\to\hardware.yaml
uef-gen generate .\path\to\hardware.yaml --output .\build\firmware --dry-run
```

The hardware configuration shape is defined by
[`uef-gen/schemas/project.schema.json`](uef-gen/schemas/project.schema.json).

The hardware-configuration workflow can assemble a project scaffold, but the
catalogue's physical targets are not yet generation-ready. A successful
scaffold is not a verified or flash-ready firmware image. See the
[`uef-gen` README](uef-gen/README.md) for supported commands and current limits.

## Standalone project language (scaffold)

The planned `.uef` language is a typed, declarative authoring surface for
standalone UETD applications. It describes exact targets, numerical profiles,
records and enums, hardware bindings, sensors and actuators, protocols, models,
algorithms, subsystem composition, and an operating-system root with tasks and
timing contracts. A separate `.uefs` format composes multiple target projects.
The standalone language parser, verifier, lowering, and generation routes
remain scaffold work. See the
[UEF-PL user handbook](docs/guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md) for a
first-time walkthrough, the [technical guide](docs/guides/UEF_PROJECT_LANGUAGE_GUIDE.md)
for detailed language rules, and the [language design](docs/design/PROJECT_LANGUAGE_DESIGN.md)
for the compiler contract. The [registry/discovery guide](docs/guides/UEF_PL_REGISTRIES.md)
explains the JSON catalogue hierarchy and how UETD names and types reach the
language. See the [generated module/type/UMATH catalogue](docs/reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md)
and [dimension-grouped unit reference](docs/reference/UEF_PL_UNITS_REFERENCE.md)
for lookup tables.

## Read next

- [UEF-PL user handbook](docs/guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md) is the
  design walkthrough for the standalone `.uef` route; the frontend and
  generation pipeline remain scaffolds.
- [UEF-PL registries and discovery](docs/guides/UEF_PL_REGISTRIES.md) explains
  the JSON files, descriptor lookup, and UETD discovery order.

- [Repository documentation](docs/README.md) indexes cross-project design, plans, and release preparation.
- [UETD](UETD/README.md) and [UETD documentation](UETD/docs/README.md) describe the embedded framework component, architecture, catalogues, and implementation work.
- [uef-gen](uef-gen/README.md) and [uef-gen documentation](uef-gen/docs/README.md) describe project assembly, the language frontend, and implementation work.
- [Contributing](CONTRIBUTING.md), [Code of Conduct](CODE_OF_CONDUCT.md), [Security](SECURITY.md), and [Support](SUPPORT.md) explain the planned community process.
- [Changelog](CHANGELOG.md) records user-visible changes as releases are prepared.
