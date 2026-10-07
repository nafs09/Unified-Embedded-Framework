# uef-gen source architecture and implementation status

`uef-gen` is the Python-side configuration resolver and project assembler. It receives hardware configuration and, when called by NEXUS, a separate ControlIR JSON payload. It locates a live UEF checkout and consumes UEF-owned manifests, C/C++ files, and templates. It does not vendor UEF or define UCON algorithms.

## Ownership boundary

```text
NEXUS
  ControlGraphSerialiser → versioned ControlIR JSON
             │
             ▼
uef-gen
  validate contract → resolve hardware → read UEF manifests
  select UEF modules → pass ControlIR/context to declared UEF templates
             │
             ▼
UEF
  reusable firmware source, public API, UCON algorithms and templates
```

- NEXUS owns graph authoring and ControlIR serialization.
- UEF owns reusable firmware behavior, UCON definitions and templates, and the metadata that declares modules and API contracts. Its current registered UCON entries are PID and lead-lag; every unregistered key has an individual fail-closed C pair in its family tree or under `ucon/future/` and is unavailable for generation.
- uef-gen owns the stable request/response bridge, configuration and resource resolution, module closure, project scaffolding, UEF path/version checks, manifest-driven template loading, output validation, and provenance.
- The `uef-gen` Python package contains no UEF source snapshot, algorithm-specific `AlgorithmIR` classes/builders, UCON implementation, or UCON template content. Its `uef_adapter/` code is glue that reads the UEF checkout and supplies raw ControlIR nodes plus resolved project context to UEF-declared templates.
- Firmware output does not depend on Python or NEXUS at runtime.

## Generation sequence

1. Load and validate the hardware configuration and optional versioned request fields.
2. Validate the optional ControlIR document against its bridge schema and version.
3. Resolve the target against the local chip database and check peripherals, pins, DMA, clocks, task timing, memory, and capabilities.
4. Locate UEF through `UEF_PATH`, `uef.path`, or documented paths relative to the project configuration. Check `version.json`, `uef_api.json` and its declared registry indexes, and every referenced registry/detail file.
5. If ControlIR is present, load UEF's `registry/control_ir/registry.json` index and its node/alias/algorithm detail files. Resolve each node by its declared subtype (or type where permitted), then read its domain, required modules, output/template paths, and manual actions from UEF metadata.
6. Copy selected UEF modules into a temporary project stage using each manifest’s declared source files and explicit public-header closure; unrelated UEF headers and implementations are not copied. Copy registry provenance separately. Resolve any UEF-managed source dependency only from its exact lock and matching architecture/compiler profile; do not download it. Render only template paths declared by UEF; project-level board/config/application placeholders are emitted by the generator scaffold. Record selected system libraries in `project_libraries.txt` and build provenance.
7. Validate generated paths, listed sources, local includes and file inventory, write provenance and `manifest/manual_actions.json`, then atomically publish the project.

Steps 1–7 have partial implementations. UEF owns a 65-entry family/series index, a 38-entry exact-part chip-base index, a 305-entry module index, and a ControlIR dispatch index with per-entry files. uef-gen reads the registry paths through `uef_api.json`, validates and expands the details, and stores no duplicate catalogue. Chip-base records capture sourced identity, core, package and headline memory but omit complete resource maps. Six Phase 0 and one Phase 1 families also have incomplete C profile scaffolds. None of these records is a generation-ready ChipSpec or can resolve hardware. Initial UPROTO wrappers and PID/lead-lag UCON wrappers are registered in the shared UEF registry; other UCON entries fail closed with an `algorithm_unregistered` diagnostic. At adapter load, uef-gen checks each unregistered key's individual header, source, module, and stage metadata against the UEF manifests. Its module selector blocks every scaffold-only module from generated firmware. Each selectable module declares its public-header closure in `headers`; generation validates those paths and copies only the selected closure. The source-free `uhal` and `uos` records are dependency-group markers. No algorithm fallback is generated inside `uef-gen`. FreeRTOS is declared as UEF-managed but its lock/profile is intentionally incomplete, so that backend fails closed. Build, artifact inspection, physical target discovery, and deployment are interface scaffolds, not ready-to-use firmware operations.

## UEF checkout and compatibility

The wheel contains only uef-gen code and its request/configuration schemas. UEF is resolved at runtime. `version.json`, the dereferenced module registry, and `uef_api.json` must agree on the UEF release version when those fields are present. `SnapshotManager` computes a deterministic source fingerprint over the live UEF public headers, implementations, templates, registry indexes, detail manifests, target records, and release metadata; generated provenance contains the same registry/detail set. It does not install a second UEF copy into the generator.

FreeRTOS Kernel is a UEF-managed dependency package with a required exact upstream commit and architecture/compiler integration profile. uef-gen copies only the source files and include directories selected by that profile; it never fetches or updates upstream code, and generation fails when the pin, checkout, or profile is missing. The firmware consumer still supplies target-specific `FreeRTOSConfig.h` and board startup integration. FatFS remains consumer-supplied and is not copied by uef-gen.

## Directory map

- `src/uef_gen/config/`: hardware configuration parsing, normalization, and schema validation.
- `src/uef_gen/chips/`: architecture/family/part records and capability lookup.
- `src/uef_gen/resolver/`: peripheral, pin, DMA, clock, memory, task, and resource diagnostics.
- `src/uef_gen/modules/`: dependency-closure selection from UEF's `registry/modules/registry.json` and per-module records, including explicit header-closure requirements.
- `src/uef_gen/registry.py`: safe path resolution and expansion of UEF registry indexes into the stable in-memory shapes used by the generator.
- `src/uef_gen/ir/control_ir.py` and `schemas/control_ir.schema.json`: NEXUS payload boundary validation; no algorithm-specific models.
- `src/uef_gen/uef_adapter/`: UEF API metadata access and manifest-driven rendering of UEF templates.
- `src/uef_gen/templates/`: project-level generated-file models and generic scaffold; it is not a UEF template library.
- `src/uef_gen/generation/`: staged assembly, generated-project validation, and provenance.
- `src/uef_gen/locator.py` and `snapshot.py`: live UEF lookup, version compatibility, fingerprinting, and selected-file copying.
- `src/uef_gen/deploy/`: future toolchain, artifact, probe, and deployment interfaces.
- `schemas/` and `examples/`: request/configuration contracts and hardware-only examples.

## ControlIR-to-UEF template contract

When ControlIR is supplied, the adapter reads `UEF/registry/control_ir/registry.json`, its referenced detail files, and `UEF/uef_api.json`. The index maps each supported ControlIR node key to a detail file that declares its domain, UEF-owned template/output paths, required modules, and manual actions. Shared templates and dispatch policy remain in the index. Template output paths are relative to the generated project and are rejected if they escape it. Each declared manual action is returned as a path-qualified warning and recorded in `manifest/manual_actions.json`; the generator does not infer actions from the node subtype.

The adapter parses the ControlIR envelope and nested graph structure, dispatches each node by its `subtype`/`type`, and builds a stable Jinja context containing the full document, raw node, node path, resolved target/project context, selected module list, and UEF API metadata. Node data such as `model`, `features`, `structural_constants`, `config_params`, `plant_constants`, ports, and execution-group references remains intact in that context for the UEF-owned template to map into generated code. The adapter does not lower graph nodes into generator-owned typed algorithm objects or synthesize equations. If UEF has no entry/template for a requested node, generation stops with a contract diagnostic.

## Verification status

The generated target startup/linker files are placeholders without a verified board profile. Chip data, drivers, and deployment backends still need target-specific implementation and review. Static source/schema inspection is not equivalent to a build, test run, or hardware validation. See [`README.md`](README.md) for installation/CLI guidance and [`TODO.md`](TODO.md) for the implementation sequence.
