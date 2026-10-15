"""Concrete deployment backend scaffolds."""

from uef_gen.deploy.backends.avrdude import AvrdudeDeployment
from uef_gen.deploy.backends.dfu import DFUDeployment
from uef_gen.deploy.backends.esptool import EsptoolDeployment
from uef_gen.deploy.backends.jlink import JLinkDeployment
from uef_gen.deploy.backends.openocd import OpenOCDDeployment
from uef_gen.deploy.backends.pyocd import PyOCDDeployment
from uef_gen.deploy.backends.stlink import STLinkDeployment
from uef_gen.deploy.backends.ti_uniflash import TIUniFlashDeployment
from uef_gen.deploy.backends.uart_boot import UARTBootDeployment

__all__ = [
    "AvrdudeDeployment",
    "DFUDeployment",
    "EsptoolDeployment",
    "JLinkDeployment",
    "OpenOCDDeployment",
    "PyOCDDeployment",
    "STLinkDeployment",
    "TIUniFlashDeployment",
    "UARTBootDeployment",
]
