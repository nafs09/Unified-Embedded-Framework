"""Select UETD modules from a normalized project and the live UETD manifest."""

from __future__ import annotations
from pathlib import Path
from typing import Any
from uef_gen.modules.selector import ModuleSelection, ModuleSelector
from uef_gen.uetd_adapter.registry import load_module_manifest
from uef_gen.resolver.models import ResolvedProject

class ProjectModulePlanner:
    """Translate project features into one deterministic UETD dependency closure."""

    def __init__(self, uetd_root: Path):
        self.root = uetd_root
        self.manifest = load_module_manifest(uetd_root)

    def select(self, config: dict[str, Any], resolved: ResolvedProject) -> ModuleSelection:
        requested: set[str] = set()
        architecture = str(getattr(resolved.chip.family.arch, "name", "")).casefold()
        def add_configured_module(name: str) -> None:
            normalized = name.casefold()
            if normalized == "uhal":
                requested.add("uhal/arm_cm" if "cortex" in architecture or "arm" in architecture else "uhal/x86")
                return
            if normalized == "uos":
                requested.add("uos/freertos" if resolved.rtos.casefold() == "freertos" else "uos/baremetal")
                return
            if normalized == "umid":
                requested.update(("umid/ringbuf", "umid/health"))
                return
            if normalized == "upal":
                requested.update(("upal/dma", "upal/uart", "upal/spi", "upal/i2c"))
                return
            if normalized == "uproto":
                return
            requested.add(normalized)
        for module in resolved.modules:
            add_configured_module(str(module))
        uetd_config = config.get("uetd", {})
        if isinstance(uetd_config, dict):
            for module in uetd_config.get("extra_modules", []):
                add_configured_module(str(module))
        requested.add("ucore")
        requested.add("uhal/arm_cm" if "cortex" in architecture or "arm" in architecture else "uhal/x86")
        peripheral_providers = self.manifest.get("peripheral_provides", {})
        for peripheral in resolved.peripherals:
            for module in peripheral_providers.get(peripheral.type_name.upper(), []):
                requested.add(str(module).casefold())
        protocol_providers = self.manifest.get("protocol_provides", {})
        for protocol_name in _protocol_names(config.get("protocols", [])):
            key = protocol_name.upper()
            # DSHOT600/300/150 are concrete rates for the common DSHOT module.
            if key.startswith("DSHOT"):
                key = "DSHOT"
            for module in protocol_providers.get(key, []):
                requested.add(str(module).casefold())
        middleware_modules = {"logging": "umid/log", "health_monitor": "umid/health", "sdcard_logging": "umid/fatfs"}
        middleware = config.get("middleware", {})
        if isinstance(middleware, dict):
            for name, settings in middleware.items():
                enabled = not (isinstance(settings, dict) and settings.get("enabled") is False)
                if enabled and name.casefold() in middleware_modules:
                    requested.add(middleware_modules[name.casefold()])
        if resolved.rtos.casefold() == "freertos":
            requested.add("uos/freertos")
        elif resolved.rtos.casefold() in {"", "none", "baremetal"}:
            requested.add("uos/baremetal")
        return ModuleSelector(self.root).select(tuple(sorted(requested)))

def _protocol_names(value: Any) -> tuple[str, ...]:
    """Read protocol names from either a list or the spec's named mapping."""
    if isinstance(value, list):
        names: list[str] = []
        for item in value:
            if isinstance(item, dict):
                names.append(str(item.get("type", item.get("name", ""))))
            else:
                names.append(str(item))
        return tuple(name for name in names if name)
    if isinstance(value, dict):
        names = []
        for name, settings in value.items():
            if isinstance(settings, dict):
                names.append(str(settings.get("type", name)))
            else:
                names.append(str(name))
        return tuple(names)
    return ()
