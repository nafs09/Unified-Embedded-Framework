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
    """Compiler/SDK runners that a board build adapter may resolve."""

    ARM_NONE_EABI = auto()
    AVR_GCC = auto()
    ESP_IDF = auto()
    ARDUINO_CLI = auto()
    TI_CGT_C2000 = auto()
    RISCV_NONE_ELF = auto()
    HOST_GCC = auto()

@dataclass(frozen=True)
class Toolchain:
    """Resolved package executables for a compiler or SDK build adapter."""

    family: ToolchainFamily
    cc: str
    cxx: str = ""
    as_: str = ""
    ld: str = ""
    objcopy: str = ""
    size: str = ""
    gdb: str = ""
    sysroot: str = ""
    package_id: str = ""
    package_version: str = ""

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
    # Build adapter and settings are resolved from UETD board records; do not
    # duplicate them in a Python target catalogue or machine-local project file.
    board_id: str = ""
    build_backend: str = ""
    toolchain_id: str = ""
    profile_settings: tuple[tuple[str, str], ...] = ()
    image_formats: tuple[str, ...] = ("elf", "hex", "bin")

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
class FirmwareImageSegment:
    """One image written at one absolute flash interval.

    ESP-IDF, bootloader/partition projects, and some multi-bank targets emit
    several independently addressed images. Addresses use a half-open byte
    interval `[start_addr, end_addr)` and each segment carries its own digest.
    """

    role: str
    path: Path
    start_addr: int
    end_addr: int
    sha256: str
    image_format: str

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
    uetd_source_fingerprint: str
    build_timestamp: str
    build_result: BuildResult
    flash_start_addr: int | None = None
    flash_end_addr: int | None = None
    # TODO(artifact-image-segments): make this the canonical address map for
    # multi-image builds; retain scalar flash_start/end only as a derived
    # envelope for single-image consumers. Validate non-overlap, per-segment
    # hashes, role uniqueness, and every interval against the verified board
    # flash/partition profile before deployment.
    segments: tuple[FirmwareImageSegment, ...] = ()

    @classmethod
    def from_build(cls, result: BuildResult, resolved: object, project_yaml_path: Path) -> "FirmwareArtifact":
        """Create provenance only after a successful, complete build."""
        raise NotImplementedError(
            "TODO(artifact-provenance): require a successful build; hash the ELF and "
            "project input; derive a stable chip-spec hash and UETD source fingerprint; "
            "record a UTC build time; create one FirmwareImageSegment per generated image "
            "using linker/SDK-provided offsets, lengths, formats, and hashes; and derive the "
            "programmed range only from verified linker/partition metadata rather than guessing."
        )

    def compatible_with(self, chip_name: str) -> bool:
        """Compare artifact target identity using the project's canonical naming rule."""
        return self.chip_name.casefold() == chip_name.casefold()

    def is_stale(self, project_yaml_path: Path, resolved: object) -> bool:
        """Report whether build inputs changed since this artifact was created."""
        raise NotImplementedError(
            "TODO(artifact-staleness): re-hash the project file and canonical chip "
            "specification, compare both with the stored hashes, and also account for "
            "the UETD snapshot and generated source manifest."
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
