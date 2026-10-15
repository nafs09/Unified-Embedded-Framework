"""Choose a deployment backend from the selected transport and user preference."""

from __future__ import annotations
from uef_gen.deploy.backends import (
    AvrdudeDeployment,
    DFUDeployment,
    EsptoolDeployment,
    JLinkDeployment,
    OpenOCDDeployment,
    PyOCDDeployment,
    STLinkDeployment,
    TIUniFlashDeployment,
    UARTBootDeployment,
)
from uef_gen.deploy.ideployment import IDeployment
from uef_gen.deploy.models import DiscoveredProbe, ProbeType

class DeploymentBackendFactory:
    """Construct a backend object; construction never connects or writes to hardware."""

    @staticmethod
    def create(probe: DiscoveredProbe, chip_name: str = "", prefer: str = "auto", flash_profile: dict[str, object] | None = None) -> IDeployment:
        preference = prefer.casefold()
        supported = {"auto", "openocd", "stlink", "pyocd", "jlink", "dfu", "esptool", "avrdude", "uart", "ti_uniflash"}
        if preference not in supported:
            raise ValueError(f"unsupported deployment backend preference: {prefer!r}")
        if preference == "auto":
            selected = flash_profile.get("backend") if isinstance(flash_profile, dict) else None
            if not isinstance(selected, str) or not selected.strip():
                raise ValueError("automatic backend selection requires the selected UETD board flash profile")
            preference = selected.strip().casefold()
            if preference not in supported - {"auto"}:
                raise ValueError(f"UETD board flash profile selects unsupported backend: {selected!r}")
        if preference == "dfu":
            if probe.probe_type != ProbeType.USB_DFU:
                raise ValueError("DFU backend requires a discovered USB_DFU probe")
            return DFUDeployment()
        if preference == "esptool":
            if probe.probe_type != ProbeType.UART_BOOTLOADER:
                raise ValueError("esptool backend requires a discovered UART bootloader")
            return EsptoolDeployment(chip_name, probe.probe_id)
        if preference == "avrdude":
            if probe.probe_type != ProbeType.UART_BOOTLOADER:
                raise ValueError("avrdude backend requires a discovered UART bootloader/programmer")
            return AvrdudeDeployment(chip_name, probe.probe_id)
        if preference == "stlink":
            if probe.probe_type not in {ProbeType.STLINK_V2, ProbeType.STLINK_V3}:
                raise ValueError("ST-LINK backend requires a discovered ST-LINK probe")
            return STLinkDeployment(probe.probe_id)
        if preference == "pyocd":
            if probe.probe_type != ProbeType.CMSIS_DAP:
                raise ValueError("pyOCD backend requires a CMSIS-DAP probe")
            return PyOCDDeployment(chip_name, probe.probe_id)
        if preference == "jlink":
            if probe.probe_type != ProbeType.JLINK:
                raise ValueError("J-Link backend requires a discovered J-Link probe")
            return JLinkDeployment(chip_name, probe.probe_id)
        if preference == "uart":
            if probe.probe_type != ProbeType.UART_BOOTLOADER:
                raise ValueError("UART boot backend requires a discovered UART bootloader")
            return UARTBootDeployment(chip_name, probe.probe_id)
        if preference == "ti_uniflash":
            return TIUniFlashDeployment(chip_name, probe.probe_id)
        if preference == "openocd":
            return OpenOCDDeployment(chip_name)
        raise ValueError(f"no backend can handle probe type {probe.probe_type.name}")
