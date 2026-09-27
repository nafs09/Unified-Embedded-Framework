"""USB DFU backend interface scaffold."""

from __future__ import annotations

from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import ChipIdentity, DiscoveredProbe, FirmwareArtifact


class DFUDeployment(IDeployment):
    """DFU transport for devices already placed in a supported bootloader mode."""

    def __init__(
        self,
        vid: int | None = None,
        pid: int | None = None,
        alt: int | None = None,
        address: str | None = None,
        dfu_util: str | None = None,
    ) -> None:
        self._vid = vid
        self._pid = pid
        self._alt = alt
        self._dfu_util = dfu_util
        self._address = address
        self._probe: DiscoveredProbe | None = None

    def connect(self, probe: DiscoveredProbe) -> bool:
        raise NotImplementedError(
            "TODO(dfu-connect): require a discovered DFU interface and explicit VID/PID/serial "
            "selection; do not detach drivers, switch boot modes, or write during discovery."
        )

    def identify(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(dfu-identify): report only identity actually exposed by the bootloader; do not "
            "invent flash/SRAM sizes from a generic VID/PID."
        )

    def halt(self) -> bool:
        raise NotImplementedError(
            "TODO(dfu-halt): document whether the chosen DFU protocol can halt application execution "
            "and return unsupported when it cannot."
        )

    def unlock_flash(self) -> bool:
        raise NotImplementedError(
            "TODO(dfu-unlock): DFU generally cannot safely unlock debug protection; fail closed and "
            "direct the user to the vendor's explicit recovery process."
        )

    def erase(self, start_addr: int, end_addr: int, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(dfu-erase): use only a device-specific DFU alternate setting and verified address "
            "range; never assume that a download command erases the intended sectors."
        )

    def program(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(dfu-program): validate image format/address, invoke dfu-util with exact device "
            "selectors, avoid temporary paths outside the artifact workspace, and report progress."
        )

    def verify(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(dfu-verify): use supported DFU upload/readback or documented checksum semantics; "
            "return unsupported rather than claiming success when readback is unavailable."
        )

    def reset(self, run: bool = True) -> bool:
        raise NotImplementedError(
            "TODO(dfu-reset): issue the supported leave/reset request and distinguish transfer "
            "completion from successful application startup."
        )

    def disconnect(self) -> None:
        raise NotImplementedError(
            "TODO(dfu-disconnect): release USB handles and any interface claim owned by this instance."
        )

    def read_chip_identity(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(dfu-read-identity): perform a read-only descriptor query and return unknown fields "
            "explicitly instead of substituting a guessed chip."
        )
