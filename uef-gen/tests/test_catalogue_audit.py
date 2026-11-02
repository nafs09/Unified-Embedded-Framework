import importlib.util
import tempfile
import unittest
from pathlib import Path


UETD = Path(__file__).resolve().parents[2] / "UETD"
spec = importlib.util.spec_from_file_location("uetd_catalogue_audit", UETD / "tools/audit_catalogue.py")
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class DeclarationAuditTests(unittest.TestCase):
    def test_missing_entry_point_is_reported_until_a_source_definition_exists(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "include").mkdir()
            (root / "src").mkdir()
            (root / "include/api.h").write_text(
                "uetd_status_t upal_example(void* output);\n", encoding="utf-8")
            self.assertEqual(len(audit.audit_public_declarations(root)), 1)
            (root / "src/api.c").write_text(
                "uetd_status_t upal_example(void* output) { return UETD_NOT_SUPPORTED; }\n",
                encoding="utf-8")
            self.assertEqual(audit.audit_public_declarations(root), [])

    def test_callback_typedef_and_inline_definition_are_not_orphan_functions(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "include").mkdir()
            (root / "src").mkdir()
            (root / "include/api.h").write_text(
                "typedef umath_status_t (*ucore_callback_t)(void* context);\n"
                "static inline int ucore_example(void) { return 0; }\n",
                encoding="utf-8")
            self.assertEqual(audit.audit_public_declarations(root), [])


if __name__ == "__main__":
    unittest.main()
