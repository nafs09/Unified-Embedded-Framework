import json
import tempfile
import unittest
from pathlib import Path

from uef_gen.modules.selector import ModuleSelectionError, ModuleSelector
from uef_gen.uetd_adapter.registry import (
    UETDRegistryError,
    load_first_pass_catalogue,
    load_future_catalogue,
    load_module_manifest,
)

UETD = Path(__file__).resolve().parents[2] / "UETD"


class CataloguePlacementTests(unittest.TestCase):
    def test_scope_catalogues_are_disjoint_and_keep_later_canopen_fd(self):
        first = load_first_pass_catalogue(UETD)
        later = load_future_catalogue(UETD)
        self.assertFalse(first["items"].keys() & later["items"].keys())
        for name in ("uapp.secure_update", "uapp.fault_history", "umid.storage_transactions",
                     "uproto.tls", "uproto.mqtt", "uproto.http", "uproto.modbus", "uproto.canopen",
                     "deploy.signed_update", "project_language.type_interchange"):
            self.assertIn(name, first["items"])
        self.assertIn("uproto.canopen_fd", later["items"])
        self.assertIn("uproto.medi_can", later["items"])
        modules = load_module_manifest(UETD)["modules"]
        self.assertIn("uproto/future/industrial/canopen_fd", modules)
        self.assertNotIn("uproto/canopen_fd", modules)

    def test_promoted_module_identity_does_not_grant_generation(self):
        modules = load_module_manifest(UETD)["modules"]
        selector = ModuleSelector(UETD)
        for module in ("umid/storage", "umid/calibration", "umid/blackbox", "umid/process_image",
                       "uproto/tls", "uproto/mqtt", "uproto/http", "uproto/modbus", "uproto/canopen",
                       "ucon/optimization/qp/qp_active_set"):
            with self.subTest(module=module):
                self.assertEqual(modules[module]["phase"], 0)
                self.assertFalse(modules[module]["available_for_generation"])
                self.assertFalse(any("/future/" in path for path in
                                     modules[module]["sources"] + modules[module]["headers"]))
                with self.assertRaises(ModuleSelectionError):
                    selector.select([module])
        self.assertNotIn("umid/future/storage", modules)
        self.assertNotIn("uproto/future/industrial/canopen", modules)
        self.assertEqual(modules["ucon/optimization/qp/qp_active_set"]["priority"], "FIRST_PASS")

    def test_implemented_portable_services_are_selectable(self):
        modules = load_module_manifest(UETD)["modules"]
        selector = ModuleSelector(UETD)
        for module in ("uapp/secure_update", "uapp/fault_history"):
            with self.subTest(module=module):
                self.assertFalse(modules[module]["scaffold_only"])
                self.assertTrue(modules[module]["available_for_generation"])
                self.assertEqual(modules[module]["contract_status"], "portable_implemented")
                self.assertIn(module, selector.select([module]).modules)

    def test_design_loader_rejects_a_generation_ready_design(self):
        api = json.loads((UETD / "api.json").read_text(encoding="utf-8"))
        catalogue = json.loads((UETD / api["first_pass_catalogue"]).read_text(encoding="utf-8"))
        catalogue["items"]["uapp.secure_update"]["generation"] = "ready"
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "api.json").write_text(json.dumps(api), encoding="utf-8")
            for relative, text in (
                (api["first_pass_catalogue"], json.dumps(catalogue)),
                (catalogue["registry_schema"], (UETD / catalogue["registry_schema"]).read_text(encoding="utf-8")),
            ):
                path = root / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text, encoding="utf-8")
            with self.assertRaisesRegex(UETDRegistryError, "generation"):
                load_first_pass_catalogue(root)


if __name__ == "__main__":
    unittest.main()
