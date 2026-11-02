"""Select UETD modules from a normalized project and the live UETD manifest."""

from __future__ import annotations
from pathlib import Path
from typing import Any
from uef_gen.modules.selector import ModuleSelection, ModuleSelector, ModuleSelectionError
from uef_gen.uetd_adapter.registry import load_module_manifest
from uef_gen.resolver.models import ResolvedProject
from uef_gen.resolver.hardware import architecture_family

class ProjectModulePlanner:
    """Translate project features into one deterministic UETD dependency closure."""

    def __init__(self, uetd_root: Path):
        self.root = uetd_root
        self.manifest = load_module_manifest(uetd_root)

    def select(self, config: dict[str, Any], resolved: ResolvedProject) -> ModuleSelection:
        requested: set[str] = set()
        uhal_module = select_uhal_module(resolved, self.manifest.get("modules", {}))
        def add_configured_module(name: str) -> None:
            normalized = name.casefold()
            if normalized == "uhal":
                requested.add(uhal_module)
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
        requested.add(uhal_module)
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

def select_uhal_module(resolved: ResolvedProject, module_records: dict[str, Any] | None = None) -> str:
    """Resolve architecture and SDK separately; legacy IDs remain stable aliases."""
    try:
        architecture = architecture_family(str(resolved.chip.family.arch.name))
    except ValueError as exc:
        raise ModuleSelectionError(str(exc)) from exc
    build = getattr(resolved, "hardware_profile", {}).get("build", {})
    backend = str(build.get("backend", "")).casefold().replace("-", "_")
    framework = str(build.get("framework", "")).casefold().replace("-", "_")
    # Exact-part records can bind new implementations without a chip-name branch
    # in Python. Projects select a registered platform, never arbitrary source
    # paths or an implementation belonging to another CPU/SDK.
    binding = getattr(resolved.chip, "resources", {}).get("uhal")
    if binding is not None:
        if not isinstance(binding, dict) or not isinstance(binding.get("platforms"), dict):
            raise ModuleSelectionError("The chip's UHAL platform binding is invalid")
        settings = build.get("settings", {})
        platform = settings.get("platform", binding.get("default_platform")) if isinstance(settings, dict) else None
        if not isinstance(platform, str):
            raise ModuleSelectionError("Select an approved UHAL platform in target.hardware.build.settings.platform")
        port = binding["platforms"].get(platform)
        if not isinstance(port, dict):
            raise ModuleSelectionError(f"UHAL platform {platform!r} is not registered for this chip")
        if port.get("architecture") != architecture:
            raise ModuleSelectionError(f"UHAL platform {platform!r} is incompatible with {architecture}")
        expected_backend = str(port.get("build_backend", "")).casefold().replace("-", "_")
        if not expected_backend or expected_backend != backend:
            raise ModuleSelectionError(f"UHAL platform {platform!r} requires build backend {expected_backend!r}, not {backend!r}")
        expected_framework = str(port.get("framework", "")).casefold().replace("-", "_")
        if framework and expected_framework != framework:
            raise ModuleSelectionError(f"UHAL platform {platform!r} is incompatible with framework {framework!r}")
        module = port.get("module")
        record = (module_records or {}).get(module) if isinstance(module, str) else None
        if not isinstance(record, dict):
            raise ModuleSelectionError(f"UHAL platform {platform!r} references an unknown module {module!r}")
        declared = record.get("architecture")
        compatible = declared == architecture or isinstance(declared, list) and architecture in declared
        if declared == "XTENSA_OR_RISCV":
            compatible = architecture in {"XTENSA", "RISCV"}
        if not compatible:
            raise ModuleSelectionError(f"UHAL module {module!r} does not declare compatibility with {architecture}")
        return module
    if backend == "esp_idf" or framework == "esp_idf":
        if architecture not in {"XTENSA", "RISCV"}:
            raise ModuleSelectionError("ESP-IDF requires an ESP Xtensa/RISC-V chip/platform, not this CPU architecture")
        if not str(resolved.chip.family.name).casefold().startswith("esp"):
            raise ModuleSelectionError("ESP-IDF requires an Espressif SoC; sharing a RISC-V ISA is insufficient")
        return "uhal/esp_idf"
    if architecture == "HOST":
        return "uhal/x86"
    if architecture == "CORTEX_M":
        return "uhal/arm_cm"
    if architecture == "AVR":
        return "uhal/avr"
    raise ModuleSelectionError(f"No complete UHAL platform selected for {architecture}; supply an exact SDK/SoC integration")

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
