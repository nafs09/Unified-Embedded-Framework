# uef-gen implementation TODO

The order below keeps configuration/resource resolution separate from target-specific build and deployment work. A method marked with `NotImplementedError` is an interface scaffold, not a partial success path.


## Source map — where the implementation work lives

Use this map to move from an observed command or missing behavior to the file
that owns it. UETD owns target facts, module contracts, templates, and package
pins. `uef-gen` owns resolution, file generation, host process orchestration,
artifact validation, and transports; it must not grow a second target or
tool-version database.


The corresponding UETD source map is in `UETD/src/uhal/README.md` and
`UETD/templates/targets/README.md`. Start chip/board/tool registration in
`UETD/registry/targets/README.md` and
`UETD/registry/targets/toolchains/README.md`; do not add Python constants to
`chips/` or `deploy/backends/` as a substitute for UETD registry data.

| Work area | Main source files | Function/class entry points and responsibility |
| --- | --- | --- |
| UETD discovery and API contract | `src/uef_gen/uetd_adapter/locator.py`, `snapshot.py`, `registry.py`, `src/uef_gen/chips/registry_records.py`, `development_boards.py`, `src/uef_gen/toolchains/catalog.py` | `UETDLocator.locate()` follows UETD API declarations; `SnapshotManager.verify()` fingerprints and validates one UETD checkout; loaders read the registries declared by `api.json`. |
| Configuration and resource resolution | `src/uef_gen/config/schema.py`, `loader.py`, `normalize.py`, `src/uef_gen/resolver/resource_resolver.py`, `models.py` | Validate input, normalize stable forms, resolve chip/board/pins/clocks/peripherals/DMA/memory into one resolved project, and attach diagnostics instead of guessing. |
| Module and dependency selection | `src/uef_gen/modules/selector.py`, `planner.py`, `bundled.py`, `src/uef_gen/uetd_adapter/api_database.py` | Validate requested module IDs; close transitive source/header dependencies; select UETD-managed packages and external dependencies. |
| Standalone project generation | `src/uef_gen/generation/pipeline.py`, `validator.py`, `manifest.py`, `src/uef_gen/templates/project_scaffold.py`, `uetd_library.py` | `generate_project()` owns validate → resolve → stage → render → validate → publish. `render_project_scaffold()` emits project shell and TODO placeholders. Manifest builders record resolved data, hashes, modules, and manual actions. |
| Build file and compiler pipeline | `src/uef_gen/deploy/build_manager.py`, `models.py`, `src/uef_gen/toolchains/catalog.py`, `bootstrap.py`, `profiles.py`, `cli.py` | Build-manager methods dispatch to supported build routes; shared path/flag/process helpers and compile/link/objcopy/size steps are separate. Package installation and verification remain explicit scaffolds until pins and safe extraction are complete. |
| Artifact identity and staleness | `schemas/artifact.schema.json`, `src/uef_gen/deploy/models.py`, `artifact_store.py`, `src/uef_gen/generation/manifest.py` | Artifact records must capture image role/hash/format/address. Populate them from linker/SDK metadata and reject modified inputs/images and overlapping/out-of-range segments. |
| Read-only probe discovery | `src/uef_gen/deploy/target_discovery.py`, `backend_factory.py`, `models.py` | Enumerate endpoints without changing target state, read silicon identity, and select only the board profile's declared backend. |
| Flash lifecycle and transports | `src/uef_gen/deploy/orchestrator.py`, `ideployment.py`, `src/uef_gen/deploy/backends/` | `DeploymentOrchestrator.deploy()` owns the safety sequence. Backends implement bounded connect, identify, halt, protection inspection, erase, program, verify, reset, disconnect, and read-only identity operations. |
| Wired command-line monitor | `src/uef_gen/deploy/monitor.py`, `cli.py`, `src/uef_gen/chips/development_boards.py` | Board metadata describes console framing; the monitor owns optional port listing, attach/read/decode/cleanup and must not reset, erase, or flash. |
| User-facing command routing | `src/uef_gen/cli.py`, `src/uef_gen/deploy/cli.py`, `src/uef_gen/toolchains/cli.py`, `src/uef_gen/diagnostics.py`, `src/uef_gen/contract.py` | Route generation, tools, build/deploy/flash/probe/monitor commands; JSON mode keeps stdout machine-readable and sends human diagnostics to stderr. |

## Recommended implementation sequence

The numbered work areas below describe the task inventory. Use this dependency
order to implement them: generation must be reproducible before a compiler can
consume it; build artifacts must be trustworthy before target discovery can
select one; and exact artifact/target matching must be complete before any
write operation is enabled.

## Declarative standalone project language

The full design and syntax proposal are in `../../docs/design/PROJECT_LANGUAGE_DESIGN.md`.

Keep the standalone project compiler and the hardware-configuration assembler
as separate input workflows with explicit ownership and validation contracts.

- [x] Implement the lexer, recursive-descent parser, immutable AST/source spans, local error recovery, and public `.uef` compiler API. All checked-in `.uef` and `.uefs` examples parse; current example failures are semantic/registry compatibility issues, not grammar failures.
- [x] Keep syntax productions and dispatch IDs in `src/uef_gen/language/registry/`; resolve UETD names and contracts from UETD's live registries.
- [x] Discover module spellings through `DSL_descriptor`, load available authoring contracts, and report registered-but-unready entries.
- [x] Implement project symbols, type/unit/shape checks, model/algorithm/protocol/FSM/OS validation, and lowering to a backend-neutral application specification. Composition output and branch contracts, scheduled-call arguments, state actions, nested UMATH restrictions, and exact registry-backed UMATH routes are preserved and checked.
- [ ] Reconcile the basic, platform, drone, ECHO, rover, optical-bench, and Tesla examples with the current UETD authoring contracts and type/unit catalogue; they parse, but none currently passes semantic compilation against the live checkout.
- [x] Implement `.uefs` parsing, node compilation, shared link-contract validation, and deterministic system-plan lowering.
- [ ] Implement the standalone `.uef` emitter from the lowered application specification and UETD-owned generation contracts.
- [ ] Implement the `.uefs` firmware emitter after single-target emission, preserving independent node builds and link metadata.
- [ ] Complete source-to-generated diagnostic maps and generated-project validation once an emitter exists.
- [ ] Add human-readable CLI summaries while retaining machine-readable JSON output for scripts and the existing request bridge.
- [ ] Route `.uef` and `.uefs` through build/flash only after generation, toolchain resolution, artifact validation, and safe deployment work end-to-end.
- [x] Connect registry-backed completion, hover, signature help, definitions, compiler diagnostics, and UTF-16 LSP coordinates to the shared editor server.
- [ ] Expand grammar-aware completion, project navigation/references, semantic tokens, formatting, code actions, and editor package release validation.

### Milestone 0 — maintain input, ownership, and live UETD contracts

1. Keep the versioned hardware configuration, request/response payload,
   resolved project, and generated provenance as distinct contracts. Record
   which component owns and validates each field.
2. [x] Require each selectable UETD module to declare its source files and transitive public-header closure; copy only the selected module dependency closure into generated projects.
3. [x] Emit selected system link libraries in the canonical project build list and provenance; link UMATH's `m` library for GNU/Clang builds and omit it for MSVC.
4. [x] Locate the separately maintained UETD checkout at runtime and compute a
   stable fingerprint from its public source, templates, release metadata, and
   manifests. Do not package or refresh a second UETD tree inside this project.
5. Make schema validation and CLI diagnostics stable, path-specific and
   independent of optional packages where the package promises fallback
   behavior.

**Exit gate:** input and response examples validate against committed schemas;
ownership is unambiguous; the live UETD version and manifests agree; generated
outputs carry reproducible provenance and manual-action records.


### Milestone 1 — trusted target data and complete resolution

1. [x] Read UETD-owned family and exact-part chip records through `api.json`
   with schema-version/shape, uniqueness, family-link, source, and fail-closed
   checks. Materialize records into the resolver without a Python-side target
   catalogue; per-record readiness gates physical generation.
2. [x] Add an API-declared development-board registry with schema validation,
   exact `chip_id` links, inline module facts, board pin reservations, and
   build/flash profile fields. Permit board-only project selection.
3. Add provenance and verification state to complete architecture/family/part
   records; do not ship guessed pins, clocks, DMA routes, IRQ priorities or
   memory maps.
4. Validate references between chip, package, pins, peripherals, capabilities,
   clocks, DMA and memory before resolving a project.
5. Split or complete the resolver stages for pins/alternate functions,
   peripherals, DMA/DMAMUX, clocks, interrupts, memory and task scheduling.
6. Reject unsupported target/backend combinations before staging any output.
7. Produce one immutable resolved-project model containing chosen values,
   constraints, provenance and diagnostics; templates must consume resolved
   values and cannot make independent hardware guesses.

**Exit gate:** every accepted target field has a source and verification
status; invalid/conflicting resources produce stable diagnostics; resolver
output is deterministic for the same input and data revision.

### Milestone 2 — reproducible project assembly

1. Validate requested UETD modules against the live UETD manifest, dependency
   closure and capability requirements; reject unknown modules and cycles.
2. Select and copy framework files with stable ordering, preserving public
   include paths and declared external dependencies. For a UETD-managed bundle,
   require an exact lock and target profile, then copy only the declared source
   and include paths without network access.
3. Generate project scaffolding, canonical source/header/build lists and
   configuration from the resolved-project model.
   - [x] Accept validated project-level `defines` and propagate them through
     `project_defines.txt` to the generated Make/CMake compile definitions;
     this supports `UMATH_ACCUMULATOR_TYPE=uetd_f64_t` in project configuration.
   - [ ] Add first-class, validated UMATH scalar/accumulator CMake settings to
     generated projects and map them to compile definitions for every target
     translation unit. Match UETD's cache names and accepted values:
     `UETD_UMATH_SCALAR_TYPE` (`uetd_f32_t` or `uetd_f64_t`) and
     `UETD_UMATH_ACCUMULATOR_TYPE` (`SCALAR`, `uetd_f32_t`, or `uetd_f64_t`).
     Reject conflicts with project `defines` and document the configure-time
     invocation. A bare `-DUMATH_ACCUMULATOR_TYPE=...` cache entry currently
     has no effect unless the generated CMake fragment explicitly maps it.
4. Verify every generated relative path and include/source reference stays
   within the staged project; record input, chip-data, UETD source-fingerprint and output
   hashes in provenance.
5. Publish atomically only after validation; specify behavior when destination
   already exists, is open, or is on another volume, and never silently
   overwrite user data.

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
   Keep bare-metal ARM/AVR compiler flows separate from Arduino CLI and
   ESP-IDF adapters: the latter own their build recipes and may emit multiple
   target images instead of one ELF/HEX/BIN tuple.
4. Derive flash/RAM ranges and sizes from the linked artifact and verified
   linker/target memory model, not configuration guesses.
5. Populate versioned artifact image segments from each adapter's linker/map or
   SDK output metadata. ESP-IDF bootloader, partition-table, and application
   files each have distinct offsets and hashes; one aggregate BIN is not an
   adequate deployment record unless its complete offset map is preserved.
6. Store a versioned artifact sidecar with source/project/chip/UETD hashes,
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

### Milestone 6 — V1 CLI diagnostics and wired console

1. Define stable human-readable CLI diagnostics, including useful default
   operation/error output and a verbosity option for additional detail.
2. Preserve the JSON bridge contract: stdout contains only its response and
   diagnostic/progress text goes to stderr.
3. Extend the UETD development-board registry contract with verified wired
   console metadata (for example USB CDC or USB-to-UART, baud where applicable,
   and any port-selection/reset-line behavior); uef-gen must not guess it.
4. Add a console-monitor command that attaches to a selected, already-running
   target over cable and streams text. It must not flash, erase, or implicitly
   reset the target; define disconnect, cancellation, and cleanup behavior.
5. Defer Wi-Fi, Bluetooth, and other wireless console transports until after
   V1.

**Exit gate:** CLI diagnostics are stable for interactive and JSON callers; a
verified board profile can stream live wired console output without changing
target state; wireless transports are not required for V1.

### Milestone 7 — integration and release

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

## 1. Ownership migration: UCON from uef-gen to UETD

- [x] Keep UETD as the single owner of UCON algorithms, public types, catalog metadata, and templates. The direct PID and lead-lag implementations are registered; all remaining keys stay fail-closed until their typed designs are implemented.
- [x] Remove generator-side `AlgorithmIR` classes/builders, standalone UCON configuration/signal models, and local template dispatch. The remaining `uetd_adapter/` package reads UETD metadata and renders UETD-owned templates.
- [x] Remove duplicate generator-owned UCON types, algorithm code, and templates. `MANIFEST.in` and wheel package data no longer include a UETD source tree.
- [x] Replace packaged snapshot refresh/integrity with runtime UETD location, version/manifest checks, and a deterministic source fingerprint over the live checkout.
- [x] Return distinct diagnostics when an algorithm key is unregistered, direct-use only, ambiguous through a compatibility alias, or unknown. The unregistered diagnostic includes the UETD-owned per-algorithm path and roadmap priority.
- [x] Validate every UETD unregistered algorithm's individual header/source/module/stage registration at adapter load, and block scaffold-only modules from generated project selection.

## 2. Configuration, target data, and resolution

- [x] Load family, exact-part chip, and development-board records only from UETD’s API-declared registry indexes and per-entry manifests; keep uef-gen free of duplicate target facts.
- [x] Expose Phase 0, Phase 1, and later entries with targets list --include-scaffolds; support --uetd for an explicit UETD checkout.
- [x] Include Part 3.2/15.1 families and broader STM32/Espressif series as non-generatable metadata. Materialize exact-part records from UETD; require `generation_available` before physical generation.
- [ ] Replace the generic sample target with part records backed by vendor reference manuals, datasheets, package pinouts, errata, and clock/DMA documentation; retain source citations and verification status per record.
- [x] Validate chip and development-board registry/record schemas, enforce paths stay within UETD, and reject chip records whose `family_id` or board records whose `chip_id` is unknown. `jsonschema` is a required runtime dependency; its absence produces an explicit registry-load error.
- [ ] Split resource resolution into focused peripheral/pin, DMA, clock, memory, task, and capability modules while preserving one resolved-project contract.
- [ ] Model pin aliases, AF constraints, DMA request routing, interrupt priorities, clock-tree dividers, linker regions, and RTOS scheduling constraints without guessing missing device data.
- [ ] Detect unsupported architectures and RTOS backends before project output is staged.

## 3. Project assembly and provenance

- [ ] Validate that all requested UETD module names and dependencies exist, reject cycles, and preserve stable build-list ordering.
- [ ] Decide whether every selected source/header path should be included in provenance and make the manifest contain enough information to reproduce the exact assembly.
- [ ] Validate copied UETD includes against actual public header paths and external dependency notes.
- [x] Carry safe project `defines` into the generated canonical definition list and CMake target. This is the current route for UMATH overrides in generated projects.
- [ ] Add and document explicit generated-CMake cache settings for UMATH scalar/accumulator selection; ensure they reach both copied UETD sources and generated application sources and cannot disagree with project-config definitions.
- [ ] Complete the FreeRTOS bundle metadata with the actual pinned upstream revision and reviewed architecture/compiler profiles; preserve fail-closed behavior until `FreeRTOSConfig.h`, kernel port, heap policy, and required sources are supplied.
- [ ] Ensure generated project diagnostics and `manifest/manual_actions.json` explain each manual driver/protocol/board task without implying that a wrapper makes the target hardware-ready.
- [ ] Confirm atomic output publication semantics on Windows when the destination exists, is open, or is on another volume; preserve the rule that user data is never overwritten.

## 4. Build manager and artifact store

- [ ] Implement coherent toolchain discovery for ARM GNU, TI C2000, RISC-V, and host builds; keep all selected compiler/binutils paths within one configured installation.
- [ ] Parse generated list files as data, canonicalize paths under the project directory, validate flags, and invoke subprocesses with argument arrays rather than a shell.
- [ ] Add per-source compile, link, map generation, ELF/HEX/BIN conversion, size parsing, timeout/cancellation, and bounded diagnostic capture.
- [ ] Derive flash and RAM use from ELF sections plus the selected linker script and compare them with verified target budgets.
- [ ] Store artifact/image/project/chip/UETD hashes, build timestamp, toolchain version, and linker-derived load range in a versioned sidecar; reject tampered or stale images.

## 5. Discovery and deployment

- [ ] Implement optional USB/serial discovery with platform-specific dependency handling; device enumeration must not initiate target writes or mode changes.
- [ ] Resolve chip identity from read-only probe operations and verified device-ID databases; require exact artifact/target compatibility.
- [ ] Define a user-visible confirmation step that names the project, artifact hash, probe serial, exact chip, and memory range before erase/program.
- [ ] Implement each backend's connect, identify, halt, protection inspection, erase geometry, program, verify, reset, and disconnect behavior with bounded timeouts and actionable errors.
- [ ] Keep mass erase, option-byte changes, protection removal, and firmware recovery as separate explicit user actions.
- [ ] Implement progress callbacks with stable states and meaningful byte totals; preserve cancellation and cleanup semantics.
- [ ] Add CLI commands for build, artifact listing, target discovery, and deploy only after the underlying operations are complete and the JSON contract for scripting is versioned.
- [ ] Integrate deployment into NEXUS through the subprocess/plugin bridge without blocking the UI thread; display logs, progress, selected target, and failure stage.

## 6. V1 diagnostics and wired runtime output

- [ ] Define stable interactive CLI diagnostics and verbosity behavior; send human-readable diagnostics to stderr whenever stdout carries JSON.
- [ ] Add verified wired-console metadata to UETD development-board records and schemas, including transport, port selection, serial settings, and reset-line behavior where applicable.
- [ ] Implement a monitor command for live text output from firmware already running over a cable; it must not flash, erase, or implicitly reset the target.
- [ ] Define cancellation, unplug/reconnect, timeout, and cleanup behavior for console monitoring.
- [ ] Keep Wi-Fi, Bluetooth, and other wireless transports outside V1.

## 7. Documentation and release

- [x] Reconcile the generator's ownership and package map with the rule that UETD owns UCON algorithms, public types, metadata and templates.
- [x] Consume the UETD algorithm catalog in the shared manifest without adding generator-side algorithm types. PID and lead-lag are registered; all other algorithms remain explicitly unregistered with no output templates and individually registered fail-closed file pairs.
- [ ] Keep the adapter schema/diagnostics synchronized as UETD promotes additional typed algorithms or changes the UCON/UPROTO manifest version.
- [ ] Build and test only after requested; this audit has not run a build, test suite, or hardware operation.

## 8. Accepted catalogue expansion contracts

- [x] Read UETD's design-only future catalogue through `load_future_catalogue`; validate its schema and resolve linked UETD/repository-owned contract files without making those entries selectable.
- [x] Implement frontend type interchange for `Option<T>`, `Result<T,E>`, tagged variants, nominal flags/bit operations, fixed-point types, bounded byte/text types, and explicit `.uefs` contract imports. The compiler parses, checks, and lowers these into typed application-model descriptors; target C/ABI emission remains a backend capability and is not implied by frontend acceptance. See `src/uef_gen/language/frontends/project/TYPE_INTERCHANGE.md`.
- [ ] Add generated C/ABI routes for each type and boundary only when the owning UETD contract and selected target route are complete. The firmware backend consumes the lowered application model; it does not parse `.uef` source or infer C layouts.
- [ ] Define the generated shared-memory ABI separately from protocol wire encoding and persistent storage codecs; validate alignment, atomic publication, version compatibility, and migration rules.
- [ ] Extend the `.uefs` system compiler with external FPGA/PLC artifact identity, atomic PLC process-image mappings, typed quality/freshness and command authority, and node/link evidence aggregation. See `src/uef_gen/language/frontends/system/future/EXTERNAL_ENDPOINTS_AND_EVIDENCE.md`.
- [ ] Extend the artifact store in a versioned migration with signed-manifest and qualification-evidence references. Keep build hashes distinct from signatures and assurance evidence.
- [ ] Add optional secure-update orchestration only after UETD target/bootloader capability records describe authentication, staging, trial boot, rollback, recovery and anti-rollback support. See `src/uef_gen/deploy/future/SECURE_UPDATE.md`.
- [ ] Keep every future contract unavailable to generation until its UETD API/manifest, source closure, target support, route/template, and promotion evidence are reviewed together.
