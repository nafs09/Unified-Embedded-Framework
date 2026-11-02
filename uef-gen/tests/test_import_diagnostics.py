"""Imported diagnostics must not turn a node hover into a whole-project log."""

import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from uef_gen.language.frontends.project.ast import SourcePosition, SourceSpan
from uef_gen.language.frontends.project.diagnostics import source_error, source_warning

try:
    from lsprotocol import types
    from uef_gen.language.server import server as lsp
except ImportError:
    lsp = None


@unittest.skipIf(lsp is None, 'Install the language-server extra for editor regressions')
class ImportedDiagnosticTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.system = self.root / 'system.uefs'
        self.child = self.root / 'child.uef'
        self.child.write_text('// 😀\ninvalid\n', encoding='utf-8')
        self.uri = self.system.as_uri()
        self.child_uri = self.child.as_uri()
        self.source = 'system Demo {\n  node child { project = "child.uef"; role = controller; }\n}\n'
        self.notifications = []
        self.documents = {self.uri: SimpleNamespace(uri=self.uri, version=1)}
        fake_server = SimpleNamespace(workspace=SimpleNamespace(text_documents=self.documents),
                                      text_document_publish_diagnostics=self.notifications.append)
        for context in (patch.object(lsp, 'server', fake_server), patch.dict(lsp._DIAGNOSTICS_BY_URI, {}, clear=True),
                        patch.dict(lsp._IMPORTED_DIAGNOSTICS_BY_URI, {}, clear=True),
                        patch.dict(lsp._PENDING_DIAGNOSTICS, {}, clear=True), patch.dict(lsp._DIAGNOSTIC_TASKS, {}, clear=True)):
            context.start()
            self.addCleanup(context.stop)

    def make_report(self):
        span = SourceSpan(self.child, SourcePosition(3, 1, 4), SourcePosition(4, 1, 5))
        diagnostics = [source_warning(f'warning_{i}', f'Warning {i}', span) for i in range(60)]
        diagnostics.extend(source_error(f'error_{i}', f'Error {i}', span) for i in range(9))
        imported = {}
        summary = lsp._adapt_diagnostics(diagnostics, self.system, self.source, imported=imported)
        return summary, imported

    def test_many_child_diagnostics_produce_one_short_summary(self):
        summary, imported = self.make_report()
        self.assertEqual(len(summary), 1)
        self.assertEqual(summary[0].severity, types.DiagnosticSeverity.Error)
        self.assertIn('9 errors, 60 warnings', summary[0].message)
        self.assertLess(len(summary[0].message), 180)
        self.assertEqual(summary[0].range.start.line, 1)
        self.assertEqual(len(summary[0].related_information), 1)
        detailed = imported[self.child_uri]
        self.assertEqual(len(detailed), 69)
        self.assertEqual(detailed[0].message, 'Warning 0')
        self.assertEqual(detailed[0].range.end.character, 5)  # UTF-16 astral character

    def test_local_diagnostics_keep_original_message_and_location(self):
        span = SourceSpan(self.system, SourcePosition(0, 1, 1), SourcePosition(6, 1, 7))
        diagnostic = source_error('local_error', 'Invalid system', span)
        imported = {}
        result = lsp._adapt_diagnostics([diagnostic], self.system, self.source, imported=imported)
        self.assertEqual(result[0].code, 'local_error')
        self.assertEqual(result[0].message, 'Invalid system')
        self.assertEqual(result[0].range.end.character, 6)
        self.assertEqual(imported, {})

    def test_details_publish_on_child_and_clear_when_import_is_removed(self):
        summary, imported = self.make_report()
        lsp._publish_diagnostic_report(self.uri, 1, summary, imported)
        reports = {item.uri: item for item in self.notifications}
        self.assertEqual(len(reports[self.uri].diagnostics), 1)
        self.assertEqual(len(reports[self.child_uri].diagnostics), 69)
        self.assertIsNone(reports[self.child_uri].version)
        self.notifications.clear()
        lsp._publish_diagnostic_report(self.uri, 1, [], {})
        self.assertTrue(all(not report.diagnostics for report in self.notifications))
        self.assertEqual({report.uri for report in self.notifications}, {self.uri, self.child_uri})

    def test_parent_never_overwrites_newer_open_child_buffer(self):
        summary, imported = self.make_report()
        self.documents[self.child_uri] = SimpleNamespace(uri=self.child_uri, version=2)
        lsp._DIAGNOSTICS_BY_URI[self.child_uri] = (1, imported[self.child_uri])
        lsp._publish_diagnostic_report(self.uri, 1, summary, imported)
        self.assertNotIn(self.child_uri, [report.uri for report in self.notifications])
        self.notifications.clear()
        lsp._publish_diagnostic_report(self.child_uri, 2, [], {})
        self.assertEqual(self.notifications[0].diagnostics, [])
        self.assertEqual(self.notifications[0].version, 2)
        self.notifications.clear()
        lsp._publish_diagnostic_report(self.uri, 1, summary, imported)
        child_report = next(report for report in self.notifications if report.uri == self.child_uri)
        self.assertEqual(child_report.diagnostics, [])
        self.assertEqual(child_report.version, 2)

    def test_multiple_importers_do_not_duplicate_or_prematurely_clear_details(self):
        summary, imported = self.make_report()
        other = (self.root / 'other.uefs').as_uri()
        self.documents[other] = SimpleNamespace(uri=other, version=1)
        lsp._publish_diagnostic_report(self.uri, 1, summary, imported)
        lsp._publish_diagnostic_report(other, 1, summary, imported)
        self.notifications.clear()
        lsp.did_close(None, types.DidCloseTextDocumentParams(types.TextDocumentIdentifier(uri=self.uri)))
        child_report = next(report for report in self.notifications if report.uri == self.child_uri)
        self.assertEqual(len(child_report.diagnostics), 69)
        self.notifications.clear()
        lsp.did_close(None, types.DidCloseTextDocumentParams(types.TextDocumentIdentifier(uri=other)))
        child_report = next(report for report in self.notifications if report.uri == self.child_uri)
        self.assertEqual(child_report.diagnostics, [])

    def test_saving_child_refreshes_importing_system(self):
        summary, imported = self.make_report()
        lsp._publish_diagnostic_report(self.uri, 1, summary, imported)
        with patch.object(lsp, '_request_diagnostics') as request:
            lsp.did_save(None, types.DidSaveTextDocumentParams(types.TextDocumentIdentifier(uri=self.child_uri)))
        self.assertEqual([call.args[0] for call in request.call_args_list], [self.child_uri, self.uri])


if __name__ == '__main__':
    unittest.main()
