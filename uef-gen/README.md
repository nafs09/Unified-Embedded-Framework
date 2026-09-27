# uef-gen

`uef-gen` is a Python tool for assembling embedded projects from a hardware configuration and a packaged UEF source snapshot. It validates the input, resolves target and resource data, selects the required framework modules, and stages a project directory with source files and build manifests. The generated firmware has no Python runtime dependency.

## Requirements and installation

- Python 3.11 or newer
- PyYAML and jsonschema (installed as package dependencies)

Install in editable mode from this folder:

```powershell
python -m pip install -e .
```

Jinja2 template support and hardware-discovery packages are optional extras declared in `pyproject.toml`.

## Generate a project

```powershell
uef-gen --config .\examples\project.yaml --output .\build\firmware --work .\build\uef-gen-work
```

Use the JSON equivalent with `--config .\examples\project.json` where YAML is not desired. The output directory receives the assembled project; the work directory is used for staging. Configuration schemas are in `schemas/`.

The sample configuration selects the `uapp` module for a `generic-reference` target and uses `float32` arithmetic with a bare-metal RTOS setting. A real project should add only modules it needs and use target records validated against vendor documentation. The `--control-ir` option can supply the older transitional control representation, but its UCON handoff is not yet the final framework contract.

The included example targets `generic-reference` and is intended to exercise configuration and assembly only. Its chip, pin, clock, DMA, and memory information is not verified hardware data. Do not use it to build or deploy firmware for a physical device.

## What the generator does

1. Loads and validates YAML or JSON configuration.
2. Resolves architecture, family, and part information from the chip database.
3. Checks resource requests such as pins, peripherals, DMA, interrupts, clocks, and memory.
4. Selects the transitive module set using the packaged framework manifest.
5. Verifies the packaged framework snapshot and assembles project files in a staging area.
6. Validates generated paths and references before publishing output and recording provenance.

The canonical build lists are the generated `.txt` files. Make and CMake fragments are convenience files that consume those lists.

The process can also read a versioned JSON request from a file or standard input. The request and response shapes are documented by `schemas/request.schema.json` and `schemas/response.schema.json`; a successful response includes diagnostics and a manifest of generated files. This mode is useful for scripting a generation step without importing the Python package into another application.

## Framework snapshot and ownership

`src/uef_gen/uef_source/` contains the packaged UEF snapshot used for project assembly. The maintained framework source lives in the sibling `UEF/` project. When updating the packaged copy, refresh the snapshot contents and hash together.

UEF owns reusable runtime code and is the intended home for UCON algorithms and reviewed templates. `uef-gen` owns configuration, target/resource resolution, module selection, project assembly, and provenance. The UCON handoff is still being designed. The generator's existing algorithm IR, bindings, and template dispatch are transitional; they are not the source of truth for future UCON algorithms.

## Build and target commands

The package exposes build, probe, deploy, and flash command shapes, but those operations are scaffolds that fail closed. They do not compile firmware, discover/program a physical target, or flash a device. Target data is sparse and must be backed by verified vendor documentation before use on hardware.

See [`ARCHITECTURE.md`](ARCHITECTURE.md) for module responsibilities and implementation status, and [`TODO.md`](TODO.md) for migration and completion work.

## Python package and extensions

The package is built from `src/` and exposes the `uef-gen` console command. Optional dependency groups in `pyproject.toml` provide Jinja2 template support and hardware-discovery libraries. Python extensions can register chip specifications and temporary legacy node/template builders through the `uef_gen.extensions` entry point; duplicate registrations are rejected. These extension points do not transfer ownership of reusable algorithms from the framework to the generator.
