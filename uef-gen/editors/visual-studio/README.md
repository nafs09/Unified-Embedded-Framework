# UEF Project Language for Visual Studio

This folder scaffolds a native Visual Studio VSIX for `.uef` and `.uefs` files.
It is aimed at Visual Studio Professional 2026 and uses the Visual Studio 17.x
extension API surface so the same package can also support Visual Studio 2022.
The manifest has no upper version bound; Microsoft documents that VS2022
extensions using supported 17.x APIs can load in Visual Studio 2026. The VSIX
still needs to be built and checked in both IDE versions before release.

The VSIX registers a code content type, maps `.uef`/`.uefs` to it, loads the
shared TextMate grammars and language configuration, and starts the same
`uef-gen-language-server` process used by VS Code. Declaration roles have
separate Visual Studio classifications and colors. The server is written in
Python using `pygls`, but it analyzes UEF-PL; it is not a Python LSP. It does not create a custom
Visual Studio project type. Open a `.uef` file directly or use **File > Open >
Folder** on the folder containing the `.uef` project. LSP language services are
designed for opened files and folders; a Solution Explorer project system is a
separate optional feature.

## Build prerequisites

- Visual Studio 2022 17.14 or Visual Studio 2026 with the **Visual Studio
  extension development** workload.
- The .NET Framework 4.7.2 targeting pack and the .NET SDK/MSBuild required by
  the installed IDE.
- Network access to restore the pinned NuGet packages on first build.
- Python 3.11+ with this checkout's language-server extra installed:

  ```powershell
  Push-Location ..\..\..
  python -m pip install -e ".\uef-gen[language-server]"
  Pop-Location
  ```

The client tries `UEF_PL_LANGUAGE_SERVER`, the `uef-gen-language-server`
command, then `UEF_PL_PYTHON`, `python`, and `py -3` with
`-m uef_gen.language.server`. Install uef-gen into the Python interpreter you
intend the IDE to use; set `UEF_PL_PYTHON` to its full path if `python` does
not resolve to it. Set `UETD_PATH` to the UETD checkout if it is not
discoverable next to the project.

## Build and launch

1. Open `UEF.ProjectLanguage.VisualStudio.sln` in Visual Studio.
2. Restore NuGet packages and build the VSIX project.
3. Start debugging (F5) to launch the Visual Studio experimental instance.
4. Open a `.uef` or `.uefs` example or open its containing folder.
5. Confirm the differentiated declaration colors, bracket/comment behavior,
   registry completion and hover, LSP diagnostics, signature help, and source
   navigation. The server also supports outline/workspace symbols, references,
   safe rename, folding, quick fixes, and conservative formatting. These
   diagnostics follow the shared compiler frontend's current coverage.
6. Install the resulting `.vsix` into the intended Visual Studio release for
   manual validation before publishing.

### Regular Visual Studio versus the Experimental instance

Pressing **F5** launches Visual Studio's isolated Experimental instance. The
VSIX deployed there is separate from the extension installed in the regular
Visual Studio window. Building or debugging the solution does not install the
VSIX into the regular instance. To use the regular IDE, install the generated
`.vsix` into that Visual Studio installation, then close and reopen the regular
IDE. Check **Extensions > Manage Extensions > Installed** in that same window
to confirm that *UEF Project Language* is installed there.

After opening a `.uef` or `.uefs` file, check
`%LOCALAPPDATA%\UEF-PL-LSP.log`. A loaded extension should log that Visual Studio
matched the UETD Project Language content type, then that the language server
started and initialized. If the log was not created or has no new entries,
open a `.uef` or `.uefs` file from its current on-disk path first. If the log
still does not appear, inspect Visual Studio's Activity Log for MEF or content
type registration errors; restarting an LSP process cannot restore a document
tab that points to a path that has moved.

Set `UETD_PATH` to the current UETD checkout (the folder containing `api.json`)
for registry-backed features when the project is outside the adjacent-checkout
layout. The old `UEF_PATH` setting is not read by the current language client.
When repository files have been moved or renamed, close stale tabs and reopen
the file from its current path; those tabs continue to refer to the old path.

The language client writes startup and process stderr details to
`%LOCALAPPDATA%\UEF-PL-LSP.log`; use the messages described above to distinguish file
activation, Python startup, and LSP initialization failures.

The Python language server writes compiler diagnostics and request traces to
`%LOCALAPPDATA%\UEF-PL-LanguageServer.log`. Errors and warnings are included at
the default log level. To capture every diagnostic plus hover-resolution and
other request traces, set the Windows user environment variable
`UEF_PL_LOG_LEVEL` to `DEBUG`, then restart Visual Studio and reopen the file.
Set `UEF_PL_LOG_FILE` to choose another server-log path. The server never writes
logs to standard output because the LSP protocol uses that stream.

## Files and ownership

| File | Purpose |
|---|---|
| `source.extension.vsixmanifest` | VSIX identity, VS2022/VS2026-compatible installation targets, MEF and pkgdef assets. |
| `UETDProjectLanguageContentTypes.cs` | Registers the `.uef`/`.uefs` editor content type for LSP activation. |
| `UEFProjectLanguageClassifications.cs` | Defines distinct colors for project, type, hardware, algorithm, runtime, and binding declarations. |
| `UETDProjectLanguageClient.cs` | Starts the shared LSP process over standard input/output. |
| `UEF-PL.pkgdef` | Registers TextMate grammars and language configuration with Visual Studio. |
| `UEF-PL.tmTheme` | Shared declaration color rules, packaged under the matching theme filename for each TextMate grammar. |
| `UEF.ProjectLanguage.VisualStudio.csproj` | VSIX project and NuGet references. It links grammar/configuration files from the VS Code package so there is one editable copy. |

The server uses `UETD_PATH` when set. Otherwise it searches the project
directory and its ancestors for an adjacent `UETD` checkout, so nested example
files work without per-example configuration. For projects outside that
layout, set `UETD_PATH` to the UETD checkout. Registry and language semantics
stay in the shared Python server; do not duplicate the registry loader in C#.

## Current scope and next implementation work

The VSIX is an experimental local extension, not a published or release-
validated package. Its shared grammar, declaration classifications, and LSP
features are implemented. Compiler diagnostics are only as complete as the
shared parser and verifier. The firmware emitter, build, and flash pipeline are
separate uef-gen work.

Keep Visual Studio-specific code limited to VSIX registration, LSP startup,
settings, and packaging. UEF-PL syntax, UETD registry lookup, diagnostics, and
navigation belong in the shared `uef-gen` language tooling.

Microsoft references:

- [Visual Studio LSP extension guide](https://learn.microsoft.com/en-us/visualstudio/extensibility/adding-an-lsp-extension?view=visualstudio)
- [Visual Studio extension compatibility](https://learn.microsoft.com/en-us/visualstudio/extensibility/migration/extension-compatibility?view=visualstudio)
- [TextMate language configuration](https://learn.microsoft.com/en-us/visualstudio/extensibility/language-configuration?view=visualstudio)
