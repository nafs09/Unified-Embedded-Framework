"""ST-LINK backend interface scaffold for STM32 targets."""

from __future__ import annotations

from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import ChipIdentity, DiscoveredProbe, FirmwareArtifact


class STLinkDeployment(IDeployment):
    """STM32CubeProgrammer-backed ST-LINK transport."""

    def __init__(self, probe_id: str, cli_path: str | None = None) -> None:
        self._probe_id = probe_id
        self._cli_path = cli_path
        self._connected = False

    def _find_cli(self) -> str:
        """Resolve the configured STM32CubeProgrammer executable."""
        raise NotImplementedError(
            "TODO(stlink-find-cli): honor an explicit path, then check documented installation "
            "locations and PATH; verify the executable version and return an install hint if absent."
        )

    def _run(self, arguments: list[str]) -> str:
        """Invoke CubeProgrammer using argv, bounded runtime, and captured diagnostics."""
        raise NotImplementedError(
            "TODO(stlink-run): reject caller-supplied arbitrary CLI options, use subprocess argv "
            "without a shell, select the exact probe serial, cap output, and classify exit codes."
        )

    def connect(self, probe: DiscoveredProbe) -> bool:
        raise NotImplementedError(
            "TODO(stlink-connect): verify probe type/serial, connect using a finite timeout, and "
            "confirm the device identity before any write command."
        )

    def identify(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(stlink-identify): parse the read-only device ID, revision, and available memory "
            "from CubeProgrammer and match only verified database records."
        )

    def halt(self) -> bool:
        raise NotImplementedError(
            "TODO(stlink-halt): halt and confirm target state while preserving an error reason."
        )

    def unlock_flash(self) -> bool:
        raise NotImplementedError(
            "TODO(stlink-unlock): inspect RDP/option bytes; never issue mass erase or lower RDP "
            "without a separate explicit operation and clear data-loss confirmation."
        )

    def erase(self, start_addr: int, end_addr: int, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(stlink-erase): translate the validated artifact range into exact sectors, "
            "reject full-chip erase, and report progress/cancellation safely."
        )

    def program(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(stlink-program): validate image address/format, choose the exact serial, run "
            "programming with no implicit mass erase, and parse progress and diagnostics."
        )

    def verify(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(stlink-verify): use a documented readback or checksum mode and compare with "
            "the artifact image hash; report unsupported verification explicitly."
        )

    def reset(self, run: bool = True) -> bool:
        raise NotImplementedError(
            "TODO(stlink-reset): reset and optionally run only after successful verification."
        )

    def disconnect(self) -> None:
        raise NotImplementedError(
            "TODO(stlink-disconnect): close the host connection/session and release the selected "
            "probe even if a CLI process failed."
        )

    def read_chip_identity(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(stlink-read-identity): query target ID without erase, unlock, reset, or option-byte writes."
        )
