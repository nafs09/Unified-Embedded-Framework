"""Safety-gated deployment state machine."""

from __future__ import annotations

import time

from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import (
    DeploymentProgress,
    DeploymentResult,
    DeploymentState,
    DiscoveredProbe,
    FirmwareArtifact,
)


class DeploymentOrchestrator:
    """Coordinate a verified artifact and a user-selected physical target."""

    def deploy(
        self,
        artifact: FirmwareArtifact,
        probe: DiscoveredProbe,
        backend: IDeployment,
        verify: bool = True,
        cb: ProgressCallback | None = None,
    ) -> DeploymentResult:
        """Run deployment phases and guarantee disconnect after any attempted connection."""
        started = time.monotonic()
        identity = None
        bytes_programmed = 0
        verify_passed = False
        connection_attempted = False

        def report(state: DeploymentState, pct: float, message: str) -> None:
            if cb is not None:
                try:
                    cb(DeploymentProgress(state, pct, message, bytes_programmed, 0))
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
                raise RuntimeError(
                    f"artifact targets {artifact.chip_name}, connected target is {identity.chip_name}"
                )
            if artifact.flash_start_addr is None or artifact.flash_end_addr is None:
                raise RuntimeError(
                    "artifact has no verified flash address range; refusing to erase or program"
                )
            if artifact.flash_end_addr <= artifact.flash_start_addr:
                raise RuntimeError("artifact flash range is invalid")
            # This sequence remains unavailable until each backend has reviewed timeout,
            # protection, erase geometry, and cancellation behavior for its supported target.
            raise NotImplementedError(
                "TODO(deployment-sequence): confirm the selected probe/target with the user; " 
                "halt; inspect protection without changing destructive option bytes; validate " 
                "the absolute artifact range against verified target flash base/length and sector "
                "geometry (capacity alone is not an address); erase exactly the required " 
                "sectors; program; verify by readback/hash; report byte progress; reset only after "
                "successful verification; and preserve a recoverable diagnostic on every failure."
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
                            cb(
                                DeploymentProgress(
                                    DeploymentState.FAILED,
                                    0.0,
                                    f"Target cleanup failed: {cleanup_error}",
                                    bytes_programmed,
                                    0,
                                )
                            )
                        except Exception:
                            pass
                finally:
                    if cb is not None:
                        try:
                            cb(
                                DeploymentProgress(
                                    DeploymentState.DISCONNECTED,
                                    100.0 if verify_passed else 0.0,
                                    "Disconnected from target.",
                                    bytes_programmed,
                                    0,
                                )
                            )
                        except Exception:
                            # UI callbacks must not mask the backend result during cleanup.
                            pass
