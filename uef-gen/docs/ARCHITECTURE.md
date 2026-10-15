# uef-gen architecture and implementation status

`uef-gen` resolves UETD hardware and module metadata, validates project inputs,
and assembles a firmware project scaffold. The `.uef` and `.uefs` frontends
parse and lower declarative project sources into backend-neutral models; their
firmware emitters are not implemented yet.

## Ownership

- UETD owns embedded C/C++ source, public APIs, target and module registries,
and module manifests.
- `uef-gen` owns hardware/resource resolution, module selection, project
scaffolding, source-language frontends, output validation, and provenance.
- Generated firmware uses the copied project files and has no Python runtime
dependency.

## Hardware-configuration assembly

1. Load and validate the hardware configuration or versioned JSON request.
2. Locate UETD through `UETD_PATH`, `uetd.path`, or documented project-relative
locations, then verify its API metadata and registry files.
3. Resolve the target, board, peripherals, pins, DMA, clocks, memory, and
capabilities from UETD records.
4. Select the module dependency closure and any locked source dependencies.
5. Copy declared sources, public headers, and registry provenance into a
staging directory; render the project scaffold.
6. Validate paths, source lists, includes, and generated-file inventory, then
publish the completed project atomically.

The hardware-configuration workflow emits a scaffold, not a verified build.
Startup code, linker settings, board initialization, toolchain configuration,
and target behavior require separate implementation and validation.

## Source-language frontends

The `.uef` frontend validates and lowers one target-local application into an
application specification. The `.uefs` frontend composes projects and typed
links into a multi-target system plan. Neither frontend currently has a
firmware emitter. Build, artifact verification, deployment, and wired target
monitoring are also incomplete.

## Package map

- `src/uef_gen/config/`: hardware configuration parsing and validation.
- `src/uef_gen/chips/` and `resolver/`: target lookup and resource resolution.
- `src/uef_gen/modules/`: module dependency selection and bundled dependencies.
- `src/uef_gen/generation/`: staged assembly, validation, and provenance.
- `src/uef_gen/language/`: project/system parsers, lowering, registries, and LSP.
- `src/uef_gen/uetd_adapter/`: UETD location, registry, API, and snapshot access.
- `src/uef_gen/templates/`: generated-file models and project scaffold.
- `schemas/` and `examples/`: request contracts and hardware-only examples.

See [`TODO.md`](TODO.md) for current implementation work and the
[UEF-PL handbook](../../docs/guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md) for the
standalone project language.
