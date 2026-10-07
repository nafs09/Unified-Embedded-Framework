"""Shared value types for uef-gen build, artifact, discovery, and deployment work.

These types define the stable shape of the pipeline. Build and flash operations
remain explicit scaffolds until toolchain, linker, target, and safety contracts
are implemented for a supported board family.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum, auto
from pathlib import Path
from typing import Optional


class ToolchainFamily(Enum):
    """Toolchain groups that the build manager may resolve."""

    ARM_NONE_EABI = auto()
    TI_CGT_C2000 = auto()
    RISCV_NONE_ELF = auto()
    HOST_GCC = auto()


@dataclass(frozen=True)
class Toolchain:
    """Resolved compiler and binary utilities for one architecture family."""

    family: ToolchainFamily
    cc: str
    cxx: str = ""
    as_: str = ""
    ld: str = ""
    objcopy: str = ""
    size: str = ""
    gdb: str = ""
    sysroot: str = ""


@dataclass(frozen=True)
class BuildConfig:
    """Inputs needed to build one generated firmware project."""

    project_dir: Path
    output_dir: Path
    toolchain: Toolchain
    sources_file: Path
    includes_file: Path
    defines_file: Path
    cflags_file: Path
    linker_script: Path
    target_name: str
    flash_budget: int = 0
    ram_budget: int = 0
    source_fingerprint: str = ""


@dataclass
class BuildResult:
    """Outputs and diagnostics from a single build attempt."""

    elf: Path
    hex_: Path
    bin_: Path
    map_: Path
    size_report: dict[str, int] = field(default_factory=dict)
    flash_used_bytes: int = 0
    ram_used_bytes: int = 0
    flash_budget: int = 0
    ram_budget: int = 0
    warnings: list[str] = field(default_factory=list)
    errors: list[str] = field(default_factory=list)
    success: bool = False
    source_fingerprint: str = ""


@dataclass(frozen=True)
class FirmwareArtifact:
    """Built image plus the exact inputs that determine target compatibility."""

    elf: Path
    hex_: Path
    bin_: Path
    elf_hash: str
    chip_name: str
    chip_hash: str
    project_hash: str
    uef_source_fingerprint: str
    build_timestamp: str
    build_result: BuildResult
    flash_start_addr: int | None = None
    flash_end_addr: int | None = None

    @classmethod
    def from_build(
        cls,
        result: BuildResult,
        resolved: object,
        project_yaml_path: Path,
    ) -> "FirmwareArtifact":
        """Create provenance only after a successful, complete build."""
        raise NotImplementedError(
            "TODO(artifact-provenance): require a successful build; hash the ELF and "
            "project input; derive a stable chip-spec hash and UEF source fingerprint; "
            "record a UTC build time; and obtain the programmed address range from "
            "verified linker/target metadata rather than guessing it."
        )

    def compatible_with(self, chip_name: str) -> bool:
        """Compare artifact target identity using the project's canonical naming rule."""
        return self.chip_name.casefold() == chip_name.casefold()

    def is_stale(self, project_yaml_path: Path, resolved: object) -> bool:
        """Report whether build inputs changed since this artifact was created."""
        raise NotImplementedError(
            "TODO(artifact-staleness): re-hash the project file and canonical chip "
            "specification, compare both with the stored hashes, and also account for "
            "the UEF snapshot and generated source manifest."
        )


class ProbeType(Enum):
    """Connection mechanisms considered by target discovery."""

    STLINK_V2 = auto()
    STLINK_V3 = auto()
    JLINK = auto()
    CMSIS_DAP = auto()
    FTDI_JTAG = auto()
    UART_BOOTLOADER = auto()
    USB_DFU = auto()


@dataclass(frozen=True)
class DiscoveredProbe:
    """A probe or bootloader endpoint found on the host."""

    probe_type: ProbeType
    probe_id: str
    description: str
    transport: str
    usb_vid: int = 0
    usb_pid: int = 0


@dataclass(frozen=True)
class ChipIdentity:
    """Read-only identity and capacity reported by a connected target."""

    idcode: int
    chip_name: str
    flash_bytes: int
    sram_bytes: int
    revision: str


class DeploymentState(Enum):
    """Visible phases of the deployment lifecycle."""

    IDLE = auto()
    CONNECTING = auto()
    IDENTIFYING = auto()
    UNLOCKING = auto()
    ERASING = auto()
    PROGRAMMING = auto()
    VERIFYING = auto()
    RESETTING = auto()
    RUNNING = auto()
    FAILED = auto()
    DISCONNECTED = auto()


@dataclass(frozen=True)
class DeploymentProgress:
    """One progress event suitable for a CLI or NEXUS progress view."""

    state: DeploymentState
    pct: float
    message: str
    bytes_done: int = 0
    bytes_total: int = 0


@dataclass(frozen=True)
class DeploymentResult:
    """Final outcome and target identity from a deployment attempt."""

    success: bool
    chip_identity: Optional[ChipIdentity]
    bytes_programmed: int
    verify_passed: bool
    elapsed_s: float
    error: Optional[str]
