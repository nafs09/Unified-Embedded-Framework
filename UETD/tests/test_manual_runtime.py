import importlib.util
import json
import tempfile
import subprocess
import sys
import unittest
from pathlib import Path
from jinja2 import Environment, FileSystemLoader, StrictUndefined

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("manual_runtime", ROOT / "tools/render_manual_runtime.py")
renderer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(renderer)

class ManualRuntimeTests(unittest.TestCase):
    def document(self):
        return json.loads((ROOT / "examples/manual_runtime/runtime.json").read_text())

    def test_committed_example_is_current(self):
        for name, text in renderer.render(self.document()).items():
            self.assertEqual((ROOT / "examples/manual_runtime/generated" / name).read_text(), text, name)

    def test_invalid_inputs_fail_before_rendering(self):
        changes = [
            lambda d: d["machines"][0].update(name="../escape"),
            lambda d: d["machines"][0].update(initial="missing"),
            lambda d: d["machines"][0]["events"].update(duplicate=1),
            lambda d: d.update(event_queue_capacity=256),
            lambda d: d["machines"][0]["states"].append({"name": "IDLE"}),
            lambda d: d["machines"][0]["transitions"].append({"from": "running", "event": "stop", "to": "idle"}),
            lambda d: d["machines"][0]["transitions"][0].update(guard="x(); bad()"),
            lambda d: d.update(tasks=[{}]),
            lambda d: d["machines"][0].update(name="callbacks"),
            lambda d: d["machines"][0].update(name="FreeRTOSConfig"),
            lambda d: d["machines"][0].update(name="CON"),
            lambda d: d["machines"][0].update(transitons=[]),
            lambda d: d["machines"][0]["transitions"][0].update(guard="hooks"),
            lambda d: d.update(schema_version=True),
        ]
        for change in changes:
            d = self.document(); change(d)
            with self.subTest(change=change), self.assertRaises(ValueError): renderer.render(d)
        with self.assertRaises(ValueError):
            json.loads('{"schema_version": 1, "schema_version": 2}', object_pairs_hook=renderer.unique_object)

    def test_any_expansion_preserves_order_and_zero_transition_tables(self):
        d = self.document(); m = d["machines"][0]
        m["transitions"] = [{"from": "any", "to": "idle", "event": "stop", "guard": "can_stop"},
                             {"from": "idle", "to": "running", "event": "stop"}]
        rows = renderer.prepare(d)["machines"][0]["rows"]
        self.assertEqual([(r["source"], r["guard"]) for r in rows], [(0, "can_stop"), (1, "can_stop"), (0, None)])
        m["transitions"] = []
        self.assertNotIn("transitions[]", renderer.render(d)["controller.c"])

    def test_task_storage_and_errors_are_explicit(self):
        d = self.document(); d["backend"] = "FREERTOS"
        d["tasks"] = [{"name": "owner", "operation": "owner_step", "priority": 2, "stack_bytes": 1024, "period_us": 1000}]
        rendered = renderer.render(d)
        self.assertIn("uos_task_create_static", rendered["tasks.c"])
        self.assertIn("manual_runtime_component_failure(status)", rendered["tasks.c"])
        self.assertIn("next += UINT64_C(1000)", rendered["tasks.c"])
        self.assertIn("attempted", rendered["tasks.c"])
        self.assertIn("uos_stack_word_t", rendered["tasks.c"])

    def test_render_preserves_application_edits(self):
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            config = folder / "input.json"
            config.write_text(json.dumps(self.document()), encoding="utf-8")
            output = folder / "generated"
            command = [sys.executable, str(ROOT / "tools/render_manual_runtime.py"), str(config), str(output)]
            for _ in range(2):
                result = subprocess.run(command, capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stderr)
            application = output / "application.c"
            application.write_text("/* Application-owned. */", encoding="utf-8")
            edited = output / "controller.c"
            edited.write_text("/* Edited generated file. */", encoding="utf-8")
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("refusing to replace edited output", result.stderr)
            self.assertEqual(edited.read_text(), "/* Edited generated file. */")
            self.assertEqual(application.read_text(), "/* Application-owned. */")

    def test_every_uos_uapp_route_renders_and_has_current_resources(self):
        index = json.loads((ROOT / "templates/modules/index.json").read_text())
        env = Environment(loader=FileSystemLoader(ROOT), undefined=StrictUndefined)
        for name, record in index["modules"].items():
            if not name.startswith(("uos", "uapp")): continue
            with self.subTest(module=name):
                self.assertFalse(record["missing_resources"])
                text = env.get_template(record["template"]).render()
                self.assertNotIn("{%", text)
                self.assertNotIn("{{", text)
                for resource in record["sources"] + record["headers"]:
                    self.assertTrue((ROOT / resource).is_file(), resource)

if __name__ == "__main__": unittest.main()
