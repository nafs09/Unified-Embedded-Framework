# UEF-PL language server

This package implements the Language Server Protocol (LSP) endpoint shared by
the VS Code extension and the Visual Studio VSIX. It is implemented in Python
with `pygls`, but the language being analyzed is UEF-PL. LSP is an editor
communication protocol, not a Python language definition or parser.

## Install and run

From the repository root, install the optional server extra into the Python
environment used by the editor:

```powershell
python -m pip install -e ".\uef-gen[language-server]"
```

This installs the `uef-gen-language-server` command. It communicates over
standard input/output using LSP framing; logs go to standard error so they do
not corrupt the editor protocol. An explicit UETD checkout can be passed as
`--uetd-root <path>`. Otherwise the shared `UETDLocator` resolves UETD from the
project location, the environment, or the documented sibling layout.

For a manual protocol launch, use:

```powershell
uef-gen-language-server --uetd-root C:\work\UETD
```

Do not run a second language server for VS Code and Visual Studio. Each editor
starts its own process instance as needed, and both use the same implementation
and compiler API.

## Current features

- Registry-backed completion for UETD module descriptors, public types, UMATH
  DSL functions, and units.
- Hover information and UMATH signature help from the live authoring catalogue.
- Readiness diagnostics for known module/function declarations and compiler
  errors from the shared parser and semantic verifier.
- Definition navigation from registered module, UMATH, and type names to the
  corresponding UETD manifest or header, local project/system declarations,
  and types imported from `.uefs` contracts.
- Component-output and typed-record member completion/hover, and signature
  help for UMATH functions, language builtins, and declared components.
- Document outline, workspace symbol search, references, safe rename across
  explicit `.uefs` imports, and same-file highlights.
- Folding, conservative indentation formatting, and a quick fix for hidden raw
  UMATH C names.
- `.uefs` compiler diagnostics for node projects, contracts, and links.

The server delegates validation to the shared compiler frontend. Diagnostic
coverage therefore tracks that frontend's current implementation. Formatting
does not rewrite expressions or reorder declarations; it only applies brace
indentation and whitespace cleanup.

## Architecture rules

- The compiler frontend and registry contract resolver own language semantics.
  The server adapts their diagnostics and source spans to LSP structures.
- Do not add a second parser or validation implementation here. CLI `check`
  and both editor clients must use the same compiler API.
- UETD remains the only source of truth for modules, type declarations, UMATH
  functions, and units. The server reads those indexes and their referenced
  manifests; it must not copy a UETD catalogue into editor code.
- Keep stdio reserved for LSP frames. Send logs to stderr.
- Keep editor features backed by the shared AST and UETD contracts. Extend
  cross-file rename only when the resolver can prove an unambiguous import
  relationship; ambiguous cases should remain unavailable rather than guess.

The server entry point is `server.py`; `python -m uef_gen.language.server`
starts the same endpoint. See the [editor integration overview](../../../../editors/README.md)
and [UEF-PL editor support in Visual Studio](../../../../editors/visual-studio/README.md).
