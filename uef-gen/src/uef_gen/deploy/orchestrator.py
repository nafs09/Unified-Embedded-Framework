"""Safety-gated deployment state machine."""

from __future__ import annotations
import time
from collections.abc import Callable
from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import ChipIdentity, DeploymentProgress, DeploymentResult, DeploymentState, DiscoveredProbe, FirmwareArtifact

_PhaseReporter = Callable[[DeploymentState, float, str, int, int], None]

class DeploymentOrchestrator:
    """Coordinate a verified artifact and a user-selected physical target."""

    def _confirm_target(self, artifact: FirmwareArtifact, probe: DiscoveredProbe, identity: ChipIdentity) -> None:
        """Require an explicit confirmation bound to the exact write operation.

        TODO(deploy-confirmation): display project name, artifact hash, exact
        chip identity, probe type/serial, and half-open flash address range;
        require an affirmative interactive response or an explicit
        non-interactive authorization token; expire the confirmation if any
        selected input changes. Confirmation must happen before halt/unlock/
        erase and must never authorize mass erase or protection changes.
        """
        raise NotImplementedError("TODO(deploy-confirmation): explicit target confirmation is not implemented.")

    def _halt_and_inspect(self, backend: IDeployment) -> None:
        """Halt safely and inspect protection without modifying option bytes.

        TODO(deploy-halt-protection): call `halt`, confirm the core state with a
        bounded poll, inspect the exact part's protection state, and refuse if
        protection blocks the requested range. Do not reinterpret an unlock
        operation as mass erase, downgrade security, or write option bytes.
        """
        raise NotImplementedError("TODO(deploy-halt-protection): halt/protection inspection is not implemented.")

    def _erase_verified_range(self, backend: IDeployment, artifact: FirmwareArtifact, report: _PhaseReporter) -> None:
        """Translate the artifact interval to exact erase units and erase them.

        TODO(deploy-erase): load the selected UETD chip's verified memory and
        erase geometry; check overflow, alignment, bootloader/secure/reserved
        regions, and image bounds; erase only intersecting pages/sectors; and
        emit progress with a recoverable failure phase.
        """
        raise NotImplementedError("TODO(deploy-erase): geometry-checked erase is not implemented.")

    def _program_artifact(self, backend: IDeployment, artifact: FirmwareArtifact, report: _PhaseReporter) -> int:
        """Program only the artifact whose hash/range was validated before connect.

        TODO(deploy-program): verify the image hash immediately before use;
        select a format declared by the board flash profile; reject image load
        addresses outside the confirmed range; stream bounded chunks; call
        `report(state, percent, message, bytes_done, bytes_total)` after each
        acknowledged chunk; and return the final byte count.
        """
        raise NotImplementedError("TODO(deploy-program): guarded image programming is not implemented.")

    def _verify_artifact(self, backend: IDeployment, artifact: FirmwareArtifact, report: _PhaseReporter) -> bool:
        """Compare programmed bytes against the recorded artifact image hash.

        TODO(deploy-verify): prefer readback of the exact image span; use a
        target checksum command only when documented and equivalent; report
        progress through the phase reporter; identify a mismatched address/range
        without dumping sensitive memory; and never report success on a
        transport's unverified completion response alone.
        """
        raise NotImplementedError("TODO(deploy-verify): artifact readback/hash verification is not implemented.")

    def _reset_after_verification(self, backend: IDeployment, verify_passed: bool) -> None:
        """Reset/run only when the selected verification policy passed.

        TODO(deploy-reset): require `verify_passed` when verification is enabled;
        execute the board profile's reset/run behavior with a bounded timeout;
        observe a documented ready/console signal when available; and preserve
        the programmed image and diagnostic if the target fails to restart.
        """
        raise NotImplementedError("TODO(deploy-reset): post-verification reset is not implemented.")

    def _run_write_sequence(
        self, artifact: FirmwareArtifact, probe: DiscoveredProbe, identity: ChipIdentity, backend: IDeployment, verify: bool, report: _PhaseReporter
    ) -> tuple[int, bool]:
        """Run destructive phases in order after all preflight checks pass.

        The order is part of the safety contract. Keep each phase independently
        testable and report its state before the backend call; on any failure,
        stop immediately and let `deploy()` perform unconditional disconnect.
        """
        self._confirm_target(artifact, probe, identity)
        self._halt_and_inspect(backend)
        self._erase_verified_range(backend, artifact, report)
        bytes_programmed = self._program_artifact(backend, artifact, report)
        verify_passed = self._verify_artifact(backend, artifact, report) if verify else False
        self._reset_after_verification(backend, verify_passed if verify else True)
        return bytes_programmed, verify_passed

    def deploy(
        self, artifact: FirmwareArtifact, probe: DiscoveredProbe, backend: IDeployment, verify: bool = True, cb: ProgressCallback | None = None
    ) -> DeploymentResult:
        """Run deployment phases and guarantee disconnect after any attempted connection."""
        started = time.monotonic()
        identity = None
        bytes_programmed = 0
        verify_passed = False
        connection_attempted = False
        def report(state: DeploymentState, pct: float, message: str, bytes_done: int | None = None, bytes_total: int = 0) -> None:
            nonlocal bytes_programmed
            if bytes_done is not None:
                bytes_programmed = bytes_done
            if cb is not None:
                try:
                    cb(DeploymentProgress(state, pct, message, bytes_programmed, bytes_total))
                except Exception:
                    # A UI observer cannot be allowed to corrupt deployment state.
                    pass
        try:
            report(DeploymentState.CONNECTING, 0.0, "Connecting to selected target.")
            # A backend may allocate a handle before reporting a connection failure.
            connection_attempted = True
            if not backend.connect(probe):
                raise RuntimeError("deployment backend could not connect")
            report(DeploymentState.IDENTIFYING, 5.0, "Checking target identity.")
            identity = backend.identify()
            if not artifact.compatible_with(identity.chip_name):
                raise RuntimeError(f"artifact targets {artifact.chip_name}, connected target is {identity.chip_name}")
            if artifact.flash_start_addr is None or artifact.flash_end_addr is None:
                raise RuntimeError("artifact has no verified flash address range; refusing to erase or program")
            if artifact.flash_end_addr <= artifact.flash_start_addr:
                raise RuntimeError("artifact flash range is invalid")
            # The exact identity and range checks above are read-only. All
            # write-capable phases remain explicit method scaffolds below.
            bytes_programmed, verify_passed = self._run_write_sequence(artifact, probe, identity, backend, verify, report)
            return DeploymentResult(
                success=True, chip_identity=identity, bytes_programmed=bytes_programmed, verify_passed=verify_passed, elapsed_s=time.monotonic() - started
            )
        except Exception as exc:
            report(DeploymentState.FAILED, 0.0, str(exc))
            return DeploymentResult(
                success=False,
                chip_identity=identity,
                bytes_programmed=bytes_programmed,
                verify_passed=verify_passed,
                elapsed_s=time.monotonic() - started,
                error=str(exc),
            )
        finally:
            if connection_attempted:
                try:
                    backend.disconnect()
                except Exception as cleanup_error:
                    # Preserve the deployment failure while still exposing cleanup trouble.
                    if cb is not None:
                        try:
                            cb(DeploymentProgress(DeploymentState.FAILED, 0.0, f"Target cleanup failed: {cleanup_error}", bytes_programmed, 0))
                        except Exception:
                            pass
                finally:
                    if cb is not None:
                        try:
                            cb(
                                DeploymentProgress(
                                    DeploymentState.DISCONNECTED, 100.0 if verify_passed else 0.0, "Disconnected from target.", bytes_programmed, 0
                                )
                            )
                        except Exception:
                            # UI callbacks must not mask the backend result during cleanup.
                            pass
