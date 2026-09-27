"""Concrete deployment backend scaffolds."""

from uef_gen.deploy.backends.dfu import DFUDeployment
from uef_gen.deploy.backends.jlink import JLinkDeployment
from uef_gen.deploy.backends.openocd import OpenOCDDeployment
from uef_gen.deploy.backends.pyocd import PyOCDDeployment
from uef_gen.deploy.backends.stlink import STLinkDeployment
from uef_gen.deploy.backends.ti_uniflash import TIUniFlashDeployment
from uef_gen.deploy.backends.uart_boot import UARTBootDeployment

__all__ = [
    "DFUDeployment",
    "JLinkDeployment",
    "OpenOCDDeployment",
    "PyOCDDeployment",
    "STLinkDeployment",
    "TIUniFlashDeployment",
    "UARTBootDeployment",
]
