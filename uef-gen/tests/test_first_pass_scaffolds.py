import unittest
from pathlib import Path

from uef_gen.modules.selector import ModuleSelectionError, ModuleSelector
from uef_gen.uetd_adapter.registry import load_generation_catalogue, load_module_manifest

UETD = Path(__file__).resolve().parents[2] / "UETD"
EXPECTED = (
    "upal/capability_contracts",
    "ucore/codec",
    "upal/transaction",
    "upal/dsi_host",
    "upal/display_panel",
    "upal/touch",
    "upal/power",
    "upal/block_device",
    "umid/buffer_pool",
    "umid/mailbox",
    "umid/event_bus",
    "umid/sensor",
    "umid/settings",
    "umid/input",
    "umid/graphics",
    "uactuator/actuator",
    "usec/provider",
    "uproto/connection_manager",
    "uproto/usb_device",
    "uproto/time_sync",
    "uproto/framing",
)


class FirstPassScaffoldTests(unittest.TestCase):
    def test_implemented_portable_modules_are_selectable(self):
        modules = load_module_manifest(UETD)["modules"]
        selector = ModuleSelector(UETD)
        for name in ('ucore/span', 'ucore/deadline', 'uos/deferred', 'uapp/product_session', 'uapp/diagnostics', 'uapp/fault_history', 'uapp/secure_update'):
            with self.subTest(module=name):
                self.assertFalse(modules[name]["scaffold_only"])
                self.assertTrue(modules[name]["available_for_generation"])
                self.assertIn(name, selector.select([name]).modules)

    def test_host_obligations_are_blocked_and_separate_from_c_modules(self):
        catalogue = load_generation_catalogue(UETD)
        modules = load_module_manifest(UETD)["modules"]
        for name, item in catalogue["items"].items():
            with self.subTest(capability=name):
                self.assertNotIn(name, modules)
                self.assertEqual(item["generation"], "blocked")
                self.assertEqual(item["status"], "design_only")
                self.assertEqual(item["priority"], "FIRST_PASS")
        self.assertIn("generation.typed_emission", catalogue["items"])
        self.assertIn("generation.deployment", catalogue["items"])

    def test_scaffolds_are_discoverable_but_cannot_be_selected(self):
        modules = load_module_manifest(UETD)["modules"]
        selector = ModuleSelector(UETD)
        for name in EXPECTED:
            with self.subTest(module=name):
                record = modules[name]
                self.assertEqual(record["priority"], "FIRST_PASS")
                self.assertEqual(record["phase"], 0)
                self.assertTrue(record["scaffold_only"])
                self.assertFalse(record["available_for_generation"])
                self.assertTrue(record["sources"])
                for path in record["headers"] + record["sources"]:
                    self.assertNotIn("/future/", path)
                    self.assertTrue((UETD / path).is_file(), path)
                with self.assertRaises(ModuleSelectionError):
                    selector.select([name])

    def test_new_modules_have_documented_first_pass_ownership(self):
        modules = load_module_manifest(UETD)["modules"]
        for name in EXPECTED:
            with self.subTest(module=name):
                document = modules[name]["design_contract"].split("#", 1)[0]
                self.assertTrue((UETD / document).is_file(), document)


if __name__ == "__main__":
    unittest.main()
