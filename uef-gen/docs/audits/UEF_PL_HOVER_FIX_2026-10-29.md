# Visual Studio imported-diagnostic hover fix

The 0.1.14 authoring update anchored every imported compiler diagnostic at the
same system node. Visual Studio's native LSP Quick Info appends all diagnostics
overlapping the hovered span, so the Tesla-coil controller node produced a
popup larger than the screen. The inactive custom Quick Info renderer was not
responsible; limiting that renderer would not fix the active native hover.

The shared server now publishes one severity/count summary per imported file
on the node and sends the full diagnostics to the actual source URI. Each
summary has one navigation link, with errors taking precedence over warnings.
The CLI compiler result is unchanged. Error List retains the complete messages
at their actual UTF-16 source positions.

Publication tracks ownership by importing document. Multiple importers do not
duplicate rows or clear another document's diagnostics. Open buffers with newer
versions take precedence over imported saved-file results. Removing an import
or closing the last owner clears obsolete entries, and saving a referenced
source refreshes its importing systems.

Validation: six new diagnostic-routing regressions and all 15 authoring tests
pass. A real stdio LSP session against `tesla_coil_system.uefs` produced two
short parent summaries: H745 with 9 errors/30 warnings and G474 with 26 warnings.
All 65 detailed diagnostics were published to their respective child files;
closing the system cleared all three URI reports. Shutdown completed cleanly.
The rebuilt Visual Studio VSIX is version 0.1.15. Native IDE presentation still
requires restarting the installed extension/server to load the new source.
