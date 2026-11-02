"""Keep physical topic grouping independent of public module identity."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

from jsonschema import Draft202012Validator
from uef_gen.templates import render_uetd_template

UETD = Path(__file__).resolve().parents[2] / "UETD"


def maintenance_tool(name):
    spec = importlib.util.spec_from_file_location(name, UETD / "tools" / f"{name}.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class UETDLayoutTests(unittest.TestCase):
    def test_complete_authored_tree_and_routes_follow_layout_policy(self):
        self.assertEqual(maintenance_tool("audit_layout").audit_layout(UETD), [])

    def test_target_indexes_accept_flat_and_vendor_groups_without_traversal(self):
        for kind in ("families", "chips", "development_boards"):
            directory = UETD / "registry/targets" / kind
            schema = json.loads((directory / "registry.schema.json").read_text(encoding="utf-8"))
            index = json.loads((directory / "registry.json").read_text(encoding="utf-8"))
            with self.subTest(catalogue=kind):
                Draft202012Validator(schema).validate(index)
                validator = Draft202012Validator(schema["properties"]["entries"]["items"]["properties"]["path"])
                for suffix in ("record.json", "vendor/record.json"):
                    self.assertTrue(validator.is_valid(f"registry/targets/{kind}/manifest/{suffix}"))
                for suffix in ("../record.json", "vendor/../../record.json", "a/b/record.json"):
                    self.assertFalse(validator.is_valid(f"registry/targets/{kind}/manifest/{suffix}"))

    def test_single_file_rule_keeps_layer_and_vendor_exceptions(self):
        tool = maintenance_tool("audit_layout")
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for relative in ("src/usec/provider.c", "src/umath/topic/one.c",
                             "third_party/upstream/one.c", "out/build/one.obj"):
                path = root / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.touch()
            self.assertEqual(tool.audit_layout(root), ["single-file folder: src/umath/topic (one.c)"])

    def test_registry_write_preserves_grouped_paths_and_public_identity(self):
        tool = maintenance_tool("registry_io")
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            grouped = "registry/modules/manifest/upal/can/fdcan_fd.json"
            index = {"modules": {"upal/fdcan_fd": {"manifest": grouped, "DSL_descriptor": "FDCAN_FD"}}}
            (root / "api.json").write_text(json.dumps({"module_registry": "registry/modules/registry.json"}))
            (root / grouped).parent.mkdir(parents=True)
            (root / grouped).write_text(json.dumps({"sources": ["src/upal/can/fdcan_fd.c"]}))
            index_file = root / "registry/modules/registry.json"
            index_file.write_text(json.dumps(index))
            document = tool.load_module_registry(root)
            tool.write_module_registry(document, root)
            self.assertEqual(json.loads(index_file.read_text()), index)
            self.assertEqual(tool.load_module_registry(root), document)
            self.assertFalse((root / "registry/modules/manifest/upal/fdcan_fd.json").exists())

    def test_all_routes_render_using_indexed_paths_and_stable_target_names(self):
        routes = json.loads((UETD / "templates/modules/index.json").read_text())["modules"]
        tool = maintenance_tool("generate_module_templates")
        for name, route in routes.items():
            with self.subTest(module=name):
                relative = Path(route["template"]).relative_to("templates").as_posix()
                rendered = render_uetd_template(UETD / "templates", relative, {})
                self.assertIn(f'"uetd_module_{tool._target_id(name)}"', rendered)
                for resource in route["sources"] + route["headers"]:
                    self.assertIn(f'${{UETD_ROOT}}/{resource}', rendered)

    def test_later_scaffold_keeps_scope_when_colocated_with_its_family(self):
        tool = maintenance_tool("generate_module_templates")
        self.assertTrue(tool._is_future("umath/future/matrix/sparse_csr", {"scaffold_stage": "future"},
                                        ["src/umath/matrix/sparse_csr.c"]))
        self.assertEqual(tool._output_relative_path("upal/fdcan_fd", "registry/modules/manifest/upal/can/fdcan_fd.json"),
                         Path("upal/can/fdcan_fd.cmake.tmpl"))
        with self.assertRaises(ValueError):
            tool._output_relative_path("upal/fdcan_fd", "registry/modules/manifest/../../outside.json")

    def test_relocated_later_type_does_not_gain_language_visibility(self):
        tool = maintenance_tool("generate_project_language_catalogues")
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            tool.UETD = root
            relative = "include/umath/matrix/sparse_csr.h"
            header = root / relative
            header.parent.mkdir(parents=True)
            header.write_text("typedef enum { DRAFT_VALUE } umath_draft_t;\n")
            manifest = root / "later.json"
            manifest.write_text(json.dumps({"headers": [relative, "include/ucore/types.h"], "scaffold_stage": "future"}))
            (root / "core.json").write_text(json.dumps({"headers": ["include/ucore/types.h"]}))
            index = {"modules": {"umath/future/matrix/sparse_csr": {"manifest": "later.json"},
                                 "ucore": {"manifest": "core.json"}}}
            later_headers = tool.future_header_paths(index)
            self.assertEqual(later_headers, {relative})
            entry, = tool.collect_types([header], future_headers=later_headers)
            self.assertEqual(entry["source_status"], "future_header")
            self.assertNotIn("language_contract", entry)

    def test_relocated_internal_math_helper_is_not_a_language_operation(self):
        tool = maintenance_tool("generate_project_language_catalogues")
        header = UETD / "include/umath/internal_scalar_math.h"
        functions = tool.collect_functions([header], {"modules": {}}, {}, {})
        self.assertEqual(functions, [])


if __name__ == "__main__":
    unittest.main()
