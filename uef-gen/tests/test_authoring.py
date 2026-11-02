"""Regression coverage for source, CLI and shared editor authoring behavior."""

import contextlib
import io
import json
import re
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from uef_gen.cli import main
from uef_gen.cli_output import format_response
from uef_gen.language.frontends.project.ast import FlagsOperationExpression, SourcePosition, SourceSpan
from uef_gen.language.frontends.project.diagnostics import source_error, source_warning
from uef_gen.language.frontends.project.lexer import Lexer
from uef_gen.language.frontends.project.parser import Parser
from uef_gen.language.frontends.project.source_formatting import format_source
from uef_gen.language.frontends.system.compiler import compile_system_document


class SourceTests(unittest.TestCase):
    def test_flags_operations_parse_without_crashing(self):
        for operation in ("has", "set", "clear"):
            with self.subTest(operation=operation):
                tokens, diagnostics = Lexer(f"{operation}(value, Mode.ready)").tokenize()
                self.assertFalse(diagnostics)
                parser = Parser(tokens)
                expression = parser._parse_expression()
                self.assertIsInstance(expression, FlagsOperationExpression)
                self.assertEqual(expression.operation, operation)
                self.assertFalse(parser.diagnostics)

    def test_formatter_preserves_tokens_and_indents_continuations(self):
        source = 'project Demo {\nvalue = map(\nx = [1,\n2],\ny = "} # literal"\n); // }\n}\n'
        output = format_source(source)
        self.assertIn('\n    x = [1,\n      2],', output)
        self.assertIn('\n  ); // }', output)
        self.assertIn('"} # literal"', output)
        self.assertEqual([(t.kind, t.text) for t in Lexer(source).tokenize()[0]],
                         [(t.kind, t.text) for t in Lexer(output).tokenize()[0]])
        self.assertEqual(format_source(output), output)

    def test_formatter_preserves_and_controls_newlines(self):
        for newline in ("\n", "\r\n", "\r"):
            for ending in ("", newline, newline * 3):
                with self.subTest(newline=newline, ending=ending):
                    source = 'project X {' + newline + '}' + ending
                    self.assertEqual(format_source(source), source)
                    self.assertEqual(format_source(source, insert_final_newline=True, trim_final_newlines=True),
                                     'project X {' + newline + '}' + newline)

    def test_unterminated_string_spaces_are_not_deleted(self):
        self.assertEqual(format_source('value = "unfinished  '), 'value = "unfinished  ')

    def test_format_write_preserves_bom_and_crlf(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'sample.uef'
            path.write_bytes(b'\xef\xbb\xbfproject Demo {\r\nvalue = 1;\r\n}\r\n')
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(main(['format', str(path), '--check']), 1)
                self.assertEqual(main(['format', str(path), '--write']), 0)
                self.assertEqual(main(['format', str(path), '--check']), 0)
            self.assertEqual(path.read_bytes(), b'\xef\xbb\xbfproject Demo {\r\n  value = 1;\r\n}\r\n')


class CommandTests(unittest.TestCase):
    def test_hardware_field_diagnostics_render_as_text(self):
        output = format_response({'ok': False, 'diagnostics': [{'severity': 'error', 'code': 'hardware_config_schema',
                                  'message': 'Invalid DMA config', 'location': 'peripherals'}]}, command='validate')
        self.assertIn('field: peripherals', output)
        self.assertIn('Invalid DMA config', output)

    def run_cli(self, arguments):
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            code = main(arguments)
        return code, output.getvalue()

    def test_check_uses_frontend_and_retains_warnings(self):
        response = {'ok': True, 'diagnostics': [source_warning('not_ready', 'Target not ready').to_dict()], 'application': {}}
        compiled = SimpleNamespace(ok=True, to_dict=lambda: response)
        with patch('uef_gen.cli._compile_uetd_source', return_value=compiled), patch('uef_gen.cli._module_selection') as select:
            code, output = self.run_cli(['check', 'sample.uef'])
            self.assertEqual(code, 0)
            self.assertEqual(json.loads(output), response)
            select.assert_not_called()
            code, output = self.run_cli(['check', 'sample.uef', '--format', 'text'])
            self.assertIn('CHECK PASSED', output)
            self.assertIn('WARNING [not_ready]', output)
            self.assertIn('Firmware emission, build, and flash remain unavailable', output)

    def test_generate_dry_run_retains_compiler_warnings_and_readiness_errors(self):
        compiled = SimpleNamespace(ok=True, hardware_configuration={}, diagnostics=[source_warning('source_warning', 'Source warning')])
        with patch('uef_gen.cli._compile_uetd_source', return_value=compiled), patch('uef_gen.cli._module_selection', return_value=(None, None, [source_error('blocked', 'Generation blocked')])):
            code, output = self.run_cli(['generate', 'sample.uef', '--dry-run'])
        self.assertEqual(code, 2)
        self.assertEqual([d['code'] for d in json.loads(output)['diagnostics']], ['source_warning', 'blocked'])

    def test_system_dry_run_returns_plan_without_enabling_emission(self):
        compiled = SimpleNamespace(ok=True, to_dict=lambda: {'ok': True, 'build_plan': {'system_name': 'Demo'}})
        with patch('uef_gen.cli._compile_uefs_source', return_value=compiled):
            code, output = self.run_cli(['generate', 'sample.uefs', '--dry-run'])
            self.assertEqual(code, 0)
            self.assertEqual(json.loads(output)['build_plan']['system_name'], 'Demo')
            self.assertEqual(self.run_cli(['generate', 'sample.uefs'])[0], 2)
            self.assertEqual(self.run_cli(['generate', 'sample.uefs', '--dry-run', '--target', 'wrong'])[0], 2)

    def test_system_keeps_warnings_from_valid_nodes(self):
        warning = source_warning('child_warning', 'Child readiness warning')
        document = SimpleNamespace(source_path=Path('system.uefs'), nodes=[SimpleNamespace(name='child', project_path=Path('child.uef'))])
        plan = SimpleNamespace(links=[], source_map={})
        with patch('uef_gen.language.frontends.system.compiler.validate_system', return_value=[]), \
             patch('uef_gen.language.frontends.system.compiler.compile_project_file', return_value=SimpleNamespace(ok=True, diagnostics=[warning])), \
             patch('uef_gen.language.frontends.system.compiler.lower_system_document', return_value=(plan, [])):
            result = compile_system_document(document, uetd_root=Path('.'))
        self.assertTrue(result.ok)
        self.assertEqual(result.diagnostics, [warning])


try:
    from lsprotocol import types
    from uef_gen.language.server import server as lsp
except ImportError:
    lsp = None


@unittest.skipIf(lsp is None, 'Install the language-server extra for editor regressions')
class EditorTests(unittest.TestCase):
    def test_member_completion_has_receiver_documentation(self):
        line = 'sensor.va'
        match = re.search(r'(\w+)\.(\w*)$', line)
        with patch.object(lsp, '_parse_partial_source', return_value=None), patch.object(lsp, '_catalogue_for_document', return_value=None), \
             patch.object(lsp, '_members_for_receiver', return_value=[('value', 'f32', 'Sample output')]):
            items = lsp._member_completion_items(Path('sample.uef').resolve().as_uri(), line, 0, line, match)
        self.assertEqual(items[0].text_edit.new_text, 'value')
        self.assertIn('sensor.value', items[0].documentation.value)

    def test_system_keywords_use_packaged_grammar(self):
        self.assertTrue({'project', 'role', 'from', 'to', 'external_rtl'}.issubset(lsp._system_language_words()))

    def test_formatting_edit_covers_eof_and_is_idempotent(self):
        for newline in ('\n', '\r\n'):
            source = f'project X {{{newline}name = "😀";{newline}}}{newline}{newline}'
            edit = lsp._formatting_edit(source, 2, True, True, True, True)
            end = lsp._offset_from_lsp(source, edit.range.end)
            self.assertEqual(end, len(source))
            updated = edit.new_text + source[end:]
            self.assertEqual(updated, f'project X {{{newline}  name = "😀";{newline}}}{newline}')
            self.assertIsNone(lsp._formatting_edit(updated, 2, True, True, True, True))

    def test_end_at_next_line_zero_does_not_format_next_line(self):
        document = SimpleNamespace(source='project X {\nvalue = 1;\nother = 2;\n}\n')
        params = types.DocumentRangeFormattingParams(text_document=types.TextDocumentIdentifier(uri='file:///sample.uef'),
                    range=types.Range(start=types.Position(1, 0), end=types.Position(2, 0)),
                    options=types.FormattingOptions(tab_size=2, insert_spaces=True))
        with patch.object(lsp, 'server', SimpleNamespace(workspace=SimpleNamespace(get_text_document=lambda uri: document))):
            edits = lsp.range_formatting(None, params)
        self.assertEqual(edits[0].range.end.line, 1)
        self.assertEqual(edits[0].new_text, '  value = 1;')

    def test_foreign_diagnostics_link_child_and_anchor_system_node(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            child = root / 'child.uef'
            child.write_text('// 😀\ninvalid\n', encoding='utf-8')
            span = SourceSpan(child, SourcePosition(3, 1, 4), SourcePosition(4, 1, 5))
            diagnostic = source_error('child_error', 'Child failure', span)
            source = 'system Demo {\n  node child { project = "child.uef"; role = controller; }\n}\n'
            adapted = lsp._adapt_diagnostics([diagnostic], root / 'demo.uefs', source)
            self.assertEqual(adapted[0].range.start.line, 1)
            self.assertEqual(adapted[0].related_information[0].location.uri, child.as_uri())
            self.assertEqual(adapted[0].related_information[0].location.range.end.character, 5)


if __name__ == '__main__':
    unittest.main()
