# UEF-PL editor integrations

The editor packages are clients of the same language definition and the same
UETD-aware Language Server Protocol (LSP) process.
They must not maintain separate parsers, type checkers, or copied UETD module
catalogues.

| Editor | Package | Intended role |
|---|---|---|
| Visual Studio Code | [`vscode/`](vscode/README.md) | Public editor package with syntax grammar, snippets, registry fallback hints, semantic coloring, and the shared LSP client. |
| Visual Studio 2026 / 2022 | [`visual-studio/`](visual-studio/README.md) | Native VSIX package for `.uef` and `.uefs` association, TextMate coloring, language configuration, and the shared LSP client. |
| Shared editor analysis | [`../src/uef_gen/language/server/`](../src/uef_gen/language/server/README.md) | Python-implemented LSP server that analyzes UEF-PL through the same compiler API used by `uef-gen`. |

The VS Code grammar and language-configuration JSON are canonical. The
Visual Studio VSIX links those files into its package during build rather than
forking their contents. Dynamic UETD identifiers continue to come from UETD's
live indexes and manifests.

## Feature ownership

- The TextMate grammar supplies immediate lexical coloring in both editors.
- The UETD registries supply module/type/unit/UMATH discovery and readiness
  information.
- The shared language server supplies diagnostics, context-aware completion,
  hover, signature help, definition/references/rename, semantic tokens,
  document/workspace symbols, folding, code actions, and conservative
  formatting. Its handlers use the UEF-PL AST and compiler API; the server is
  not a Python-language service.
- The compiler/verifier owns parser errors, type and unit
  checking, registry-contract linting, model and protocol validation,
  scheduling checks, and source locations.
- Formatting is limited to indentation and whitespace cleanup. Source
  navigation and cross-file rename are offered only when the declaration or
  explicit contract import resolves unambiguously.

The shared compiler frontend is implemented. Eight of ten `.uef` examples
currently pass validation; the sorter intentionally references an unregistered
`stepper` descriptor, while the H745 coil example remains an author-owned
design sketch with unresolved plant and controller details. The `.uefs` coil
system imports that sketch. UETD-incomplete module contracts appear as
readiness warnings. See the
[UEF-PL technical guide](../../docs/guides/UEF_PROJECT_LANGUAGE_GUIDE.md) and
[language design](../../docs/design/PROJECT_LANGUAGE_DESIGN.md).
