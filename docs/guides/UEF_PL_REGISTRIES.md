# UEF-PL registries and discovery

This guide is for two audiences: people writing `.uef`/`.uefs` projects who
want to know where names and type information come from, and contributors who
need to update a UETD catalogue or the language grammar. It describes the
current file layout and loader behavior. The standalone project compiler is
still scaffold work, so registry discovery does not mean every discovered
module can already be checked or generated.

For a first-time walkthrough, read the [UEF-PL user handbook](UEF_PROJECT_LANGUAGE_HANDBOOK.md).
For detailed source rules, read the [technical guide](UEF_PROJECT_LANGUAGE_GUIDE.md)
and [language design](../design/PROJECT_LANGUAGE_DESIGN.md). For record
authoring rules, see the [UETD registry README](../../UETD/registry/README.md).


## Catalogue ownership and file roles

```text
UETD/api.json
  ├─ module_registry ──> registry/modules/registry.json
  │                       └─ module ID -> DSL_descriptor + manifest path
  │                                              └─ manifest authoring/generation contract
  ├─ type_registry ────> registry/types/registry.json
  ├─ unit_registry ────> registry/units/registry.json
  ├─ umath_api.function_registry -> registry/umath/registry.json
  └─ target catalogue paths ─────> family/chip/board/toolchain indexes

uef-gen language_registry/registry.json + schema.json + dispatch.py
  └─ UEF-PL syntax and allowlisted parser/lowering dispatch IDs

uef-gen system_language/registry.json + schema.json
  └─ .uefs syntax and system-level constructs
```

These layers have different jobs:

| File or catalogue | What it means | When you edit it |
|---|---|---|
| `UETD/api.json` | UETD's public path index. Tools read it to find the catalogue indexes and some schemas; other schemas are linked from their catalogue index. Paths are relative to the UETD checkout. | Change it when a catalogue index/schema path moves or a new catalogue path must be published. Adding a record to an already indexed catalogue normally does not require an `api.json` edit. |
| `UETD/registry/modules/registry.json` | The canonical module index: stable module ID to manifest path and unique `DSL_descriptor`. The descriptor is the spelling used in `.uef`. | Add or rename a source-visible UETD module descriptor here, with the matching manifest reference. |
| `UETD/registry/modules/manifest/**/*.json` | Per-module source/dependency data and, when supported, its authoring contract: kind, settings, ports, operations, constraints, readiness, and generation route. | Update a module's actual project-language contract and readiness here. |
| `registry/modules/registry.schema.json` and `authoring_contract.schema.json` | Shape constraints for the module index and the nested authoring contract. | Change only when the data contract itself changes; update records and schema together. |
| `registry/types/registry.json` | Header-derived index of public UETD C type declarations, their kind, source status, visibility, and explicit `dsl_name` where UETD defines a language spelling. | Regenerate after public header changes. A module contract still decides whether a type is usable by value in UEF-PL. |
| `registry/umath/registry.json` | Header-derived UMATH function index plus UETD-owned DSL names, signatures, binding metadata, readiness, and optional operator bindings. | Regenerate C declaration facts after header changes; edit the DSL binding metadata in this index. |
| `registry/units/registry.json` | Canonical unit symbols, dimensions, exact scales, and affine offsets used by type checking. | Edit when adding or correcting a unit. |
| `registry/targets/**/registry.json` and `manifest/` | Target family, exact chip, development-board, and toolchain catalogues. `api.json` points to their indexes. | Add target/board/build facts in the catalogue that owns them; do not put board data in the module catalogue. |
| `registry/future/registry.json` | Design-only proposals and cross-layer capabilities. Entries here are not selectable UETD modules. | Track planned work here; promote a capability only after its API and generation contract are ready. |
| `uef-gen/src/uef_gen/language/registry/registry.json` | Grammar productions, tokens, builtins, operators, and language-only constructs such as `map`, `select`, `subsystem`, and `hybrid`. It is not a UETD module list. | Edit when the language syntax or a language-owned rule changes. |
| `uef-gen/src/uef_gen/language/frontends/system/registry.json` | The additional `.uefs` grammar and system-only rules. Its target/module/type facts still come from UETD and the referenced `.uef` projects. | Edit when multi-target syntax or semantics change. |

## How names are discovered

The standalone compiler first resolves a UETD checkout. The current `uef-gen`
locator checks, in order, `UETD_PATH`, `uetd.path` in project configuration,
then `../UETD`, `../../UETD`, and `./UETD` relative to the project directory. It
requires `api.json`, the required API-declared catalogues, and a compatible
UETD version when `version.json` is present. Catalogue paths are resolved from
the UETD root and must remain inside that checkout; the loader follows index
references rather than guessing IDs from filenames.

For UEF-PL data, the resolver follows `api.json` to the module index. Each
module entry supplies a stable ID, one `DSL_descriptor`, and a path to its
manifest. The descriptor is the language spelling; the ID is the canonical
registry identity; the manifest is the detailed contract. Duplicate
descriptors or broken manifest references are invalid. The same API index
locates the type and unit catalogues and, under `umath_api`, the UMATH
function catalogue. Target and board catalogues are resolved through their
own UETD index paths.

This means a name can be **known** without being **usable for generation**:

1. A `DSL_descriptor` makes a module discoverable for completion and name
   resolution.
2. A reviewed `authoring` contract tells the checker what settings, ports,
   operations, and constraints the declaration accepts.
3. A ready generation route plus implemented source/templates and target
   support are required before generation can use that module.

UMATH follows the same distinction. A public `umath_*` declaration appears in
the generated index and receives an operation name from its registered family
and C symbol (for example, `umath_vec3_cross` maps to `cross`). Its former
family-qualified spelling remains a compatibility alias. Several functions may
share one operation name; UEF-PL selects one only when every candidate has
reviewed logical parameter types and exactly one candidate matches. The chosen
function must also have an implemented UETD route and a ready
`dsl_binding_contract`. The raw C symbol, pointer ABI, and status return stay
hidden from project source.

The project-language adapter reads these UETD indexes and validates their
schemas and authoring contracts. The `.uef` and `.uefs` frontends parse,
validate, and lower source into backend-neutral inputs. The target code emitter
and per-function/per-module generation routes still determine which validated
designs can become firmware; a registry declaration alone does not make a
feature generation-ready.

## VS Code discovery

The VS Code extension resolves the UETD root separately from the `uef-gen`
CLI. Set `uef-project-language.uefRoot` to the UETD checkout root, for example
`C:/work/Unified-Embedded-Framework/UETD`. If it is unset, the extension looks
in each open workspace for `UETD/registry/modules/registry.json`,
`registry/modules/registry.json`, and a sibling `../UETD/registry/modules/registry.json`.

Once it finds the module index, the language server reads each linked module
manifest, then reads the type, UMATH, and unit indexes through `api.json`. This
feeds semantic coloring, completion, hover, definition navigation, and
readiness diagnostics. The TextMate grammar remains static and comes from the
editor package. The compiler frontend owns parsing and type checking; registry
hinting is not a substitute for schema validation. See the [VS Code extension
README](../../uef-gen/editors/vscode/README.md) for setup details.

## Visual Studio support

The repository has a native VSIX project at
[`uef-gen/editors/visual-studio`](../../uef-gen/editors/visual-studio/README.md).
It registers `.uef` and `.uefs` as source-file content types, reuses the VS
Code TextMate grammar, and starts the same Python-implemented
`uef-gen-language-server`. LSP is a neutral editor protocol; its server code
uses Python libraries but analyzes UEF-PL. Both editor clients obtain module,
type, UMATH, and unit names from UETD's indexed JSON registries. They do not
maintain a separate list of UETD components.

The current language server provides source diagnostics, registry-backed
completion/hover/signature help, definition/reference/implementation
navigation, rename, document/workspace symbols, formatting, folding, semantic
tokens, and quick fixes through the UEF-PL compiler frontend. The VSIX targets
the Visual Studio 17.x API surface for VS2022 and VS2026; build and test it in
the intended IDE before distributing it.

For the first version, Visual Studio's Open Folder or single-file workflow is
enough to edit a standalone project; loading `.uef` as a full project-system
container is a separate feature. See the [user handbook's editor section](UEF_PROJECT_LANGUAGE_HANDBOOK.md#editor-support)
and Microsoft's [Visual Studio LSP extension guide](https://learn.microsoft.com/en-us/visualstudio/extensibility/adding-an-lsp-extension?view=visualstudio).

## Making a registry change

### Add or update a UETD module

1. Add the module's header/source/template files and update its manifest under
   `UETD/registry/modules/manifest/` with files and dependencies.
2. Add one entry to `UETD/registry/modules/registry.json`. Give it a unique,
   case-sensitive `DSL_descriptor` that is a valid identifier. The manifest
   reference is repository-relative, for example:

   ```json
   {
     "modules": {
       "ucon/control/classical/pid": {
         "manifest": "registry/modules/manifest/ucon/control/classical/pid.json",
         "DSL_descriptor": "pid"
       }
     }
   }
   ```

3. Add the module's `authoring` contract to the referenced manifest. Describe
   every field the project may set, typed inputs/outputs, supported runtime,
   target and numeric limits, operations, failure behavior, dependencies, and
   generation route. Keep readiness at `draft` until that route is real and
   reviewed.
4. Check that the index and authoring contract match their JSON Schemas. A
   descriptor alone only makes the name discoverable. Missing or draft
   metadata must stay visible as incomplete rather than being guessed.
5. Do **not** add that module to `uef-gen/language_registry/registry.json`, a
   Python allowlist, or a static VS Code keyword list. The language already
   discovers ordinary module names through the UETD index.

If only a module's settings, ports, or generation readiness change, update
its manifest. If the source spelling or manifest path changes, update the
module index as well. If a catalogue index itself moves, update its pointer in
`api.json`.

### Update types, UMATH, or units

After changing public UETD headers, run this from the repository root:

```powershell
python UETD/tools/generate_project_language_catalogues.py
```

It rebuilds `registry/types/registry.json`, refreshes the C declaration facts
in `registry/umath/registry.json`, and writes the generated catalogue and unit
references under `docs/reference/`. It does not rewrite the user handbook or
technical language guide. It preserves UMATH binding metadata matched to the
same C function and header: `dsl_aliases`, `dsl_signature`,
`dsl_binding_status`, and `dsl_binding_contract`. The canonical operation name
is derived from its registered family and C symbol. Edit binding and alias
metadata in
`registry/umath/registry.json`, not the generated C signature columns.
The frontend groups identical operation names into overload sets. Their
reviewed parameter descriptors may identify a concrete UETD type or a shared
type category, constrain units and array dimensions, and recursively describe
generic type arguments. Matching is exact and deterministic; the language
does not guess a function from its C name or silently convert a value. Keep a
logical signature in `reviewed` status while its user-facing inputs/outputs
are being authored, then mark it `ready` only after its failure, availability,
cost, and C-ABI binding facts are complete.
Type-index records are regenerated from headers, so do not use that index as
a hand-maintained place for module usability rules. Edit unit definitions in
`registry/units/registry.json` and keep its schema valid. The unit reference
groups dimensions and formats exact scales with Markdown/LaTeX math.

### Add language syntax

Change `uef-gen/src/uef_gen/language/registry/registry.json` for `.uef`, or
`system_language/registry.json` for `.uefs`. Update the matching schema when
the JSON record shape changes. Then implement the syntax in the fixed,
allowlisted dispatch/parser/validation/lowering code and update editor grammar,
snippets, docs, and examples. The JSON contains data and dispatch identifiers;
it does not load Python functions or execute project code. Adding a UETD module
does not require this syntax work.

VS Code keeps separate editor configuration beside the extension:

| Extension file | Role |
|---|---|
| `uef-gen/editors/vscode/package.json` | Extension manifest: language IDs/file extensions, grammar and snippet registrations, activation, and the `uefRoot` setting. |
| `syntaxes/uetd.tmLanguage.json` and `uefs.tmLanguage.json` | Static TextMate token patterns. These color syntax but do not own registered module names or validate a project. |
| `snippets/uef.json` and `uefs.json` | Text-expansion templates for common source structures. |
| `language-configuration.json` | Comments, brackets, indentation, and editor auto-pair behavior. |

These files are editor support, not another registry. Static grammar scopes
need to be kept in sync when syntax changes; component and UMATH names remain
dynamic and come from the UETD indexes.

## Quick decision guide

- New algorithm/sensor/protocol/module name: **UETD module index + manifest**.
- New exact chip or development board: **the matching UETD target catalogue**.
- New public C type or UMATH function: **header, then regenerate the derived
  index/reference**; add reviewed UMATH DSL metadata when callable.
- New unit or conversion scale: **UETD units registry**.
- New keyword, operator, syntax production, or language-only builtin: **the
  `uef-gen` language registry and implementation**.
- New `.uefs` declaration or system-level rule: **the `uef-gen` system-language
  registry and implementation**.
