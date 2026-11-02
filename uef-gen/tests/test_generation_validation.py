import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from uef_gen.generation.pipeline import generate_project
from uef_gen.generation.validator import validate_generated_project


class GenerationValidationTests(unittest.TestCase):
    def test_staging_failure_preserves_existing_empty_destination(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / 'output'
            output.mkdir()
            snapshot = SimpleNamespace(root=root)
            manager = SimpleNamespace(root=root, verify=lambda: snapshot, warnings=[])
            resolved = SimpleNamespace(diagnostics=[], rtos='BareMetal')
            selection = SimpleNamespace(bundled_dependency_names=(), required_capabilities=(), modules=())
            with patch('uef_gen.generation.pipeline.load_extensions'), \
                 patch('uef_gen.generation.pipeline.validate_configuration', return_value=[]), \
                 patch('uef_gen.generation.pipeline.load_chip_database'), \
                 patch('uef_gen.generation.pipeline.load_development_boards'), \
                 patch('uef_gen.generation.pipeline.ResourceResolver') as resolver, \
                 patch('uef_gen.uetd_adapter.registry.load_module_manifest'), \
                 patch('uef_gen.generation.pipeline.ProjectModulePlanner') as planner, \
                 patch('uef_gen.generation.pipeline.resolve_bundled_dependencies', return_value=()), \
                 patch('uef_gen.generation.pipeline.SnapshotManager.copy_modules', side_effect=OSError('staging failed')):
                resolver.return_value.resolve.return_value = resolved
                planner.return_value.select.return_value = selection
                result = generate_project({}, output, root / 'work', snapshot_manager=manager)
            self.assertFalse(result.ok)
            self.assertIn('generation_failed', [item.code for item in result.diagnostics])
            self.assertTrue(output.is_dir())
            self.assertEqual(list(output.iterdir()), [])

    def test_declared_dependency_include_directories_are_resolved(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / 'vendor/include').mkdir(parents=True)
            (root / 'vendor/include/dependency.h').write_text('/* interface */', encoding='utf-8')
            (root / 'main.c').write_text('#include "dependency.h"\n', encoding='utf-8')
            self.assertEqual(validate_generated_project(root, ['main.c', 'vendor/include/dependency.h'], ['main.c'], include_directories=['vendor/include']), [])
            self.assertIn('generated_include_missing', [d.code for d in validate_generated_project(root, ['main.c'], ['main.c'])])

    def test_sources_and_include_directories_cannot_escape(self):
        with tempfile.TemporaryDirectory() as temporary:
            parent = Path(temporary)
            root = parent / 'project'
            root.mkdir()
            (parent / 'outside.c').write_text('int outside;\n', encoding='utf-8')
            diagnostics = validate_generated_project(root, [], ['../outside.c'], include_directories=['..'])
            self.assertEqual({d.code for d in diagnostics}, {'generated_path_escape', 'generated_include_escape'})


if __name__ == '__main__':
    unittest.main()
