# UEF-PL support for VS Code

This extension associates `.uef` and `.uefs` with the language, supplies
TextMate coloring and snippets, and reads names and readiness information from
the live UETD registries. It is an editor client, not the UEF-PL compiler.
Language analysis belongs to the shared `uef-gen` frontend and language server.
For a user walkthrough, see the [UEF-PL handbook](../../../docs/guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md).

## Features

- File associations, line comments, brackets, indentation, and folding.
- Syntax coloring and snippets for `.uef` and `.uefs` structures.
- Registry-derived completion, hover, and semantic coloring for UETD module
  descriptors, public types, UMATH DSL functions, and units.
- Warning diagnostics for registered entries with incomplete authoring or
  generation contracts, and a quick fix for hidden raw `umath_*` C names.
- Shared language-server support for diagnostics, context-aware completion
  (including component outputs and record fields), hover, signature help,
  go-to-definition, document/workspace symbols, references, safe rename,
  document highlights, folding, and brace-indentation formatting.

The extension includes a `UEF-PL Dark` color theme with a more distinct palette
for declarations, control-flow words, types, registered UETD names, functions,
units, enum cases, properties, and values. Select it from **File > Preferences
> Theme > Color Theme** (or run **Preferences: Color Theme**) to use those
colors. The TextMate grammar still works with other themes, which may choose a
more limited palette. Visual Studio uses the same grammar and maps its scopes
to the IDE's built-in classifications; those colors follow the selected Visual
Studio theme and can be adjusted under **Tools > Options > Environment > Fonts
and Colors**.

The grammar handles lexical highlighting; the language server provides
semantic colors and language features from the shared parser and UETD
registries. Registry names are not copied into an editor keyword list. The
server delegates diagnostics to the same compiler frontend as `uef-gen check`;
it does not implement a second parser or verifier. Formatting is deliberately
limited to deterministic brace indentation and whitespace cleanup.

## Registry discovery

Set `uef-project-language.uefRoot` to the UETD checkout root when it is not
discoverable from the workspace:

```json
{
  "uef-project-language.uefRoot": "C:/work/Unified-Embedded-Framework/UETD"
}
```

If blank, the extension looks for `UETD/registry/modules/registry.json`,
`registry/modules/registry.json`, or a sibling `../UETD/registry/modules/registry.json`
from each open workspace. It reads module descriptors and manifests, then
follows `UETD/api.json` to the type, UMATH, and unit indexes. It does not scan C
headers or validate registry JSON schemas. See the
[registry/discovery guide](../../../docs/guides/UEF_PL_REGISTRIES.md) for the
source-of-truth and contributor workflow.

## Run locally

1. Install Node.js 22 or newer and run `npm ci` in this directory. The lockfile pins the extension client and packaging tool dependencies.
2. Open this directory in VS Code and press F5 to start an Extension
   Development Host.
3. Open a `.uef`/`.uefs` example in that host. If registry suggestions are
   missing, open the repository as a workspace or set `uefRoot` in the host's
   settings.

The included `.vscode/launch.json` starts the extension host. The extension is
plain JavaScript and has no compile step. `npm ci` installs the language
client and `@vscode/vsce` packaging tool. Run `npm run package` to produce the
installable `.vsix` file in this directory.

The registry hints work without the language server. To enable the full shared
language service, install the optional server package from the `uef-gen`
project:

```powershell
python -m pip install -e ".\uef-gen[language-server]"
```

The extension tries `UEF_PL_LANGUAGE_SERVER`, the configured executable, the
`uef-gen-language-server` command, and finally Python module launchers. Set
`uef-project-language.languageServer.pythonExecutable` to the interpreter
where you installed the package (or set `UEF_PL_PYTHON`) if `python` is not the
right interpreter. Use `uef-project-language.uefRoot` to select a UETD
checkout; the extension passes it to the server.

The language server writes errors and warnings to
`%LOCALAPPDATA%\UEF-PL-LanguageServer.log` on Windows (or the per-user state
directory on Unix). For full diagnostic details and hover-request traces, set
`UEF_PL_LOG_LEVEL=DEBUG` in the environment before launching VS Code, then
restart VS Code and reopen the document. Set `UEF_PL_LOG_FILE` to select a
different file. The **UEF-PL Language Server** output channel (View > Output)
shows client startup and connection messages.

## Status

The editor client, shared LSP features, and common registry-backed hints are
implemented. Diagnostic completeness follows the current shared parser and
semantic verifier, so gaps there will also appear in the editor. Code
generation, firmware build, and flash backends remain separate uef-gen work;
UETD entries without authoring/generation contracts remain discoverable and
receive readiness warnings.

The [Visual Studio VSIX scaffold](../visual-studio/README.md) uses the same
server process and links these grammar/configuration files so there is one
editable source for both IDEs.

See the [language design](../../../docs/design/PROJECT_LANGUAGE_DESIGN.md),
[technical guide](../../../docs/guides/UEF_PROJECT_LANGUAGE_GUIDE.md), and
[generated catalogue](../../../docs/reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md).
