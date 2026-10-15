"""Locate and validate the separately maintained UETD source checkout."""

from __future__ import annotations
import json
import os
import re
from pathlib import Path
from uef_gen import __version__

class UETDNotFoundError(FileNotFoundError):
    """Raised when no candidate directory contains the UETD release contract."""

class UETDVersionIncompatible(ValueError):
    """Raised when the located framework is older than this generator supports."""


_VERSION_PATTERN = re.compile(r"^(\d+)\.(\d+)\.(\d+)(?:[-+][0-9A-Za-z.-]+)?$")

def _version_tuple(value: str, field: str) -> tuple[int, int, int]:
    """Parse the numeric compatibility portion of a semantic version."""
    match = _VERSION_PATTERN.fullmatch(value.strip())
    if not match:
        raise UETDVersionIncompatible(f"Invalid {field} semantic version: {value!r}")
    return tuple(int(part) for part in match.groups())

class UETDLocator:
    """Resolve UETD_PATH, project configuration, then documented relative paths."""

    REQUIRED_VERSION = "1.1.0"

    def __init__(self, project_directory: Path | None = None):
        self.project_directory = (project_directory or Path.cwd()).expanduser().resolve()
        self.warnings: list[str] = []
        self.searched: tuple[Path, ...] = ()

    def locate(self, config: dict | None = None) -> Path:
        """Return the first UETD root with its API-declared registries and a compatible version."""
        config = config or {}
        uetd_config = config.get("uetd", {})
        if not isinstance(uetd_config, dict):
            uetd_config = {}
        raw_candidates: list[Path] = []
        if env_path := os.environ.get("UETD_PATH", "").strip():
            raw_candidates.append(Path(env_path).expanduser())
        configured_path = uetd_config.get("path")
        if isinstance(configured_path, str) and configured_path.strip():
            raw_candidates.append(Path(configured_path).expanduser())
        raw_candidates.extend((Path("../UETD"), Path("../../UETD"), Path("./UETD")))
        # Projects often live several directories below the uef-gen checkout
        # (for example examples/showcases/...). Walk their ancestors so the
        # adjacent UETD checkout remains discoverable without requiring every
        # example or user project to duplicate UETD_PATH configuration.
        raw_candidates.extend(parent / "UETD" for parent in self.project_directory.parents)
        candidates: list[Path] = []
        seen: set[str] = set()
        for candidate in raw_candidates:
            rooted = candidate if candidate.is_absolute() else self.project_directory / candidate
            normalized = rooted.resolve(strict=False)
            key = os.path.normcase(str(normalized))
            if key not in seen:
                candidates.append(normalized)
                seen.add(key)
        self.searched = tuple(candidates)
        for root in candidates:
            api_path = root / "api.json"
            if not api_path.is_file():
                continue
            try:
                api = json.loads(api_path.read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError):
                continue
            if not isinstance(api, dict):
                continue
            # UETD's API contract is the sole locator for its live catalogues.
            try:
                registry_paths = [
                    api["module_registry"],
                    api["type_registry"],
                    api["type_registry_schema"],
                    api["unit_registry"],
                    api["unit_registry_schema"],
                    api["umath_api"]["function_registry"],
                    api["umath_api"]["function_registry_schema"],
                    api["future_catalogue"],
                    api["uhal_api"]["target_profiles"]["target_family_catalogue"]["path"],
                    api["uhal_api"]["chip_catalogue"]["path"],
                    api["uhal_api"]["target_profiles"]["target_family_catalogue"]["registry_schema_path"],
                    api["uhal_api"]["chip_catalogue"]["schema_path"],
                    api["uhal_api"]["chip_catalogue"]["registry_schema_path"],
                    api["uhal_api"]["development_boards"]["path"],
                    api["uhal_api"]["development_boards"]["schema_path"],
                    api["uhal_api"]["development_boards"]["registry_schema_path"],
                    api["uhal_api"]["toolchain_catalogue"]["path"],
                    api["uhal_api"]["toolchain_catalogue"]["schema_path"],
                    api["uhal_api"]["toolchain_catalogue"]["registry_schema_path"],
                ]
            except (KeyError, TypeError):
                continue
            resolved_paths = []
            for relative in registry_paths:
                if not isinstance(relative, str) or not relative.strip():
                    break
                declared = Path(relative.replace(chr(92), "/"))
                if declared.is_absolute() or ".." in declared.parts:
                    break
                resolved = (root / declared).resolve()
                if not resolved.is_relative_to(root.resolve()):
                    break
                resolved_paths.append(resolved)
            if len(resolved_paths) != len(registry_paths) or any(not path.is_file() for path in resolved_paths):
                continue
            self._check_version(root, api)
            return root
        searched = "\n".join(f"  - {path}" for path in candidates) or "  (no candidates)"
        raise UETDNotFoundError(
            "UETD was not found. Set UETD_PATH, set uetd.path in the project configuration, "
            "or place UETD at one of the documented relative paths. Searched:\n" + searched
        )

    def _check_version(self, root: Path, api: dict) -> None:
        """Enforce compatibility from UETD's API manifest, with old-release fallback."""
        version_path = root / "version.json"
        metadata: dict = {}
        if isinstance(api.get("uetd_version"), str):
            metadata = api
        elif version_path.is_file():
            try:
                metadata = json.loads(version_path.read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError) as exc:
                raise UETDVersionIncompatible(f"Cannot read UETD version metadata at {version_path}: {exc}") from exc
        if not isinstance(metadata, dict) or not isinstance(metadata.get("uetd_version", metadata.get("version")), str):
            raise UETDVersionIncompatible(f"UETD API at {root} must declare uetd_version (or version.json must declare version)")
        version = metadata.get("uetd_version", metadata.get("version"))
        found = _version_tuple(version, "UETD")
        required = _version_tuple(self.REQUIRED_VERSION, "required UETD")
        if found < required:
            raise UETDVersionIncompatible(f"UETD {version} found at {root}, but UETD {self.REQUIRED_VERSION} or newer is required")
        minimum_generator = metadata.get("min_uef_gen")
        if minimum_generator is not None:
            if not isinstance(minimum_generator, str):
                raise UETDVersionIncompatible("UETD min_uef_gen metadata must be a semantic version string")
            current = _version_tuple(__version__, "uef-gen")
            minimum = _version_tuple(minimum_generator, "minimum uef-gen")
            if current < minimum:
                raise UETDVersionIncompatible(f"UETD requires uef-gen {minimum_generator} or newer; installed uef-gen is {__version__}")
