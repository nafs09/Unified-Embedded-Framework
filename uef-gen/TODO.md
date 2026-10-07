# uef-gen implementation TODO

The order below keeps configuration/resource resolution separate from target-specific build and deployment work. A method marked with `NotImplementedError` is an interface scaffold, not a partial success path.

## Recommended implementation sequence

The numbered work areas below describe the task inventory. Use this dependency
order to implement them: generation must be reproducible before a compiler can
consume it; build artifacts must be trustworthy before target discovery can
select one; and exact artifact/target matching must be complete before any
write operation is enabled.

### Milestone 0 — maintain input, ownership, and live UEF contracts

1. Keep the versioned hardware configuration, request/response payload,
   NEXUS ControlIR, resolved project, and generated provenance as distinct
   contracts. Record which component owns and validates each field.
2. [x] Remove generator-owned algorithm IR/builders, standalone UCON hardware
   configuration, signal models, and local template dispatch. Keep only a
   generic ControlIR validator and the UEF manifest/template adapter. UEF now
   owns the initial PID/lead-lag contracts and wrappers plus fail-closed source
   scaffolds for every other catalog entry.
3. [x] Require each selectable UEF module to declare its source files and transitive public-header closure; copy only the selected module dependency closure into generated projects.
4. [x] Emit selected system link libraries in the canonical project build list and provenance; link UMATH's `m` library for GNU/Clang builds and omit it for MSVC.
5. [x] Locate the separately maintained UEF checkout at runtime and compute a
   stable fingerprint from its public source, templates, release metadata, and
   manifests. Do not package or refresh a second UEF tree inside this project.
6. Make schema validation and CLI diagnostics stable, path-specific and
   independent of optional packages where the package promises fallback
   behavior.

**Exit gate:** input and response examples validate against committed schemas;
ownership is unambiguous; the live UEF version and manifests agree; ControlIR
requests are routed only through UEF-declared template paths; and every emitted
manual action has UEF provenance.

### Milestone 1 — trusted target data and complete resolution

1. [x] Read UEF-owned family and 38 exact-part chip-base records through
   uef_api.json with schema-version/shape, uniqueness, family-link, source, and fail-closed
   checks. Preserve the distinction between sourced base facts and verified
   generation-ready part resources.
2. Add provenance and verification state to complete architecture/family/part
   records; do not ship guessed pins, clocks, DMA routes, IRQ priorities or
   memory maps.
2. Validate references between chip, package, pins, peripherals, capabilities,
   clocks, DMA and memory before resolving a project.
3. Split or complete the resolver stages for pins/alternate functions,
   peripherals, DMA/DMAMUX, clocks, interrupts, memory and task scheduling.
4. Reject unsupported target/backend combinations before staging any output.
5. Produce one immutable resolved-project model containing chosen values,
   constraints, provenance and diagnostics; templates must consume resolved
   values and cannot make independent hardware guesses.

**Exit gate:** every accepted target field has a source and verification
status; invalid/conflicting resources produce stable diagnostics; resolver
output is deterministic for the same input and data revision.

### Milestone 2 — reproducible project assembly

1. Validate requested UEF modules against the live UEF manifest, dependency
   closure and capability requirements; reject unknown modules and cycles.
2. Select and copy framework files with stable ordering, preserving public
   include paths and declared external dependencies. For a UEF-managed bundle,
   require an exact lock and target profile, then copy only the declared source
   and include paths without network access.
3. Generate project scaffolding, canonical source/header/build lists and
   configuration from the resolved-project model.
4. Verify every generated relative path and include/source reference stays
   within the staged project; record input, chip-data, UEF source-fingerprint and output
   hashes in provenance.
5. Publish atomically only after validation; specify behavior when destination
   already exists, is open, or is on another volume, and never silently
   overwrite user data.
6. [x] Remove transitional generator-side algorithm lowering and template
   selection. Keep UEF's shared ControlIR template manifest authoritative;
   unsupported UCON families remain gated until their UEF algorithms and
   templates are actually designed and implemented.

**Exit gate:** repeated generation from identical inputs produces equivalent
file inventory and provenance; invalid generation leaves prior output intact;
manifest paths and copied sources are reproducible and auditable.

### Milestone 3 — compiler and artifact provenance

1. Choose toolchain discovery/configuration model per supported compiler family
   and ensure compiler, linker and binutils belong to the same installation.
2. Parse generated build lists as data, canonicalize project-local paths,
   validate flags, and invoke tools with argument arrays, bounded output and
   explicit timeout/cancellation.
3. Add compile/link/map/image conversion and actionable per-file diagnostics.
4. Derive flash/RAM ranges and sizes from the linked artifact and verified
   linker/target memory model, not configuration guesses.
5. Store a versioned artifact sidecar with source/project/chip/UEF hashes,
   toolchain identity, build settings, image hashes and linker-derived load
   range; reject tampered or stale artifacts.

**Exit gate:** a generated project for one verified target builds through the
configured toolchain; artifacts can be reproduced and their exact target,
memory range and input provenance are validated before use.

### Milestone 4 — read-only discovery and target identity

1. Implement optional probe enumeration with no write, reset, mode-change or
   erase behavior during discovery.
2. Identify devices through documented read-only probe operations and a
   provenance-backed device-ID database.
3. Match the selected artifact to exact device identity, memory geometry,
   image format and probe serial; refuse ambiguous or partial matches.
4. Add stable progress, cancellation and cleanup contracts before backend
   operations can change device state.

**Exit gate:** discovery is read-only and repeatable; the selected device and
artifact match exactly; unsupported identity is a clear refusal.

### Milestone 5 — guarded deployment

1. Specify each backend's connect, identify, halt, protection inspection,
   erase geometry, program, verify, reset and disconnect lifecycle.
2. Define bounded timeouts and cleanup for every backend state, including
   disconnect/recovery after cancellation or failure.
3. Before enabling erase/program, require an explicit confirmation naming the
   project, artifact hash, exact chip, probe serial and memory range.
4. Keep mass erase, protection changes and recovery separate from ordinary
   deployment. Verify programmed bytes and report the failure stage.
5. Add deploy/flash CLI orchestration only after these guarantees are enforced
   by the shared API, not only by a front-end prompt.

**Exit gate:** no path can program an unverified artifact or ambiguous target;
confirmation precedes destructive actions; cancellation and failures leave
the backend in a documented safe state.

### Milestone 6 — integration and release

1. Version and document machine-facing CLI/request contracts and their exit
   behavior; keep stdout protocol-only when emitting JSON.
2. Integrate generation/build/discovery/deployment through the consumer's
   process boundary with progress, logs, cancellation and typed outcomes.
3. Keep README, architecture, TODO, schemas, CLI help and snapshot metadata
   synchronized for each release.
4. Advertise only target/toolchain/backend combinations that pass the release
   criteria above; separate software scaffolding status from physical-board
   validation status.

**Exit gate:** a supported configuration can move from validated input to
provenanced build artifact and, where enabled, guarded verified deployment
without relying on undocumented machine state.

## 1. Ownership migration: UCON from uef-gen to UEF

- [x] Keep UEF as the single owner of UCON algorithms, public types, catalog metadata, and templates. The direct PID and lead-lag implementations are registered; all remaining keys stay fail-closed until their typed designs are implemented.
- [x] Keep graph authoring and ControlIR serialization in NEXUS. Hardware configuration stays a separate request field; the bridge validates the serialized ControlIR payload and supplies it to UEF's manifest-declared templates.
- [x] Remove generator-side `AlgorithmIR` classes/builders, standalone UCON configuration/signal models, and local template dispatch. The remaining `uef_adapter/` package reads UEF metadata and renders UEF-owned templates.
- [x] Remove duplicate generator-owned UCON types, algorithm code, and templates. `MANIFEST.in` and wheel package data no longer include a UEF source tree.
- [x] Replace packaged snapshot refresh/integrity with runtime UEF location, version/manifest checks, and a deterministic source fingerprint over the live checkout.
- [x] Read UCON/UPROTO dispatch and manual-action declarations from UEF's `registry/control_ir/registry.json` index and per-entry manifests; initial registered protocols include DSHOT, CRSF, SBUS, PPM, MAVLink, and UAVCAN/DroneCAN, and initial UCON entries include PID and lead-lag.
- [x] Return distinct diagnostics when an algorithm key is unregistered, direct-use only, ambiguous through a compatibility alias, or unknown. The unregistered diagnostic includes the UEF-owned per-algorithm path and roadmap priority.
- [x] Validate every UEF unregistered algorithm's individual header/source/module/stage registration at adapter load, and block scaffold-only modules from generated project selection.

## 2. Configuration, target data, and resolution

- [x] Load family and chip-base roadmaps only from UEF’s API-declared registry indexes and per-entry manifests; keep uef-gen free of a duplicate catalogue.
- [x] Read UEF module and ControlIR registries as ID-to-path indexes, validate every referenced detail file, and expand them in memory for existing selection and generation interfaces.
- [x] Expose Phase 0, Phase 1, and later entries with targets list --include-scaffolds; support --uef for an explicit UEF checkout.
- [x] Include Part 3.2/15.1 families and broader STM32/Espressif series as non-generatable metadata. Only exact verified parts may enter ChipDatabase.
- [ ] Replace the generic sample target with part records backed by vendor reference manuals, datasheets, package pinouts, errata, and clock/DMA documentation; retain source citations and verification status per record.
- [x] Validate chip-base registry and record schemas, enforce paths stay within UEF, and reject chip records whose `family_id` is not in the family registry. `jsonschema` is a required runtime dependency; its absence produces an explicit catalogue-load error.
- [ ] Split resource resolution into focused peripheral/pin, DMA, clock, memory, task, and capability modules while preserving one resolved-project contract.
- [ ] Model pin aliases, AF constraints, DMA request routing, interrupt priorities, clock-tree dividers, linker regions, and RTOS scheduling constraints without guessing missing device data.
- [ ] Detect unsupported architectures and RTOS backends before project output is staged.

## 3. Project assembly and provenance

- [ ] Validate that all requested UEF module names and dependencies exist, reject cycles, and preserve stable build-list ordering.
- [ ] Decide whether every selected source/header path should be included in provenance and make the manifest contain enough information to reproduce the exact assembly.
- [ ] Validate copied UEF includes against actual public header paths and external dependency notes.
- [ ] Complete the FreeRTOS bundle metadata with the actual pinned upstream revision and reviewed architecture/compiler profiles; preserve fail-closed behavior until `FreeRTOSConfig.h`, kernel port, heap policy, and required sources are supplied.
- [ ] Ensure generated project diagnostics and `manifest/manual_actions.json` explain each manual driver/protocol/board task without implying that a wrapper makes the target hardware-ready.
- [ ] Confirm atomic output publication semantics on Windows when the destination exists, is open, or is on another volume; preserve the rule that user data is never overwritten.
- [x] Keep generated project scaffolding free of UCON behavior: all ControlIR algorithm outputs are rendered from the live UEF manifest, and unregistered entries fail before staging.

## 4. Build manager and artifact store

- [ ] Implement coherent toolchain discovery for ARM GNU, TI C2000, RISC-V, and host builds; keep all selected compiler/binutils paths within one configured installation.
- [ ] Parse generated list files as data, canonicalize paths under the project directory, validate flags, and invoke subprocesses with argument arrays rather than a shell.
- [ ] Add per-source compile, link, map generation, ELF/HEX/BIN conversion, size parsing, timeout/cancellation, and bounded diagnostic capture.
- [ ] Derive flash and RAM use from ELF sections plus the selected linker script and compare them with verified target budgets.
- [ ] Store artifact/image/project/chip/UEF hashes, build timestamp, toolchain version, and linker-derived load range in a versioned sidecar; reject tampered or stale images.

## 5. Discovery and deployment

- [ ] Implement optional USB/serial discovery with platform-specific dependency handling; device enumeration must not initiate target writes or mode changes.
- [ ] Resolve chip identity from read-only probe operations and verified device-ID databases; require exact artifact/target compatibility.
- [ ] Define a user-visible confirmation step that names the project, artifact hash, probe serial, exact chip, and memory range before erase/program.
- [ ] Implement each backend's connect, identify, halt, protection inspection, erase geometry, program, verify, reset, and disconnect behavior with bounded timeouts and actionable errors.
- [ ] Keep mass erase, option-byte changes, protection removal, and firmware recovery as separate explicit user actions.
- [ ] Implement progress callbacks with stable states and meaningful byte totals; preserve cancellation and cleanup semantics.
- [ ] Add CLI commands for build, artifact listing, target discovery, and deploy only after the underlying operations are complete and the JSON contract for scripting is versioned.
- [ ] Integrate deployment into NEXUS through the subprocess/plugin bridge without blocking the UI thread; display logs, progress, selected target, and failure stage.

## 6. Documentation and release

- [x] Reconcile the generator's ownership and package map with the rule that UEF owns UCON algorithms, public types, metadata and templates.
- [x] Align `README.md`, `ARCHITECTURE.md`, TODO, CLI, schemas, and the request bridge around NEXUS ControlIR input and live UEF template lookup.
- [x] Add the shared UEF ControlIR manifest example and synchronize the adapter with its UPROTO entries, module requirements, outputs, and manual-action records.
- [x] Consume the UEF algorithm catalog in the shared manifest without adding generator-side algorithm types. PID and lead-lag are registered; all other algorithms remain explicitly unregistered with no output templates and individually registered fail-closed file pairs.
- [ ] Keep the adapter schema/diagnostics synchronized as UEF promotes additional typed algorithms or changes the UCON/UPROTO manifest version.
- [ ] Build and test only after requested; this audit has not run a build, test suite, or hardware operation.
