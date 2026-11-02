"""CPU, platform, physical assembly and programming recipe remain independent."""
import json
import unittest
from dataclasses import replace
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from jsonschema import Draft202012Validator
from uef_gen.chips.database import ChipDatabase
from uef_gen.chips.specs import ArchSpec, ChipSpec, FamilySpec
from uef_gen.generation.manifest import build_manifest
from uef_gen.language.frontends.project import compile_project_text
from uef_gen.modules.planner import select_uhal_module
from uef_gen.modules.selector import ModuleSelectionError, ModuleSelector
from uef_gen.resolver.hardware import compose_hardware_profile, extend_memory_regions
from uef_gen.resolver.resource_resolver import ResourceResolver

ROOT = Path(__file__).resolve().parents[2]
UETD = ROOT / "UETD"

def resolved(architecture="Cortex-M4F", family="STM32G4", hardware=None):
    chip = ChipSpec("TestChip", FamilySpec(family, ArchSpec(architecture, 32)), "test", 32,
                    65536, 8192, verified=True, registry_id="test-chip", generation_available=True)
    database = ChipDatabase()
    database.register(chip)
    config = {"target": {"chip": "test-chip", "hardware": hardware or {"build": {"backend": "cmake", "toolchain": "arm-none-eabi"}}}, "modules": []}
    return ResourceResolver(database).resolve(config)

class ArchitectureTests(unittest.TestCase):
    def test_exact_chip_can_bind_a_platform_without_python_chip_branches(self):
        project = resolved()
        binding = {"default_platform": "custom_registers", "platforms": {"custom_registers": {
            "architecture": "CORTEX_M", "module": "uhal/test_platform", "build_backend": "cmake"}}}
        project = replace(project, chip=replace(project.chip, resources={"uhal": binding}))
        modules = {"uhal/test_platform": {"architecture": "CORTEX_M"}}
        self.assertEqual(select_uhal_module(project, modules), "uhal/test_platform")
        with self.assertRaisesRegex(ModuleSelectionError, "unknown module"):
            select_uhal_module(project, {})
        with self.assertRaisesRegex(ModuleSelectionError, "does not declare compatibility"):
            select_uhal_module(project, {"uhal/test_platform": {"architecture": "AVR"}})
        with self.assertRaisesRegex(ModuleSelectionError, "not registered for this chip"):
            select_uhal_module(replace(project, hardware_profile={"build": {"backend": "cmake", "settings": {"platform": "foreign"}}}), modules)
        with self.assertRaisesRegex(ModuleSelectionError, "requires build backend"):
            select_uhal_module(replace(project, hardware_profile={"build": {"backend": "esp_idf"}}), modules)

    def test_only_explicit_host_architecture_selects_simulation(self):
        for architecture in ("AVR 8-bit", "Xtensa LX6", "RISC-V RV32IMC", "unknown", "ARM", ""):
            with self.subTest(architecture=architecture):
                try:
                    module = select_uhal_module(resolved(architecture))
                except ModuleSelectionError:
                    continue
                self.assertNotEqual(module, "uhal/x86")
        self.assertEqual(select_uhal_module(resolved("HOST")), "uhal/x86")

    def test_core_and_sdk_are_separate(self):
        hardware = {"build": {"backend": "esp_idf", "toolchain": "esp-idf-classic-esp32"}}
        self.assertEqual(select_uhal_module(resolved("Xtensa LX6", "ESP32", hardware)), "uhal/esp_idf")
        self.assertEqual(select_uhal_module(resolved("RISC-V RV32IMC", "ESP32-C3", hardware)), "uhal/esp_idf")
        with self.assertRaises(ModuleSelectionError):
            select_uhal_module(resolved("Cortex-M4F", "STM32G4", hardware))
        with self.assertRaises(ModuleSelectionError):
            select_uhal_module(resolved("RISC-V RV32IMC", "CH32V", hardware))

    def test_registry_preserves_chip_core_count(self):
        from uef_gen.chips.registry_records import load_chip_database
        database = load_chip_database(UETD)
        self.assertEqual(database.get("esp32-d0wdq6-v3").core_count, 2)
        self.assertEqual(database.get("atmega328p-pu").core_count, 1)

    def test_unfinished_hardware_variants_remain_blocked(self):
        for module in ("uhal/avr", "uhal/esp_idf", "uhal/arm_cm"):
            with self.subTest(module=module), self.assertRaisesRegex(ModuleSelectionError, "scaffold"):
                ModuleSelector(UETD).select([module])

    def test_conflicting_implementations_cannot_link_together(self):
        manifest = {"modules": {name: {"sources": [], "headers": [], "dependencies": []} for name in ("a", "b")},
                    "selection_groups": {"uhal": {"selection": "exactly_one", "options": {"A": "a", "B": "b"}}}}
        with patch("uef_gen.modules.selector.load_module_manifest", return_value=manifest):
            with self.assertRaisesRegex(ModuleSelectionError, "Conflicting uhal"):
                ModuleSelector(UETD).select(["a", "b"])

class HardwareTests(unittest.TestCase):
    def test_custom_chip_has_no_development_board_requirement(self):
        project = resolved()
        self.assertTrue(project.valid, project.diagnostics)
        self.assertIsNone(project.development_board)
        self.assertEqual(project.hardware_profile["build"]["toolchain"], "arm-none-eabi")

    def test_missing_build_reports_missing_information(self):
        project = resolved(hardware={"reserved_pins": []})
        self.assertIn("target_build_profile_required", [d.code for d in project.diagnostics])
        self.assertNotIn("development_board_required", [d.code for d in project.diagnostics])

    def test_preset_is_not_mutated_and_reserved_pins_are_preserved(self):
        board = {"reserved_pins": ["PA1"], "clock_sources": {"hse": 8000000},
                 "build": {"backend": "cmake", "toolchain": "arm-none-eabi", "settings": {"old": True}}}
        profile, errors = compose_hardware_profile({"hardware": {"reserved_pins": ["PA2"], "clock_sources": {"hse": 16000000}}}, board)
        self.assertFalse(errors)
        self.assertEqual(profile["reserved_pins"], ["PA1", "PA2"])
        self.assertEqual(board["clock_sources"]["hse"], 8000000)
        self.assertEqual(board["reserved_pins"], ["PA1"])

    def test_changed_backend_does_not_inherit_foreign_settings(self):
        board = {"flash": {"backend": "stlink", "toolchain": "old", "settings": {"foreign": True}}}
        profile, errors = compose_hardware_profile({"hardware": {"flash": {"backend": "openocd", "toolchain": "openocd"}}}, board)
        self.assertFalse(errors)
        self.assertNotIn("settings", profile["flash"])

    def test_custom_settings_cannot_claim_chip_readiness(self):
        _, errors = compose_hardware_profile({"hardware": {"generation_available": True}}, None)
        self.assertEqual([d.code for d in errors], ["hardware_field_unknown"])

    def test_bad_clock_and_recipe_are_rejected(self):
        _, errors = compose_hardware_profile({"hardware": {"clock_sources": {"osc": True}, "build": {"backend": "cmake"}}}, None)
        self.assertEqual({d.code for d in errors}, {"hardware_clock_invalid", "hardware_recipe_incomplete"})

    def test_custom_profile_reaches_generated_manifest_without_board(self):
        hardware = {"build": {"backend": "cmake", "toolchain": "arm-none-eabi"},
                    "flash": {"backend": "openocd", "toolchain": "openocd", "interface": "swd"}}
        project = resolved(hardware=hardware)
        manifest = build_manifest(project, SimpleNamespace(version="test", source_hash="0" * 64), (), [], [], [], [], [], [], (), (), [], {})
        self.assertNotIn("development_board", manifest["target"])
        self.assertEqual(manifest["target"]["architecture"], "CORTEX_M")
        self.assertEqual(manifest["target"]["flash_profile"], hardware["flash"])

    def test_chip_memory_cannot_be_redefined_or_overlapped(self):
        chip = {"ram": {"origin": 4096, "length_bytes": 1024}}
        for memory in ({"ram": {"origin": 0, "length_bytes": 8192}}, {"external": {"origin": 4096, "length_bytes": 1024}}):
            regions, errors = extend_memory_regions(chip, {"memory_regions": memory})
            self.assertTrue(errors)
            self.assertEqual(regions, chip)
        regions, errors = extend_memory_regions(chip, {"memory_regions": {"external": {"origin": 8192, "length_bytes": 1024}}})
        self.assertFalse(errors)
        self.assertIn("external", regions)

    def test_harvard_address_spaces_can_share_numeric_addresses(self):
        regions, errors = extend_memory_regions({"program": {"origin": 0, "length_bytes": 1024, "address_space": "program"},
                                                "ram": {"origin": 4096, "length_bytes": 1024, "address_space": "data"}},
                                               {"memory_regions": {"data": {"origin": 0, "length_bytes": 1024, "address_space": "data"}}})
        self.assertFalse(errors)
        self.assertEqual(len(regions), 3)

    def test_project_cannot_invent_address_spaces_to_hide_overlap(self):
        _, errors = extend_memory_regions({"ram": {"origin": 4096, "length_bytes": 1024}},
                                          {"memory_regions": {"external": {"origin": 4096, "length_bytes": 1024, "address_space": "invented"}}})
        self.assertEqual([d.code for d in errors], ["hardware_memory_address_space_unknown"])

    def test_custom_sdk_owns_startup_without_board_record(self):
        from uef_gen.templates.project_scaffold import _render_target_files
        project = resolved("Xtensa LX6", "ESP32", {"build": {"backend": "esp_idf", "toolchain": "esp-idf-classic-esp32"}})
        self.assertEqual([item.path for item in _render_target_files(project)], ["target/README.md"])

class LanguageTests(unittest.TestCase):
    def source(self, properties):
        return f'''project Custom {{
          target = "stm32g474ret6";
          hardware {{ {properties} }}
          subsystem application;
          operatingsystem Runtime {{ runtime = BareMetal; component_failure_policy = assert_safe_state;
            loop control {{ run = application(request = 0.0); rate = 100 Hz; deadline = 10 ms; }}
          }}
          implementation {{ subsystem application {{ input request : f32; output command : f32; output command = request; }} }}
        }}'''

    def test_uef_pl_lowers_custom_target_settings(self):
        result = compile_project_text(self.source('build.backend = cmake; build.toolchain = "arm-none-eabi"; flash.backend = openocd; flash.toolchain = openocd; clock_sources.hse = 8000000; reserved_pins = ["PA13", "PA14"]; build.settings = map(platform = "stm32g4");'), uetd_root=UETD)
        self.assertTrue(result.ok, [d.to_dict() for d in result.diagnostics])
        hardware = result.hardware_configuration["target"]["hardware"]
        self.assertEqual(hardware["build"]["settings"], {"platform": "stm32g4"})
        self.assertEqual(hardware["reserved_pins"], ["PA13", "PA14"])
        schema = json.loads((ROOT / "uef-gen/src/uef_gen/schemas/project.schema.json").read_text())
        Draft202012Validator(schema).validate(result.hardware_configuration)

    def test_duplicate_and_runtime_hardware_settings_fail(self):
        for properties in ("build.backend = cmake; build.backend = other;", "clock_sources.hse = 2 + 3;"):
            with self.subTest(properties=properties):
                result = compile_project_text(self.source(properties), uetd_root=UETD)
                self.assertFalse(result.ok)

if __name__ == "__main__":
    unittest.main()
