# uef-gen

`uef-gen` is a standalone Python project-construction tool. It has no NEXUS
imports and can be driven by a project YAML file, JSON request, CI job, or the
NEXUS subprocess bridge. Generated firmware has no Python or NEXUS runtime
dependency. The code tree is a scaffold: resource resolution and project
assembly have implementations, while verified target builds and deployment
remain future work.

## Project generation

Generation is staged and stops before publishing output when configuration,
chip resolution, resource allocation, module selection, snapshot verification,
or generated-file validation fails.

1. Load YAML/JSON hardware configuration and validate `project.schema.json`.
2. Resolve the target through the architecture/family/part `ChipDatabase`.
3. Resolve pin/AF, peripheral, DMA/DMAMUX, IRQ-priority, clock, memory, and
   scheduler allocations. Templates receive resolved values, not raw guesses.
4. Normalize UEF layer aliases to granular modules, select the dependency
   closure from `uef_modules.json`, and check required capabilities.
5. Verify the packaged UEF snapshot hash, copy selected sources, and copy the
   shared public include tree once.
6. Assemble generated project scaffolding and canonical build lists. The
   existing ControlIR/AlgorithmIR and UCON signal-binding path is transitional.
   UEF is becoming the owner of UCON algorithms and reviewed templates;
   uef-gen will consume that UEF-owned contract to assemble projects. Do not add
   equations, algorithm-specific type definitions, or duplicate template
   bodies to uef-gen while the UEF algorithms and handoff contract are being
   designed.
7. Validate output paths, listed sources, and project-local include references,
   then publish the completed project atomically.
8. Write a provenance manifest containing the verified UEF snapshot hash and
   resolved resources.

The `.txt` build lists are the canonical outputs. `config/project.mk` and
`config/project.cmake` are convenience fragments that consume those lists.

The deployment interfaces follow the file names in Part XIX of the current
specification, including `deploy/backend_factory.py`. Build and hardware
operations remain fail-closed scaffolds until their TODOs are implemented.

## CLI and request contract

Standalone project mode:

```powershell
uef-gen --config project.yaml --output .\build\firmware --work .\build\uef-gen-work
```

`examples/project.json` is the same reference configuration in JSON for
environments that do not use PyYAML.

The NEXUS contract mode reads one JSON request from stdin (or `--request
request.json`) and writes exactly one JSON response to stdout. The request
schema is in `schemas/request.schema.json`. The checked-in ControlIR 1.0 schema
and its consumers belong to the earlier UCON design. Reconcile them with the
finalized UEF/uef-gen UCON handoff before treating the bridge payload as final;
the serialized graph/configuration ownership is still to be agreed.

`examples/project.yaml` uses `generic-reference` only to exercise project
assembly. It contains no verified silicon, pin, clock, DMA, or memory data and
must not be used as a physical target. Production part data must come from
verified vendor documentation. `ChipCapabilities` merges part, family, and
architecture properties so templates can avoid chip-name branching.

The legacy generator-side `ir/`, `ucon/`, and algorithm `TemplateSet` modules
remain as migration material. They are not the source of truth for UCON
algorithms. Replace their duplicate responsibilities after UEF publishes its
algorithm, parameter, signal, template metadata, and generation contract.

## Extensions

Install a Python distribution exposing a `uef_gen.extensions` entry point to
register verified chip specs and, temporarily, legacy `NodeIRBuilder` and
`TemplateSet` extensions. Do not add new algorithm implementations to those
registries. Duplicate registration is rejected, missing builders/templates
produce diagnostics, and generated artifacts must use unique relative paths.

## UEF snapshot

UEF release sources and its current UCON catalogue are copied under
`src/uef_gen/uef_source`. The content hash excludes only `snapshot.json` and
covers source, public headers, templates, API descriptions, and the module
manifest. Refresh the packaged snapshot and its hash together for each
intentional UEF release update.

## Build and deployment

`src/uef_gen/deploy` mirrors the specification's toolchain, build manager,
artifact store, target discovery, orchestration, and backend boundaries. These
are fail-closed scaffolds; they do not currently compile firmware or modify a
connected target. Deployment refuses to proceed without verified linker-derived
flash bounds. Treat the package as an implementation map, not a working flasher.

See [ARCHITECTURE.md](ARCHITECTURE.md) for component ownership and
[TODO.md](TODO.md) for the implementation sequence and migration work.
