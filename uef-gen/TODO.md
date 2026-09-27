# uef-gen implementation TODO

The order below keeps configuration/resource resolution separate from target-specific build and deployment work. A method marked with `NotImplementedError` is an interface scaffold, not a partial success path.

## 1. Ownership migration: UCON from uef-gen to UEF

- [ ] Define UEF's public UCON algorithm, parameter, signal, template metadata, and generation contract after the algorithm designs are ready; do not invent missing equations or bounds.
- [ ] Decide which serialized graph data remains NEXUS-owned and which UCON configuration UEF consumes. Update the versioned NEXUS/uef-gen request contract at the same time.
- [ ] Replace the generator-side algorithm-specific `ir/algorithms.py`, `ir/registry.py`, `ucon/bindings.py`, and UCON-specific `TemplateSet` responsibilities with adapters that consume the finalized UEF contract.
- [ ] Remove duplicate generator-owned UCON type definitions and algorithm/template material after the UEF contract has a reviewed implementation. Keep the current transitional code clearly labeled until then.
- [x] Removed generator-emitted `include/uef/ucon/ucon_types.h`; UEF will provide the reusable public UCON type contract when it is defined.
- [ ] Refresh `src/uef_gen/uef_source` and its hash from the actual UEF tree whenever a UEF release snapshot is intentionally updated.

## 2. Configuration, target data, and resolution

- [ ] Replace the generic sample target with part records backed by vendor reference manuals, datasheets, package pinouts, errata, and clock/DMA documentation; retain source citations and verification status per record.
- [ ] Validate every schema constraint when `jsonschema` is unavailable and return stable path-specific diagnostics for malformed nested data.
- [ ] Split resource resolution into focused peripheral/pin, DMA, clock, memory, task, and capability modules while preserving one resolved-project contract.
- [ ] Model pin aliases, AF constraints, DMA request routing, interrupt priorities, clock-tree dividers, linker regions, and RTOS scheduling constraints without guessing missing device data.
- [ ] Detect unsupported architectures and RTOS backends before project output is staged.

## 3. Project assembly and provenance

- [ ] Validate that all requested UEF module names and dependencies exist, reject cycles, and preserve stable build-list ordering.
- [ ] Decide whether every selected source/header path should be included in provenance and make the manifest contain enough information to reproduce the exact assembly.
- [ ] Validate copied UEF includes against actual public header paths and external dependency notes.
- [ ] Confirm atomic output publication semantics on Windows when the destination exists, is open, or is on another volume; preserve the rule that user data is never overwritten.
- [ ] Keep generated project scaffolding minimal and free of algorithm behavior that UEF does not yet define.

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

- [ ] Reconcile the user-provided uef-gen specification's older UCON ownership sections with the current decision that UEF owns UCON algorithms/templates.
- [ ] Keep `README.md`, `ARCHITECTURE.md`, this TODO, CLI help, schemas, and NEXUS bridge contract aligned.
- [ ] Add verification procedures only when requested; the current pass has not run a build, tests, or hardware operation.
