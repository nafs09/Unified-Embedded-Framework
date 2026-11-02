# uef-gen review and UEF-PL authoring refresh

Reviewed 29 October 2026 against the local UEF/UETD working tree. Existing
UETD changes were retained. This is a host-side code and behavior review,
not physical target qualification or an exhaustive proof of compiler correctness.

## Results

Eight of ten shipped `.uef` projects pass frontend validation, with readiness
warnings. The sorter fails on unregistered descriptors; the H745 coil sketch
fails on incomplete model dimensions/equations and an invocation argument.
The sole `.uefs` example imports that sketch and also fails. The new default
`uef-project` snippet passes without diagnostics.

Before this review, **all ten `.uef` examples failed CLI `check`**, including
the eight accepted by the compiler. The CLI reran the hardware-generation
resolver, rejected unqualified targets, and discarded frontend warnings.
`check` now reports the same frontend result as the editor. This does not
make those targets eligible for generation.

## Confirmed defects fixed

| Priority | Trigger and consequence | Change |
|---|---|---|
| High | `has`, `set`, or `clear` flags expressions raised `NameError` in the parser. | Import the existing `FlagsOperationExpression` AST class; exercise all three operations. |
| High | Completing a record/component member raised `NameError` while building documentation. | Bind the receiver name before constructing completion items. |
| Medium | `.uef check` disagreed with the compiler/editor and hid readiness warnings. | Use the public frontend result directly, including source spans and warnings. Hardware YAML/JSON checks retain resource/module validation. |
| Medium | Valid child-node warnings disappeared from a system result. Child errors appeared at child-file coordinates in the system editor. | Aggregate warnings and errors; anchor imported diagnostics at their node/import and link to the actual source with UTF-16 coordinates. |
| Medium | `.uefs generate --dry-run` unconditionally reported an unavailable emitter. | Return the validated system plan without writing firmware. Ordinary generation remains unavailable, and CLI target overrides are rejected. |
| Medium | Generation dry runs and unavailable-emitter responses lost frontend warnings. | Preserve them alongside generation diagnostics. |
| Medium | Generated-project validation ignored selected dependency include roots and could accept an escaping source-list entry. | Validate source containment and resolve headers through the actual project include list. |
| Medium | The generator removed an existing empty destination before staging/validation completed. | Defer removal until the complete staged copy is ready for publication. A destination populated in the meantime is refused. |
| Medium | Formatting mishandled EOF/newline options and a selection ending at column zero could modify the next line. | Replace the complete document through its real EOF; respect exclusive range ends and preserve LF/CRLF/CR by default. |
| Medium | VS Code could still report the compiler active after it stopped, suppressing fallback hints. | Track client state; restore registry hints on stop and expose compiler status/restart/output actions. |
| Medium | A missing module manifest could be labelled ready by fallback hints. | Require an explicit `available_for_generation: true` flag. |
| Low | System completion loaded a nonexistent registry path and silently used a short fallback keyword list. | Read the packaged `frontends/system/registry.json`. |

## Remaining issues, in implementation order

### 1. Firmware emission and deployment are still unavailable — release blocker

The frontend lowers application/system plans, but nothing consumes those plans
to emit firmware. The build, flash, probe and monitor routes contain explicit
unavailable scaffolds. The hardware generator emits startup/linker/application
placeholders. Preserve these gates until one exact board is qualified end to end.

Evidence: [`cli.py`](../../src/uef_gen/cli.py),
[`deploy/`](../../src/uef_gen/deploy/README.md), and
[`project_scaffold.py`](../../src/uef_gen/templates/project_scaffold.py).

### 2. Peripheral lowering and hardware configuration disagree — high

The drone and ECHO projects compile, but their lowered peripheral arrays fail
`validate_configuration`. Both contain `dma: "enabled"`; the hardware schema
requires a mapping of DMA requests to routes. The lowerer copies the DSL value
verbatim. An emitter must translate an allocation request through verified
target data, or reject unsupported syntax with a source-located diagnostic.
Changing the schema to accept the string would not implement DMA allocation.

Reproduce by compiling either showcase and passing
`result.hardware_configuration` to `uef_gen.config.schema.validate_configuration`.
Evidence: [`lowering.py`](../../src/uef_gen/language/frontends/project/lowering.py)
and [`project.schema.json`](../../src/uef_gen/schemas/project.schema.json).

### 3. Architecture selection defaults unsupported targets to x86 — high

`ProjectModulePlanner.select()` requests `uhal/arm_cm` for architecture names
containing `arm` or `cortex`, and `uhal/x86` for everything else, including
Xtensa/RISC-V/AVR. Present target-readiness gates mask the problem; enabling
such a target could select the wrong HAL. Likewise the generic `uos` alias
maps every non-FreeRTOS runtime to BareMetal. Resolve both from UETD-owned
bindings, and reject absent bindings before enabling target generation.

Evidence: [`modules/planner.py`](../../src/uef_gen/modules/planner.py).

### 4. Manual-action provenance is an empty placeholder — medium

The pipeline initializes `manual_actions = []` and never populates it before
serializing `manifest/manual_actions.json`. It is not evidence that a scaffold
requires no integration work. The README now describes that limitation.
Define the producer/aggregation contract before presenting the manifest as a
complete checklist.

Evidence: [`generation/pipeline.py`](../../src/uef_gen/generation/pipeline.py)
and [`generation/manifest.py`](../../src/uef_gen/generation/manifest.py).

### 5. Generated GNU Make fragments do not support project roots with spaces — medium

The fragment uses `$(shell cat $(PROJECT_ROOT)/...)`, `abspath`, and Make
word-list transformations without a path-with-spaces contract. A checkout
such as `Unified Embedded Framework` exposes this problem. The CMake fragment
quotes file-reading paths; prefer it until Make path handling is designed and
tested on the supported hosts.

Evidence: the `config/project.mk` template in
[`generation/pipeline.py`](../../src/uef_gen/generation/pipeline.py).

### 6. Coverage and packaging still need expansion — medium

This review adds the first checked-in uef-gen regression suite. It covers the
confirmed authoring bugs, examples, selected staging validation, and extension
lifecycle; it does not cover every grammar production, numerical rule,
registry corruption, filesystem race, or eventual target backend. The broad
Ruff `F,E9` pass also reported unused imports/locals and import-name shadowing;
those unrelated cleanups were left out. Undefined-name checks now pass.

VSIX packaging succeeds with a bundling recommendation because the extension
ships the language-client dependencies directly. IDE-host interaction and the
Visual Studio VSIX build matrix still require release testing.

## Authoring redesign and compatibility

- Preserve the language grammar and existing projects; no syntax migration.
- `check FILE --format text` prints source locations, excerpts, underlines and
  a result summary. JSON remains the default for existing scripts/bridges.
- `check .uef` now returns the compiler response (`application`,
  `application_summary`, `hardware_configuration`, `backend_input`, diagnostics).
  Callers needing module/source selection must use `generate --dry-run`.
- `.uef generate --dry-run` still checks hardware/module readiness. `.uefs
  generate --dry-run` returns a frontend system plan; it does not qualify its
  nodes for firmware generation. Neither route publishes firmware.
- `format FILE` writes formatted source to stdout; `--write` edits the file;
  `--check` exits 1 if changes are needed, 0 if clean, and 2 on an error.
- CLI and both LSP clients share conservative indentation for braces,
  parentheses and brackets, with token spelling/comments preserved.
- VS Code 0.1.5 adds coordinated dark/light themes, readable numeric/unit
  colors, generic-theme semantic inheritance, two-space language defaults,
  a compiler-status menu, and restart/log/settings commands.
- The starter is a small typed BareMetal loop with an explicit numeric policy.
  Add algorithms and hardware bindings after the initial source checks cleanly.

## Validation

Run from `uef-gen`, with its source package and language-server extra installed:

```powershell
$env:PYTHONPATH = (Resolve-Path src).Path
python -m unittest discover -s tests -v
python -m ruff check src tests --select F821,F822,F823,E9
Set-Location editors/vscode
npm test
npm run package
```

The Python suite passes 20 tests, including all 11 examples and the starter.
The four Node tests pass, including client restart/shutdown/fallback and source
color contrast of at least 4.5:1 against both theme backgrounds. Formatting is
idempotent and preserves the lexer token stream across all examples. A real
stdio LSP session also passed initialize, didOpen, formatting, shutdown and
exit. Host validation used Python 3.14; the supported Python-version matrix
and live IDE interactions remain release checks. No physical hardware was accessed.
