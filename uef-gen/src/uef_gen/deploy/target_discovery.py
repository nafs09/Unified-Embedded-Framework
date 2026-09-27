"""Host-side discovery of debug probes and bootloader devices.

USB enumeration is optional and must not be required for generating or building
firmware. A discovered VID/PID is a transport clue, not proof of MCU identity.
"""

from __future__ import annotations

from uef_gen.deploy.models import (
    ChipIdentity,
    DiscoveredProbe,
    FirmwareArtifact,
    ProbeType,
)


class TargetDiscovery:
    """Enumerate available transports and match them to an artifact target."""

    KNOWN_PROBES: dict[tuple[int, int], ProbeType] = {
        (0x0483, 0x3748): ProbeType.STLINK_V2,
        (0x0483, 0x374B): ProbeType.STLINK_V2,
        (0x0483, 0x374F): ProbeType.STLINK_V3,
        (0x0483, 0x3753): ProbeType.STLINK_V3,
        (0x1366, 0x0101): ProbeType.JLINK,
        (0x1366, 0x0105): ProbeType.JLINK,
        (0x0D28, 0x0204): ProbeType.CMSIS_DAP,
        (0x1209, 0x000C): ProbeType.CMSIS_DAP,
    }

    def enumerate_probes(self) -> list[DiscoveredProbe]:
        """Return known debug probes plus explicitly supported bootloaders."""
        raise NotImplementedError(
            "TODO(target-enumeration): make pyusb and serial enumeration optional; handle "
            "permission/driver errors without breaking generation; normalize serial numbers "
            "and USB paths; and avoid claiming identity from VID/PID alone."
        )

    def _enumerate_uart_bootloaders(self) -> list[DiscoveredProbe]:
        """Find UART bootloaders only when a protocol-specific detector is configured."""
        raise NotImplementedError(
            "TODO(uart-bootloader-discovery): enumerate serial ports without opening them "
            "destructively, identify bootloader protocols through safe read-only probes, and "
            "require the user to choose ambiguous devices."
        )

    def _enumerate_dfu(self) -> list[DiscoveredProbe]:
        """Find standards-compliant DFU interfaces without initiating a transfer."""
        raise NotImplementedError(
            "TODO(dfu-discovery): inspect USB interface descriptors and alternate settings, "
            "record bus/port identity, and do not detach kernel drivers or change target mode."
        )

    def identify_target(self, probe: DiscoveredProbe) -> ChipIdentity:
        """Read silicon identity through the selected backend."""
        raise NotImplementedError(
            "TODO(target-identification): create a read-only backend, connect with bounded "
            "timeouts, read a stable device ID/revision, map it through verified chip data, "
            "and always release the connection."
        )

    def find_compatible(
        self,
        probes: list[DiscoveredProbe],
        artifact: FirmwareArtifact,
    ) -> DiscoveredProbe | None:
        """Return a probe only when its verified chip name exactly matches the artifact."""
        for probe in probes:
            try:
                identity = self.identify_target(probe)
            except Exception:
                # A disconnected/busy probe should not prevent examining other candidates.
                continue
            if identity.chip_name.casefold() == artifact.chip_name.casefold():
                return probe
        return None
