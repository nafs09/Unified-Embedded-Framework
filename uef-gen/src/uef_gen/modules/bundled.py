"""Resolve exact UEF-managed source packages for the selected target profile."""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from uef_gen.registry import UEFRegistryError, load_module_manifest


class BundledDependencyError(ValueError):
    """Raised when a pinned UEF dependency cannot be safely selected."""


@dataclass(frozen=True)
class BundledDependencySelection:
    name: str
    root: str
    version: str
    revision: str
    profile: str
    sources: tuple[str, ...]
    include_directories: tuple[str, ...]
    defines: tuple[str, ...]
    license_files: tuple[str, ...]


def resolve_bundled_dependencies(
    uef_root: Path,
    names: tuple[str, ...],
    config: dict[str, Any],
    resolved: Any,
) -> tuple[BundledDependencySelection, ...]:
    """Select and validate only dependency files declared for this target.

    This function never downloads or updates upstream code. The lock, source
    tree and exact architecture/compiler profile must already be present in the
    UEF checkout before a project can include the dependency.
    """
    if not names:
        return ()
    try:
        module_manifest = load_module_manifest(uef_root)
    except UEFRegistryError as exc:
        raise BundledDependencyError(f"cannot read UEF module registry: {exc}") from exc
    registry = module_manifest.get("bundled_dependencies", {})
    if not isinstance(registry, dict):
        raise BundledDependencyError("UEF bundled_dependencies must be an object")

    target_config = config.get("target", {})
    target_config = target_config if isinstance(target_config, dict) else {}
    architecture = str(getattr(resolved.chip.family.arch, "name", "")).strip()
    compiler = str(target_config.get("toolchain", "")).strip()
    if not compiler:
        compiler = _default_toolchain(architecture)

    selections: list[BundledDependencySelection] = []
    for name in names:
        record = registry.get(name)
        if not isinstance(record, dict):
            raise BundledDependencyError(f"UEF bundled dependency {name!r} has no manifest record")
        root_rel = _safe_relative(record.get("root"), f"{name}.root")
        lock_rel = _safe_relative(record.get("lock_file"), f"{name}.lock_file")
        integration_rel = _safe_relative(
            record.get("integration_manifest"), f"{name}.integration_manifest"
        )
        package_root = _under_root(uef_root, root_rel, f"{name}.root")
        lock_path = _under_root(uef_root, lock_rel, f"{name}.lock_file")
        integration_path = _under_root(uef_root, integration_rel, f"{name}.integration_manifest")
        try:
            lock = json.loads(lock_path.read_text(encoding="utf-8"))
            integration = json.loads(integration_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            raise BundledDependencyError(
                f"{name} source package is incomplete; lock/integration metadata cannot be read: {exc}"
            ) from exc
        revision = lock.get("revision") if isinstance(lock, dict) else None
        if not isinstance(revision, str) or not re.fullmatch(r"[0-9a-fA-F]{40}", revision):
            raise BundledDependencyError(
                f"{name} is not pinned: set a full 40-character upstream commit in {lock_rel}"
            )
        if not package_root.is_dir():
            raise BundledDependencyError(
                f"{name} source tree is missing at {root_rel}; add the exact locked upstream checkout"
            )
        license_files = _string_list(
            lock.get("license_files", []), f"{name}.license_files"
        )
        if not license_files:
            raise BundledDependencyError(
                f"{name} lock must list the upstream license text files to preserve in generated projects"
            )
        for relative in license_files:
            license_path = _under_root(package_root, relative, f"{name} license")
            if not license_path.is_file():
                raise BundledDependencyError(
                    f"{name} lock references missing license file {relative}"
                )
        if not isinstance(integration, dict) or integration.get("schema_version") != "1.0":
            raise BundledDependencyError(f"{name} integration manifest is invalid or unsupported")
        profiles = integration.get("profiles")
        if not isinstance(profiles, dict):
            raise BundledDependencyError(f"{name} profiles must be an object")
        profile_key, profile = _find_profile(profiles, architecture, compiler)
        if profile is None:
            raise BundledDependencyError(
                f"{name} has no verified source profile for architecture {architecture!r} "
                f"and toolchain {compiler!r}"
            )

        common_sources = _string_list(integration.get("common_sources", []), f"{name}.common_sources")
        port_sources = _string_list(profile.get("port_sources", []), f"{name}.{profile_key}.port_sources")
        heap_source = profile.get("heap_source", "")
        if not isinstance(heap_source, str):
            raise BundledDependencyError(f"{name}.{profile_key}.heap_source must be a path string")
        source_paths = [*common_sources, *port_sources]
        if heap_source:
            source_paths.append(_safe_relative(heap_source, f"{name}.{profile_key}.heap_source"))
        includes = [
            *_string_list(integration.get("common_include_directories", []), f"{name}.common_include_directories"),
            *_string_list(profile.get("include_directories", []), f"{name}.{profile_key}.include_directories"),
        ]
        defines = _definition_list(profile.get("defines", []), f"{name}.{profile_key}.defines")
        for relative in source_paths:
            source = _under_root(package_root, relative, f"{name} source")
            if not source.is_file():
                raise BundledDependencyError(
                    f"{name} profile {profile_key!r} references missing source {relative}"
                )
        for relative in includes:
            include = _under_root(package_root, relative, f"{name} include directory")
            if not include.is_dir():
                raise BundledDependencyError(
                    f"{name} profile {profile_key!r} references missing include directory {relative}"
                )
        selections.append(
            BundledDependencySelection(
                name=name,
                root=root_rel,
                version=str(lock.get("version", "")),
                revision=revision.lower(),
                profile=profile_key,
                sources=tuple(dict.fromkeys(source_paths)),
                include_directories=tuple(dict.fromkeys(includes)),
                defines=tuple(defines),
                license_files=tuple(dict.fromkeys(license_files)),
            )
        )
    return tuple(selections)


def _find_profile(
    profiles: dict[str, Any], architecture: str, compiler: str
) -> tuple[str, dict[str, Any] | None]:
    for key, value in profiles.items():
        if not isinstance(key, str) or not isinstance(value, dict):
            continue
        profile_architecture = value.get("architecture")
        profile_compiler = value.get("compiler")
        if (
            isinstance(profile_architecture, str)
            and profile_architecture.casefold() == architecture.casefold()
            and isinstance(profile_compiler, str)
            and profile_compiler.casefold() == compiler.casefold()
        ):
            return key, value
    return "", None


def _default_toolchain(architecture: str) -> str:
    """Use the existing automatic architecture choice when config omits it."""
    name = architecture.casefold()
    if "cortex" in name or "arm" in name:
        return "gcc-arm-none-eabi"
    if "riscv" in name or "rv32" in name:
        return "riscv-none-elf-gcc"
    if "c2000" in name or "c28x" in name:
        return "ti-cgt-c2000"
    return "host-gcc"


def _string_list(value: Any, location: str) -> list[str]:
    if not isinstance(value, list) or any(not isinstance(item, str) or not item.strip() for item in value):
        raise BundledDependencyError(f"{location} must be an array of non-empty strings")
    return [_safe_relative(item, location) for item in value]


def _definition_list(value: Any, location: str) -> list[str]:
    """Validate compiler definitions as single safe build-list entries."""
    if not isinstance(value, list):
        raise BundledDependencyError(f"{location} must be an array of compile definitions")
    definitions: list[str] = []
    for item in value:
        if (
            not isinstance(item, str)
            or not item.strip()
            or item.startswith("-")
            or any(character in item for character in ";\r\n")
        ):
            raise BundledDependencyError(
                f"{location} entries must be non-empty single compile definitions"
            )
        definitions.append(item.strip())
    return definitions


def _safe_relative(value: Any, location: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise BundledDependencyError(f"{location} must be a non-empty relative path")
    path = Path(value.replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise BundledDependencyError(f"{location} must remain inside the UEF checkout")
    return path.as_posix()


def _under_root(root: Path, relative: str, location: str) -> Path:
    candidate = (root / Path(relative)).resolve()
    try:
        candidate.relative_to(root.resolve())
    except ValueError as exc:
        raise BundledDependencyError(f"{location} resolves outside its declared root") from exc
    return candidate
