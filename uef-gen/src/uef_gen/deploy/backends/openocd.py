"""OpenOCD backend interface scaffold.

A real implementation should use a bounded TCL/RPC client or a managed process
with captured output. Do not infer erase geometry or target configuration from
a chip-name substring.
"""

from __future__ import annotations

from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import ChipIdentity, DiscoveredProbe, FirmwareArtifact


class OpenOCDDeployment(IDeployment):
    """SWD/JTAG backend driven by a configured OpenOCD installation."""

    # Populate only from verified OpenOCD target scripts or explicit project configuration.
    CHIP_CONFIGS: dict[str, tuple[str, str]] = {}

    def __init__(self, chip_name: str, openocd_exe: str = "openocd") -> None:
        self._chip_name = chip_name
        self._exe = openocd_exe
        self._probe: DiscoveredProbe | None = None
        self._connected = False

    def _select_config(self, probe: DiscoveredProbe | None = None) -> tuple[str, str]:
        """Choose an interface and target script for verified part/probe data."""
        raise NotImplementedError(
            "TODO(openocd-config-selection): resolve exact chip data from an explicit project "
            "configuration or a reviewed part mapping; choose the interface from the selected "
            "probe type; reject unknown chips instead of guessing a family config."
        )

    def connect(self, probe: DiscoveredProbe) -> bool:
        raise NotImplementedError(
            "TODO(openocd-connect): resolve a user-configured interface/target config pair, "
            "start or connect to an isolated OpenOCD instance, bind only the selected probe, "
            "and enforce startup/command/shutdown timeouts."
        )

    def _cmd(self, command: str) -> str:
        """Send one allowlisted TCL command and return its bounded response."""
        raise NotImplementedError(
            "TODO(openocd-command): restrict commands to backend-owned operations, frame TCL "
            "responses, enforce timeouts, cap captured output, and terminate a wedged process."
        )

    def identify(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(openocd-identify): read the target ID/revision using configured read-only "
            "commands and match it to verified chip database data."
        )

    def halt(self) -> bool:
        raise NotImplementedError(
            "TODO(openocd-halt): issue halt, confirm the core state, and report timeout distinctly."
        )

    def unlock_flash(self) -> bool:
        raise NotImplementedError(
            "TODO(openocd-unlock): inspect protection state; require a separate explicit user "
            "flow for any operation that can erase user data or change option bytes."
        )

    def erase(self, start_addr: int, end_addr: int, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(openocd-erase): map the validated half-open address interval to exact flash "
            "sectors from verified target geometry, report progress, and reject out-of-range spans."
        )

    def program(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(openocd-program): select a supported image format, validate the embedded load "
            "address against artifact bounds, stream with finite timeouts, and report byte progress."
        )

    def verify(self, artifact: FirmwareArtifact, cb: ProgressCallback | None = None) -> bool:
        raise NotImplementedError(
            "TODO(openocd-verify): read back the programmed range or use a documented checksum "
            "operation, then compare with the artifact's verified image hash."
        )

    def reset(self, run: bool = True) -> bool:
        raise NotImplementedError(
            "TODO(openocd-reset): reset and optionally resume execution only after successful "
            "verification; confirm the target leaves the halted state."
        )

    def disconnect(self) -> None:
        raise NotImplementedError(
            "TODO(openocd-disconnect): close the command socket, stop only the process owned by "
            "this instance, drain logs, and release the selected probe."
        )

    def read_chip_identity(self) -> ChipIdentity:
        raise NotImplementedError(
            "TODO(openocd-read-identity): make a bounded temporary connection, read identity, "
            "and always close it without halting or modifying target state."
        )
