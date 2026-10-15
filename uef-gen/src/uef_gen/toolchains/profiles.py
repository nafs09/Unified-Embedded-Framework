"""Resolve UETD board build/flash records into host-local executable selections.

This module is the bridge between UETD's canonical JSON profiles and runtime
tool paths. A registry record owns package identity/version; the user's install
root and explicit executable override are machine-local and never written back
into UETD.
"""

from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
from typing import Any
from uef_gen.toolchains.bootstrap import detect_host_platform

class TargetToolProfileError(ValueError):
    """Raised when a board profile cannot be resolved to a ready UETD package."""

@dataclass(frozen=True)
class ResolvedToolSelection:
    """A board profile resolved to one UETD package and local executable paths."""

    board_id: str
    profile_kind: str
    backend_id: str
    toolchain_id: str
    package_version: str
    host_platform: str
    install_root: Path
    executables: dict[str, Path]
    settings: dict[str, Any]
    supporting_package_ids: tuple[str, ...] = ()

def resolve_board_tool_selection(
    board: dict[str, Any], catalogue: dict[str, dict[str, Any]], install_root: Path, profile_kind: str, host_platform: str | None = None
) -> ResolvedToolSelection:
    """Resolve one board build/flash profile against a UETD package catalogue.

    TODO(board-tool-profile-resolution): require `generation_available` for
    hardware builds; select `board[profile_kind]`; map its `backend` and
    `toolchain` IDs plus profile `dependencies` to existing UETD records; require
    all required records to be ready and host-compatible; select exactly one
    package version for this host;
    resolve every declared executable beneath the versioned install root; and
    return board settings without adding local paths to generated provenance.
    Reject unknown profile kinds, missing package IDs, ambiguous host packages,
    scaffold-only records, missing receipts, and executable paths that escape
    the install root. A pipeline fixture may use a separate explicit policy.
    """
    raise NotImplementedError("TODO(board-tool-profile-resolution): UETD board-to-host tool resolution is not implemented.")

def resolve_executable_map(record: dict[str, Any], package: dict[str, Any], install_root: Path) -> dict[str, Path]:
    """Return executable role paths after checking package-root containment.

    TODO(executable-map-resolution): read the selected package's UETD
    `executables` map; reject absolute/traversing relative paths; resolve
    symlinks; require each executable to remain beneath `install_root` and
    exist; verify executable permissions where the host exposes them; and keep
    role names stable (`cc`, `cxx`, `objcopy`, `size`, `flash_cli`, etc.).
    When several package records compose one SDK, report their roots by stable
    package ID so mixed-version installations can be detected.
    """
    raise NotImplementedError("TODO(executable-map-resolution): pinned package executable resolution is not implemented.")

def validate_build_flash_pair(board: dict[str, Any], build_selection: ResolvedToolSelection, flash_selection: ResolvedToolSelection) -> None:
    """Check a board's build artifact format and flash input contract agree.

    TODO(build-flash-profile-compatibility): compare declared build output
    formats, ELF/load-address support, address units, target/core identity,
    memory range, and required sidecar/map metadata. Reject a profile pair that
    can build an image the selected flasher cannot safely locate or verify.
    Keep programmer transport/interface settings from the board JSON and never
    infer them from a chip or backend ID.
    """
    raise NotImplementedError("TODO(build-flash-profile-compatibility): build/flash profile matching is not implemented.")

def current_host_platform() -> str:
    """Expose the normalized UETD host key for CLI diagnostics and plan output."""
    return detect_host_platform()
