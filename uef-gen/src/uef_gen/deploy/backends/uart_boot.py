"""UART bootloader backend scaffold for explicitly supported chip protocols."""

from __future__ import annotations

from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import ChipIdentity, DiscoveredProbe, FirmwareArtifact


class UARTBootDeployment(IDeployment):
    """Serial bootloader transport; protocol implementation is target-specific."""

    def __init__(self, chip_name: str, port: str) -> None:
        self._chip_name = chip_name
        self._port = port
        self._serial = None

    def connect(self, probe: DiscoveredProbe) -> bool:
        raise NotImplementedError(
            "TODO(uart-connect): validate the selected port and baud/protocol settings, open it "
            "with a timeout, and synchronize without issuing erase/program commands."
        )

    def identify(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(uart-identify): use the bootloader's documented read-only ID command and map it "
            "to verified chip data; do not infer identity from the COM port."
        )

    def halt(self) -> bool:
        raise NotImplementedError(
            "TODO(uart-halt): document bootloader state semantics and return unsupported if the "
            "protocol cannot establish a safe halted state."
        )

    def unlock_flash(self) -> bool:
        raise NotImplementedError(
            "TODO(uart-unlock): expose protection state and never issue mass-erase unlock implicitly."
        )

    def erase(self, start_addr: int, end_addr: int, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(uart-erase): validate address and sector alignment against chip metadata, use "
            "the target protocol's bounded erase command, and report progress/errors."
        )

    def program(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(uart-program): validate image format/range, packetize with protocol checksums, "
            "handle retries within a deadline, and report bytes acknowledged by the target."
        )

    def verify(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(uart-verify): use target readback or checksum commands and compare with artifact "
            "content; fail clearly if the protocol provides no verification."
        )

    def reset(self, run: bool = True) -> bool:
        raise NotImplementedError(
            "TODO(uart-reset): Require successful image verification, send the bootloader's documented "
            "reset/run command, wait for its acknowledgement or reconnect state, and honor run without "
            "issuing an undocumented command."
        )

    def disconnect(self) -> None:
        raise NotImplementedError(
            "TODO(uart-disconnect): Close the serial handle owned by this instance, cancel pending I/O, "
            "release the selected port, and make cleanup safe after partial connection."
        )

    def read_chip_identity(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(uart-read-identity): open a temporary read-only serial session and close it on all paths."
        )
