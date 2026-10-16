# uef-gen source architecture and implementation status

`uef-gen` is the Python-side project compiler and assembler. Its hardware-configuration path resolves resources and assembles a project scaffold; its standalone `.uef`/`.uefs` frontends parse, validate, and lower source into backend-neutral application/system models. It reads UETD-owned registries, manifests, C/C++ files, and templates at runtime. It does not vendor UETD or define UCON algorithms.

## Ownership boundary

```text
uef-gen
  `.uef` → parse/check → typed application specification → (emitter unfinished)
  hardware config → resolve hardware → select modules → assemble scaffold
             │
             ▼
UETD
  reusable firmware source, public APIs, algorithms, and target/module metadata
```

- UETD owns reusable firmware behavior, UCON definitions, public types, and the metadata that declares modules and API contracts. Its current registered UCON entries are PID and lead-lag; every unregistered key has an individual fail-closed C pair in its family tree or under `ucon/future/` and is unavailable for generation.
- `uef-gen` owns the stable request/response bridge, configuration and resource resolution, module closure, project scaffolding, UETD path/version checks, output validation, and provenance.
- The `uef-gen` Python package contains no UETD source snapshot, algorithm-specific `AlgorithmIR` classes/builders, or UCON implementation. Its UETD adapter reads the live checkout and supplies registry, target, and project context to the generator.
- Firmware output does not depend on Python at runtime.

## Hardware-configuration generation sequence

1. Load and validate the hardware configuration and versioned request fields.
2. Locate UETD through `UETD_PATH`, `uetd.path`, or documented paths relative to the project configuration. Read `api.json` as the canonical version and registry-discovery contract, then validate each declared family, chip, development-board, module, type, unit, and UMATH registry and detail file. A legacy `version.json` is optional and checked only when present.
3. Materialize the selected chip and optional development board from UETD's live registries, then resolve peripherals, pins, DMA, clocks, task timing, memory, and capabilities. No chip or board facts are compiled into the Python package.
4. Copy selected UETD modules into a temporary project stage using each manifest's declared source files and explicit public-header closure; unrelated UETD headers and implementations are not copied. Copy registry provenance separately. Resolve any UETD-managed source dependency only from its exact lock and matching architecture/compiler profile; do not download it. Project-level board/config/application placeholders are emitted by the generator scaffold. Record selected system libraries in `project_libraries.txt` and build provenance.
5. Validate generated paths, listed sources, local includes and file inventory, write provenance and `manifest/manual_actions.json`, then atomically publish the project.

The hardware-configuration path implements validation, registry loading, resource resolution, module selection, staged assembly, manifest/provenance output, and generated-file validation. It emits a project scaffold, not a verified build. The standalone `.uef` frontend currently produces an application specification, and `.uefs` produces a system plan; neither has an emitter. Build, artifact verification, physical target discovery, deployment, and wired monitoring are unavailable. UETD owns target/module facts; the remaining work inside `uef-gen` is listed in [`TODO.md`](TODO.md).

## UETD checkout and compatibility

The wheel contains only uef-gen code and its request/configuration schemas. UETD is resolved at runtime. `api.json` owns the UETD release version and registry paths; registry indexes and manifests are checked against it. A legacy `version.json` is optional and, if present, must agree. `SnapshotManager` computes a deterministic source fingerprint over the live UETD public headers, implementations, templates, registry indexes, detail manifests, target records, and release metadata; generated provenance contains the same registry/detail set. It does not install a second UETD copy into the generator.

FreeRTOS Kernel is a UETD-managed dependency package with a required exact upstream commit and architecture/compiler integration profile. `uef-gen` copies only the source files and include directories selected by that profile; it never fetches or updates upstream code, and generation fails when the pin, checkout, or profile is missing. The firmware consumer still supplies target-specific `FreeRTOSConfig.h` and board startup integration. FatFS remains consumer-supplied and is not copied by `uef-gen`.

## Directory map

- `src/uef_gen/config/`: hardware configuration parsing, normalization, and schema validation.
- `src/uef_gen/chips/`: architecture/family/part records and capability lookup.
- `src/uef_gen/resolver/`: peripheral, pin, DMA, clock, memory, task, and resource diagnostics.
- `src/uef_gen/modules/`: dependency-closure selection from UETD's `registry/modules/registry.json` and per-module records, including explicit header-closure requirements.
- `src/uef_gen/uetd_adapter/`: UETD checkout lookup/version checks, registry and snapshot handling, and API metadata access.
- `src/uef_gen/templates/`: project-level generated-file models and generic scaffold; it is not a UETD template library.
- `src/uef_gen/generation/`: staged assembly, generated-project validation, and provenance.
- `src/uef_gen/language/frontends/`: source-language frontends, grouped separately from shared registry and LSP services.
  - `project/`: standalone `.uef` lexer, parser, semantic/type verifier, and application-spec lowering.
  - `system/`: `.uefs` parser, cross-project/link validation, and deterministic system-plan lowering.
- `src/uef_gen/language/registry/`: language grammar metadata and bounded builtin dispatch, not a UETD module catalogue.
- `src/uef_gen/language/server/`: shared LSP adapters for compiler diagnostics, registry hints, and navigation.
- `src/uef_gen/toolchains/`: UETD-owned host package selection and provisioning interfaces; install/profile resolution remain incomplete.
- `src/uef_gen/deploy/`: future toolchain, artifact, probe, deployment, and wired-console interfaces.
- `schemas/` and `examples/`: request/configuration contracts and hardware-only examples.

## V1 CLI diagnostics and wired runtime output

The intended V1 user-facing debug scope is text diagnostics in the command-line tools and a live text console for firmware running on a target connected by cable. CLI diagnostics cover the generator's own operations and errors; they are not a source-level debugger. When a command emits the versioned JSON bridge response, stdout stays machine-readable and human diagnostics go to stderr.

The live console attaches to a running target through a wired board interface, such as USB CDC or a USB-to-UART bridge. The development-board record in UETD is the source of truth for the console interface and settings; uef-gen must not invent a port, baud rate, or board behavior. Attaching to the console is observational: it must not flash, erase, or implicitly reset the target. Wireless transports are outside V1 and may be added in a later release. This scope is planned; the CLI diagnostic contract and wired console are not implemented yet.

## Verification status

The generated target startup/linker files are placeholders without a verified board profile. Chip data, drivers, deployment backends, CLI diagnostics, and the wired runtime console still need implementation and target-specific review. See [`README.md`](../README.md) for installation/CLI guidance and [`TODO.md`](TODO.md) for the implementation sequence.
