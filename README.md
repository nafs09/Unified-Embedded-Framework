# Embedded Framework Projects

This repository contains two standalone projects for building embedded firmware:

- [`UEF/`](UEF/) is a C11 embedded software framework with portable interfaces for hardware abstraction, peripheral access, middleware, protocols, and application lifecycle.
- [`uef-gen/`](uef-gen/) is a Python tool that validates a project configuration, resolves a target and its resources, selects framework modules, and assembles a project directory.

The generator packages a UEF source snapshot under `uef-gen/src/uef_gen/uef_source`. This lets the Python package assemble projects from a known snapshot. The maintained framework source is in `UEF/`; refresh the packaged snapshot and its hash when intentionally updating that copy. The two projects can also be used separately: UEF can be added to a firmware build directly, while the generator is a host-side tool that prepares firmware source and build inputs.

## Build UEF

UEF uses CMake 3.20 or newer and a C11 compiler. A host build uses the simulation backend and bare-metal scheduler boundary:

```powershell
cmake -S UEF -B UEF/build -DUEF_TARGET=HOST -DUEF_UOS_BACKEND=BAREMETAL
cmake --build UEF/build
```

To build the optional lifecycle example, add `-DUEF_BUILD_EXAMPLE=ON` during configuration. A Cortex-M build requires the appropriate CMSIS/device include directory and device header; see `UEF/CMakeLists.txt` and `UEF/ARCHITECTURE.md`. FreeRTOS and FatFS integration are opt-in and require external packages.

## Install and use uef-gen

Python 3.11 or newer is required. From the repository root:

```powershell
python -m pip install -e .\uef-gen
uef-gen --config .\uef-gen\examples\project.yaml --output .\build\firmware --work .\build\uef-gen-work
```

The checked-in `generic-reference` example is for exercising configuration and assembly paths only. Its target data is not verified for physical hardware. Build and target deployment commands are scaffolds and currently fail closed; they do not compile or program firmware.

## A typical development sequence

1. Start with `uef-gen/examples/project.yaml` and choose the framework modules and target configuration your project needs.
2. Use the generator to resolve and assemble a project, then review the generated source lists and manifest.
3. Integrate the generated source and any board-specific startup, linker, and vendor files into the firmware build for the real target.
4. When framework source changes, update the generator's packaged snapshot deliberately and verify that its recorded hash matches the packaged files.

The current sample target is not verified device data, and the build/deployment interfaces are not complete. A successful configuration or assembly step does not establish that the output is ready to compile for a board or safe to program onto hardware.

## Repository map

| Path | Contents |
|---|---|
| `UEF/include/uef/` | Public framework headers, grouped by layer |
| `UEF/src/` | C implementations and target/backend-specific code |
| `UEF/templates/` | Framework-owned template material, including the planned UCON area |
| `UEF/examples/` | Minimal and hardware-scenario example slots with status notes |
| `uef-gen/src/uef_gen/config/` | Configuration loading and schema support |
| `uef-gen/src/uef_gen/chips/`, `resolver/` | Target data and resource-resolution code |
| `uef-gen/src/uef_gen/generation/`, `modules/` | Module selection, project assembly, validation, and manifests |
| `uef-gen/src/uef_gen/uef_source/` | Packaged framework snapshot consumed by the generator |
| `uef-gen/src/uef_gen/deploy/` | Future build, artifact, probe, and deployment interfaces |

## Ownership and current status

UEF owns reusable runtime code and is the intended home for UCON algorithms and their reviewed templates. `uef-gen` owns configuration, chip/resource resolution, module selection, assembly, and project provenance. The UCON catalogue and the handoff between framework-owned algorithms and generator assembly are still being designed; generator-side UCON code is transitional material. Do not treat planned catalogue entries as implemented algorithms.

Several low-level APIs have linkable scaffolds without target-backed behavior. Peripheral drivers, verified board data, complete protocol implementations, UCON algorithms, and physical build/deployment support are incomplete. The component READMEs, `ARCHITECTURE.md`, and `TODO.md` in each project describe those boundaries and next steps.

## Project documentation

- [`UEF/README.md`](UEF/README.md) — framework layers, CMake configuration, examples, and implementation status.
- [`UEF/ARCHITECTURE.md`](UEF/ARCHITECTURE.md) and [`UEF/TODO.md`](UEF/TODO.md) — source mapping and framework work plan.
- [`uef-gen/README.md`](uef-gen/README.md) — installation, project generation, snapshot policy, and generator limitations.
- [`uef-gen/ARCHITECTURE.md`](uef-gen/ARCHITECTURE.md) and [`uef-gen/TODO.md`](uef-gen/TODO.md) — generator ownership, implementation status, and migration plan.
