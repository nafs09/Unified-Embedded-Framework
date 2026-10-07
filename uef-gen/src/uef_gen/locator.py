"""Locate and validate the separately maintained UEF source checkout."""

from __future__ import annotations

import json
import os
import re
from pathlib import Path

from uef_gen import __version__


class UEFNotFoundError(FileNotFoundError):
    """Raised when no candidate directory contains the UEF release contract."""


class UEFVersionIncompatible(ValueError):
    """Raised when the located framework is older than this generator supports."""


_VERSION_PATTERN = re.compile(r"^(\d+)\.(\d+)\.(\d+)(?:[-+][0-9A-Za-z.-]+)?$")


def _version_tuple(value: str, field: str) -> tuple[int, int, int]:
    """Parse the numeric compatibility portion of a semantic version."""
    match = _VERSION_PATTERN.fullmatch(value.strip())
    if not match:
        raise UEFVersionIncompatible(f"Invalid {field} semantic version: {value!r}")
    return tuple(int(part) for part in match.groups())


class UEFLocator:
    """Resolve UEF_PATH, project configuration, then documented relative paths."""

    REQUIRED_VERSION = "1.1.0"

    def __init__(self, project_directory: Path | None = None):
        self.project_directory = (project_directory or Path.cwd()).expanduser().resolve()
        self.warnings: list[str] = []
        self.searched: tuple[Path, ...] = ()

    def locate(self, config: dict | None = None) -> Path:
        """Return the first UEF root with its API-declared registries and a compatible version."""
        config = config or {}
        uef_config = config.get("uef", {})
        if not isinstance(uef_config, dict):
            uef_config = {}

        raw_candidates: list[Path] = []
        if env_path := os.environ.get("UEF_PATH", "").strip():
            raw_candidates.append(Path(env_path).expanduser())
        configured_path = uef_config.get("path")
        if isinstance(configured_path, str) and configured_path.strip():
            raw_candidates.append(Path(configured_path).expanduser())
        raw_candidates.extend((Path("../UEF"), Path("../../UEF"), Path("./UEF")))

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
            api_path = root / "uef_api.json"
            if not api_path.is_file():
                continue
            try:
                api = json.loads(api_path.read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError):
                continue
            if not isinstance(api, dict):
                continue

            # UEF's API contract is the sole locator for its live catalogues.
            try:
                registry_paths = [
                    api["module_registry"],
                    api["ucon_api"]["catalogue_manifest"],
                    api["uhal_api"]["target_profiles"]["target_family_catalogue"]["path"],
                    api["uhal_api"]["chip_base_catalogue"]["path"],
                    api["uhal_api"]["target_profiles"]["target_family_catalogue"]["registry_schema_path"],
                    api["uhal_api"]["chip_base_catalogue"]["schema_path"],
                    api["uhal_api"]["chip_base_catalogue"]["registry_schema_path"],
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
            if len(resolved_paths) != len(registry_paths) or any(
                not path.is_file() for path in resolved_paths
            ):
                continue
            self._check_version(root)
            return root

        searched = "\n".join(f"  - {path}" for path in candidates) or "  (no candidates)"
        raise UEFNotFoundError(
            "UEF was not found. Set UEF_PATH, set uef.path in the project configuration, "
            "or place UEF at one of the documented relative paths. Searched:\n" + searched
        )

    def _check_version(self, root: Path) -> None:
        """Enforce the minimum UEF and uef-gen versions declared by UEF."""
        version_path = root / "version.json"
        if not version_path.is_file():
            self.warnings.append(
                f"UEF at {root} has no version.json; treating it as a pre-versioned development checkout"
            )
            return

        try:
            metadata = json.loads(version_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            raise UEFVersionIncompatible(f"Cannot read UEF version metadata at {version_path}: {exc}") from exc
        if not isinstance(metadata, dict) or not isinstance(metadata.get("version"), str):
            raise UEFVersionIncompatible(f"UEF version metadata is malformed: {version_path}")

        found = _version_tuple(metadata["version"], "UEF")
        required = _version_tuple(self.REQUIRED_VERSION, "required UEF")
        if found < required:
            raise UEFVersionIncompatible(
                f"UEF {metadata['version']} found at {root}, but UEF {self.REQUIRED_VERSION} or newer is required"
            )

        minimum_generator = metadata.get("min_uef_gen")
        if minimum_generator is not None:
            if not isinstance(minimum_generator, str):
                raise UEFVersionIncompatible("UEF min_uef_gen metadata must be a semantic version string")
            current = _version_tuple(__version__, "uef-gen")
            minimum = _version_tuple(minimum_generator, "minimum uef-gen")
            if current < minimum:
                raise UEFVersionIncompatible(
                    f"UEF requires uef-gen {minimum_generator} or newer; installed uef-gen is {__version__}"
                )
