"""pyOCD backend scaffold for supported Cortex-M probes and targets."""

from __future__ import annotations

from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import ChipIdentity, DiscoveredProbe, FirmwareArtifact


class PyOCDDeployment(IDeployment):
    """Python-native SWD backend; pyOCD remains an optional host dependency."""

    def __init__(self, chip_name: str, probe_id: str) -> None:
        self._chip_name = chip_name
        self._probe_id = probe_id
        self._session = None

    def connect(self, probe: DiscoveredProbe) -> bool:
        raise NotImplementedError(
            "TODO(pyocd-connect): verify the selected CMSIS-DAP probe serial, create a pyOCD "
            "session with finite timeouts, select an explicit target definition, and acquire "
            "resources without modifying target flash."
        )

    def identify(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(pyocd-identify): read the core/device ID through the session and resolve it "
            "against verified chip metadata; do not trust a requested chip name by itself."
        )

    def halt(self) -> bool:
        raise NotImplementedError("TODO(pyocd-halt): halt the core and confirm it stopped before writes.")

    def unlock_flash(self) -> bool:
        raise NotImplementedError(
            "TODO(pyocd-unlock): inspect target protection state and fail closed when unlocking "
            "would require destructive mass erase or security changes."
        )

    def erase(self, start_addr: int, end_addr: int, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(pyocd-erase): map the validated address interval to target flash pages, reject "
            "out-of-range pages, and report bounded progress and cancellation."
        )

    def program(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(pyocd-program): verify artifact/hash and load range, program only the selected "
            "target region, and report transport errors without claiming partial success."
        )

    def verify(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(pyocd-verify): read back the programmed region or use a documented target hash "
            "operation and compare it with the stored artifact digest."
        )

    def reset(self, run: bool = True) -> bool:
        raise NotImplementedError("TODO(pyocd-reset): reset/resume only after successful verification.")

    def disconnect(self) -> None:
        raise NotImplementedError(
            "TODO(pyocd-disconnect): close the session and release probe resources in all error paths."
        )

    def read_chip_identity(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(pyocd-read-identity): use a temporary read-only session and close it even when "
            "identity lookup fails."
        )
