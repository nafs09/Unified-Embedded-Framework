# uef-gen architecture and implementation status

This document maps the uef-gen source tree to its intended role and distinguishes implemented project-assembly behavior from scaffolding. The user-provided uef-gen specification still contains the earlier design in which generator-side modules own UCON IR, templates, and bindings. The current ownership direction is newer: UEF will own UCON algorithms and reviewed templates; uef-gen remains the Python compiler/assembler that consumes UEF's eventual generation contract. This file records that migration boundary without editing the source specification.

## Ownership boundary

```text
NEXUS authoring UI and graph serializer
          │ versioned request/configuration
          ▼
uef-gen: validate target resources, select UEF modules, assemble project
          │ UEF snapshot and project-specific configuration
          ▼
UEF: portable runtime plus UCON algorithms/templates as those designs arrive
```

- UEF owns reusable C/C++ runtime components, UCON algorithm definitions, and the reviewed UCON template library.
- uef-gen owns project configuration loading, chip/resource resolution, UEF module selection, snapshot verification, project assembly, provenance, and the eventual build/deployment workflow.
- NEXUS owns authoring and serialization. Firmware output must not depend on Python or NEXUS at runtime.
- The current generator-side `ir/`, `ucon/`, and algorithm template registries are transitional code. Do not add algorithm equations or make them the authoritative UCON contract. Once UEF publishes its contract, replace/remove those duplicate definitions and update the request schemas and NEXUS bridge together.
- UEF has not yet received the complete UCON algorithm designs. The sparse UEF catalogue is expected and is not evidence of implemented algorithms.

## Generation pipeline

1. Load YAML/JSON configuration and validate its schema.
2. Resolve a target against registered architecture, family, and part data.
3. Check peripheral/pin/AF, DMA, IRQ, clock, memory, task, and capability constraints.
4. Verify the packaged UEF snapshot and select the transitive module closure.
5. Assemble sources, headers, project files, canonical build lists, and provenance in a staging directory.
6. Validate generated paths and source/include references before publishing output atomically.
7. Later, consume UEF's UCON contract to select and assemble the algorithm code that belongs to UEF.
8. Later, build, retain provenance for, discover a matching target for, and deploy verified artifacts.

Steps 1–6 have partial implementations. Chip data is intentionally sparse and marked unverified. The extension and legacy UCON paths do not make the sample target suitable for hardware. Steps 7–8 are not complete.

## Deployment package

`src/uef_gen/deploy` follows Part XIX's ownership boundaries:

| Package | Responsibility | Current state |
|---|---|---|
| `models.py` | Toolchain/build/artifact/probe/progress data types | Types and explicit provenance stubs |
| `build_manager.py` | Toolchain detection, compile, link, image conversion, size report | Fail-closed scaffolds; no subprocess build is enabled |
| `artifact_store.py` | Durable artifact metadata and hash verification | Hash helper plus persistence/validation TODOs |
| `target_discovery.py` | Read-only probe and bootloader enumeration/identity | Interface scaffold; optional host discovery is not implemented |
| `ideployment.py` | Shared backend lifecycle interface | Abstract interface is defined |
| `orchestrator.py` | Progress and safe deployment state machine | Identity/range preflight exists; write sequence intentionally disabled |
| `backends/` | OpenOCD, ST-LINK, pyOCD, J-Link, DFU, UART boot, and TI UniFlash | Complete method surfaces with detailed TODOs |
| `backend_factory.py` | Choose a backend from transport and explicit preference | Construction-only dispatch; does not connect to hardware |

Before enabling a backend, derive the image's address range from the linker/ELF and verified target memory model, require an exact target match, and specify erase geometry and protection semantics. The specification's illustrative sample addresses and broad erase examples are not implementation data. Building or flashing from a GUI must present the selected artifact and physical target before any destructive step.

## Directory map

- `config/`: configuration parsing and schema validation.
- `chips/`: immutable architecture/family/part descriptions and registry.
- `resolver/`: resource and scheduling resolution into template-safe project data.
- `modules/`: transitive UEF module dependency selection.
- `generation/`: staged assembly, validation, build lists, and provenance.
- `templates/`: generated project scaffold and extension registry; UCON-specific dispatch is transitional pending the UEF-owned contract.
- `ir/`, `ucon/`, and `schemas/control_ir.schema.json`: legacy UCON migration surface. Keep stable only until the replacement UEF/NEXUS contract is agreed.
- `snapshot.py` and `uef_source/`: hash-verified packaged UEF release snapshot.
- `deploy/`: toolchain, artifact, physical target, and backend skeletons.

## Review status

The generated firmware is not yet build-ready for a physical target: the sample chip database is a non-verified reference entry, project startup/linker data is incomplete, and deployment methods are stubs. Module resolution and project assembly are useful development scaffolding but should not be described as hardware validation. No compiler invocation or flash write should be added until the corresponding TODOs and user-confirmation flow are implemented.
