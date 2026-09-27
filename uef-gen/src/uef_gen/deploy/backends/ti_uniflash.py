"""TI UniFlash backend scaffold for explicitly supported TI targets."""

from __future__ import annotations

from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import ChipIdentity, DiscoveredProbe, FirmwareArtifact


class TIUniFlashDeployment(IDeployment):
    """TI UniFlash CLI backend for supported C2000 and TI wireless devices."""

    def __init__(self, chip_name: str, probe_id: str) -> None:
        self._chip_name = chip_name
        self._probe_id = probe_id
        self._cli = None

    def connect(self, probe: DiscoveredProbe) -> bool:
        raise NotImplementedError(
            "TODO(ti-uniflash-connect): resolve a supported UniFlash installation and exact probe, "
            "select a reviewed target configuration, and establish a read-only initial connection."
        )

    def identify(self) -> ChipIdentity:
        raise NotImplementedError("TODO(ti-uniflash-identify): read device identity and memory from target metadata.")

    def halt(self) -> bool:
        raise NotImplementedError("TODO(ti-uniflash-halt): halt and confirm the supported target core.")

    def unlock_flash(self) -> bool:
        raise NotImplementedError(
            "TODO(ti-uniflash-unlock): inspect device security; require explicit recovery for any destructive unlock."
        )

    def erase(self, start_addr: int, end_addr: int, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(ti-uniflash-erase): derive exact sector boundaries for the selected TI part and "
            "erase only the validated image range."
        )

    def program(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(ti-uniflash-program): validate the TI image format and address map, execute the "
            "configured CLI without shell interpolation, and capture progress/errors."
        )

    def verify(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(ti-uniflash-verify): use documented readback/checksum support and compare the "
            "artifact digest; report unsupported verification instead of passing."
        )

    def reset(self, run: bool = True) -> bool:
        raise NotImplementedError("TODO(ti-uniflash-reset): reset/resume through the supported probe API.")

    def disconnect(self) -> None:
        raise NotImplementedError("TODO(ti-uniflash-disconnect): close CLI/debug sessions and release probe resources.")

    def read_chip_identity(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(ti-uniflash-read-identity): perform read-only target identification and always close the session."
        )
