"""Exercise the shipped examples against the live sibling UETD registry."""

import contextlib
import io
import json
import re
import unittest
from pathlib import Path

from uef_gen.cli import main
from uef_gen.language.frontends.project import compile_project_text
from uef_gen.language.frontends.project.lexer import Lexer
from uef_gen.language.frontends.project.source_formatting import format_source

ROOT = Path(__file__).resolve().parents[1]
UETD = ROOT.parent / 'UETD'


@unittest.skipUnless((UETD / 'api.json').is_file(), 'Live UETD checkout required')
class ExampleTests(unittest.TestCase):
    def test_example_checks_and_formatting(self):
        expected_errors = {
            'esp32_mecanum_sorter_freertos.uef': 'unknown_uetd_descriptor',
            'tesla_coil_h745_controller.uef': 'model_equations_empty',
            'tesla_coil_system.uefs': 'model_equations_empty',
        }
        paths = sorted(path for path in (ROOT / 'examples').rglob('*') if path.suffix in {'.uef', '.uefs'})
        self.assertEqual(len(paths), 11)
        for path in paths:
            with self.subTest(file=path.name):
                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    code = main(['check', str(path), '--uetd', str(UETD)])
                response = json.loads(output.getvalue())
                self.assertEqual(code, 2 if path.name in expected_errors else 0)
                if path.name in expected_errors:
                    self.assertIn(expected_errors[path.name], [item['code'] for item in response['diagnostics']])
                else:
                    self.assertTrue(response['ok'])
                    self.assertTrue(response['diagnostics'], 'Readiness warnings must remain visible')
                source = path.read_text(encoding='utf-8-sig')
                formatted = format_source(source)
                self.assertEqual(format_source(formatted), formatted)
                self.assertEqual([(t.kind, t.text) for t in Lexer(source).tokenize()[0]],
                                 [(t.kind, t.text) for t in Lexer(formatted).tokenize()[0]])

    def test_default_project_snippet_compiles_without_diagnostics(self):
        snippets = json.loads((ROOT / 'editors/vscode/snippets/uef.json').read_text(encoding='utf-8'))
        source = '\n'.join(snippets['UEF-PL project shell']['body'])
        source = re.sub(r'\$\{\d+:([^}]*)\}', r'\1', source).replace('${0}', '')
        result = compile_project_text(source, uetd_root=UETD)
        self.assertTrue(result.ok, [item.to_dict() for item in result.diagnostics])
        self.assertEqual(result.diagnostics, [])


if __name__ == '__main__':
    unittest.main()
