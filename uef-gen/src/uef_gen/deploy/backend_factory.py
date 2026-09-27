"""Choose a deployment backend from the selected transport and user preference."""

from __future__ import annotations

from uef_gen.deploy.backends import (
    DFUDeployment,
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
    def create(
        probe: DiscoveredProbe,
        chip_name: str = "",
        prefer: str = "auto",
    ) -> IDeployment:
        preference = prefer.casefold()
        if preference not in {
            "auto",
            "openocd",
            "stlink",
            "pyocd",
            "jlink",
            "dfu",
            "uart",
            "ti_uniflash",
        }:
            raise ValueError(f"unsupported deployment backend preference: {prefer!r}")

        if preference == "dfu" or (
            preference == "auto" and probe.probe_type == ProbeType.USB_DFU
        ):
            if probe.probe_type != ProbeType.USB_DFU:
                raise ValueError("DFU backend requires a discovered USB_DFU probe")
            return DFUDeployment()

        stlink_probe = probe.probe_type in {ProbeType.STLINK_V2, ProbeType.STLINK_V3}
        if preference == "stlink" or (
            preference == "auto"
            and stlink_probe
            and chip_name.upper().startswith("STM32")
        ):
            if probe.probe_type not in {ProbeType.STLINK_V2, ProbeType.STLINK_V3}:
                raise ValueError("ST-LINK backend requires a discovered ST-LINK probe")
            if not chip_name.upper().startswith("STM32"):
                raise ValueError("ST-LINK backend is restricted to verified STM32 target data")
            return STLinkDeployment(probe.probe_id)

        if preference == "pyocd" or (
            preference == "auto" and probe.probe_type == ProbeType.CMSIS_DAP
        ):
            if probe.probe_type != ProbeType.CMSIS_DAP:
                raise ValueError("pyOCD backend requires a CMSIS-DAP probe")
            return PyOCDDeployment(chip_name, probe.probe_id)

        if preference == "jlink" or (
            preference == "auto" and probe.probe_type == ProbeType.JLINK
        ):
            if probe.probe_type != ProbeType.JLINK:
                raise ValueError("J-Link backend requires a discovered J-Link probe")
            return JLinkDeployment(chip_name, probe.probe_id)

        if preference == "uart" or (
            preference == "auto" and probe.probe_type == ProbeType.UART_BOOTLOADER
        ):
            if probe.probe_type != ProbeType.UART_BOOTLOADER:
                raise ValueError("UART boot backend requires a discovered UART bootloader")
            return UARTBootDeployment(chip_name, probe.probe_id)

        if preference == "ti_uniflash":
            return TIUniFlashDeployment(chip_name, probe.probe_id)

        if preference == "openocd" or preference == "auto":
            return OpenOCDDeployment(chip_name)

        raise ValueError(f"no backend can handle probe type {probe.probe_type.name}")
