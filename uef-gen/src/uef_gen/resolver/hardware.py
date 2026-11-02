"""Compose physical target settings independently of development-board presets."""
from __future__ import annotations
from copy import deepcopy
from typing import Any
from uef_gen.diagnostics import Diagnostic, error

HARDWARE_FIELDS = frozenset({"clock_sources", "memory_regions", "reserved_pins", "module", "build", "flash", "debug_console"})

def compose_hardware_profile(target: dict[str, Any], board: dict[str, Any] | None) -> tuple[dict[str, Any], list[Diagnostic]]:
    """A board is an optional set of defaults; a project can describe any assembly.

    Readiness and on-chip resources cannot be overridden by project settings.
    Reserved preset pins are retained when project constraints are added.
    """
    profile = {key: deepcopy(value) for key, value in (board or {}).items() if key in HARDWARE_FIELDS}
    diagnostics: list[Diagnostic] = []
    supplied = target.get("hardware", {})
    if not isinstance(supplied, dict):
        return profile, [error("hardware_profile_invalid", "target.hardware must be an object", "target.hardware")]
    for key, value in supplied.items():
        location = f"target.hardware.{key}"
        if key not in HARDWARE_FIELDS:
            diagnostics.append(error("hardware_field_unknown", f"Unknown hardware setting {key!r}; chip resources and readiness belong to UETD", location))
            continue
        if key == "reserved_pins":
            if not isinstance(value, list) or any(not isinstance(pin, str) or not pin for pin in value):
                diagnostics.append(error("hardware_profile_invalid", "reserved_pins must be an array of pin names", location))
            else:
                profile[key] = list(dict.fromkeys([*profile.get(key, []), *value]))
            continue
        if not isinstance(value, dict):
            diagnostics.append(error("hardware_profile_invalid", f"{key} must be an object", location))
            continue
        previous = profile.get(key, {})
        # Changing a build/flash backend replaces its whole recipe: carrying old
        # backend settings into a different tool would be an invalid composition.
        if key in {"build", "flash"} and "backend" in value and value["backend"] != previous.get("backend"):
            previous = {}
        profile[key] = _merge(previous, value)
    for name, hz in profile.get("clock_sources", {}).items():
        if not isinstance(hz, int) or isinstance(hz, bool) or hz <= 0:
            diagnostics.append(error("hardware_clock_invalid", f"Clock source {name!r} needs a positive integer frequency", f"target.hardware.clock_sources.{name}"))
    for name, region in list(profile.get("memory_regions", {}).items()):
        if not isinstance(region, dict) or any(not isinstance(region.get(field), int) or isinstance(region.get(field), bool) for field in ("origin", "length_bytes")):
            diagnostics.append(error("hardware_memory_invalid", f"Memory region {name!r} needs integer origin and length_bytes", f"target.hardware.memory_regions.{name}"))
            del profile["memory_regions"][name]
        elif region["origin"] < 0 or region["length_bytes"] <= 0 or region["origin"] + region["length_bytes"] > (1 << 64):
            diagnostics.append(error("hardware_memory_invalid", f"Memory region {name!r} has an invalid address interval", f"target.hardware.memory_regions.{name}"))
            del profile["memory_regions"][name]
    for name in ("flash_bytes", "psram_bytes", "crystal_hz"):
        value = profile.get("module", {}).get(name)
        if value is not None and (not isinstance(value, int) or isinstance(value, bool) or value <= 0):
            diagnostics.append(error("hardware_module_invalid", f"Module {name} must be a positive integer", f"target.hardware.module.{name}"))
    for name in ("build", "flash"):
        recipe = profile.get(name)
        if recipe is not None:
            for field in ("backend", "toolchain"):
                if not isinstance(recipe.get(field), str) or not recipe[field].strip():
                    diagnostics.append(error("hardware_recipe_incomplete", f"{name} requires a {field} identifier", f"target.hardware.{name}.{field}"))
    return profile, diagnostics

def extend_memory_regions(chip_regions: dict[str, dict[str, Any]], hardware: dict[str, Any]) -> tuple[dict[str, dict[str, Any]], list[Diagnostic]]:
    """Assembly memory can add external storage; it cannot redefine on-chip RAM.

    Overlap is checked inside each declared address space, so Harvard targets
    can represent program/data memories with the same numeric addresses.
    """
    regions = deepcopy(chip_regions)
    diagnostics: list[Diagnostic] = []
    address_spaces = {region.get("address_space", "unified") for region in chip_regions.values()} or {"unified"}
    for name, region in hardware.get("memory_regions", {}).items():
        if region.get("address_space", "unified") not in address_spaces:
            diagnostics.append(error("hardware_memory_address_space_unknown", f"Memory region {name!r} names an address space not declared by the chip", f"target.hardware.memory_regions.{name}.address_space"))
            continue
        if name in regions:
            if regions[name] != region:
                diagnostics.append(error("hardware_memory_override", f"Assembly memory cannot replace chip region {name!r}", f"target.hardware.memory_regions.{name}"))
            continue
        start, end = region["origin"], region["origin"] + region["length_bytes"]
        for previous_name, previous in regions.items():
            previous_start, previous_length = previous.get("origin"), previous.get("length_bytes")
            if isinstance(previous_start, int) and isinstance(previous_length, int) and previous.get("address_space", "unified") == region.get("address_space", "unified"):
                if start < previous_start + previous_length and previous_start < end:
                    diagnostics.append(error("hardware_memory_overlap", f"Memory region {name!r} overlaps {previous_name!r}", f"target.hardware.memory_regions.{name}"))
                    break
        else:
            regions[name] = deepcopy(region)
    return regions, diagnostics

def _merge(defaults: dict[str, Any], overrides: dict[str, Any]) -> dict[str, Any]:
    result = deepcopy(defaults)
    for key, value in overrides.items():
        result[key] = _merge(result[key], value) if isinstance(value, dict) and isinstance(result.get(key), dict) else deepcopy(value)
    return result

def architecture_family(name: str) -> str:
    """Only explicit host architecture names select simulation."""
    normalized = name.casefold().replace("_", "-").replace(" ", "")
    if normalized in {"host", "x86", "x86-64", "amd64"}:
        return "HOST"
    if normalized.startswith(("cortex-m", "armcortex-m", "arm-cortex-m")):
        return "CORTEX_M"
    if normalized.startswith("avr"):
        return "AVR"
    if normalized.startswith("xtensa"):
        return "XTENSA"
    if normalized.startswith(("risc-v", "riscv", "rv32", "rv64")):
        return "RISCV"
    raise ValueError(f"Unsupported CPU architecture {name!r}; no host fallback is permitted")
