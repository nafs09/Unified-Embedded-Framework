"""SEGGER J-Link backend scaffold."""

from __future__ import annotations

from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import ChipIdentity, DiscoveredProbe, FirmwareArtifact


class JLinkDeployment(IDeployment):
    """SWD/JTAG backend using a configured J-Link SDK or command-line tool."""

    def __init__(self, chip_name: str, probe_id: str) -> None:
        self._chip_name = chip_name
        self._probe_id = probe_id
        self._session = None

    def connect(self, probe: DiscoveredProbe) -> bool:
        raise NotImplementedError(
            "TODO(jlink-connect): resolve the configured J-Link tool/SDK, select the exact serial, "
            "and open a bounded session without selecting a guessed device profile."
        )

    def identify(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(jlink-identify): read device ID and revision, then match verified target data."
        )

    def halt(self) -> bool:
        raise NotImplementedError("TODO(jlink-halt): halt and confirm the core state.")

    def unlock_flash(self) -> bool:
        raise NotImplementedError(
            "TODO(jlink-unlock): inspect security/protection and reject implicit mass erase or "
            "irreversible security changes."
        )

    def erase(self, start_addr: int, end_addr: int, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(jlink-erase): derive exact erase sectors from verified target geometry and "
            "erase only the validated half-open artifact range."
        )

    def program(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(jlink-program): verify artifact identity and address metadata, write with a "
            "bounded session, and publish meaningful byte progress."
        )

    def verify(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(jlink-verify): compare readback/checksum with artifact content and distinguish "
            "unsupported verification from a successful match."
        )

    def reset(self, run: bool = True) -> bool:
        raise NotImplementedError("TODO(jlink-reset): reset and resume only after verified programming.")

    def disconnect(self) -> None:
        raise NotImplementedError("TODO(jlink-disconnect): close SDK/process handles and release probe.")

    def read_chip_identity(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(jlink-read-identity): perform a read-only identification session and guarantee cleanup."
        )
