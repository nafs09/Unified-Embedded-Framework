from __future__ import annotations
import hashlib
import json
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Any
from uef_gen.chips.development_boards import load_development_boards
from uef_gen.uetd_adapter.registry import load_module_manifest
from uef_gen.toolchains.catalog import load_toolchain_catalogue

class SnapshotError(ValueError):
    """Raised when a located UETD checkout is incomplete or internally inconsistent."""

    pass

@dataclass(frozen=True)
class UETDSnapshot:
    root: Path
    version: str
    source_hash: str

class SnapshotManager:
    """Validate a live UETD checkout and copy its selected modules into a project."""

    def __init__(self, root: Path | None = None):
        self.root = root
        self.warnings: list[str] = []

    def verify(self) -> UETDSnapshot:
        """Validate release metadata/manifests and compute a reproducible source fingerprint."""
        if self.root is None:
            from uef_gen.uetd_adapter.locator import UETDLocator
            locator = UETDLocator()
            self.root = locator.locate()
            self.warnings.extend(locator.warnings)
        version_path = self.root / "version.json"
        version_metadata: dict = {}
        if version_path.is_file():
            try:
                version_metadata = json.loads(version_path.read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError) as exc:
                raise SnapshotError(f"UETD version metadata cannot be read: {exc}") from exc
            if not isinstance(version_metadata, dict):
                raise SnapshotError("UETD version metadata must be a JSON object")
        api_path = self.root / "api.json"
        if not api_path.is_file():
            raise SnapshotError("UETD checkout is incomplete: api.json is required")
        try:
            api = json.loads(api_path.read_text(encoding="utf-8"))
            modules = load_module_manifest(self.root)
            development_boards = load_development_boards(self.root)
            toolchains = load_toolchain_catalogue(self.root)
        except (OSError, json.JSONDecodeError, ValueError) as exc:
            raise SnapshotError(f"UETD API or registry entries cannot be read: {exc}") from exc
        if not isinstance(modules, dict) or not isinstance(api, dict) or not isinstance(development_boards, dict) or not isinstance(toolchains, dict):
            raise SnapshotError("UETD API, module, development-board, and tool package registries must be valid")
        try:
            declared_registries = [
                api["module_registry"],
                api["uhal_api"]["target_profiles"]["target_family_catalogue"]["path"],
                api["uhal_api"]["target_profiles"]["target_family_catalogue"]["registry_schema_path"],
                api["uhal_api"]["chip_catalogue"]["path"],
                api["uhal_api"]["chip_catalogue"]["schema_path"],
                api["uhal_api"]["chip_catalogue"]["registry_schema_path"],
                api["uhal_api"]["development_boards"]["path"],
                api["uhal_api"]["development_boards"]["schema_path"],
                api["uhal_api"]["development_boards"]["registry_schema_path"],
                api["uhal_api"]["toolchain_catalogue"]["path"],
                api["uhal_api"]["toolchain_catalogue"]["schema_path"],
                api["uhal_api"]["toolchain_catalogue"]["registry_schema_path"],
            ]
        except (KeyError, TypeError) as exc:
            raise SnapshotError("UETD API does not declare every canonical registry") from exc
        for relative in declared_registries:
            path = Path(str(relative).replace(chr(92), "/"))
            if path.is_absolute() or ".." in path.parts or not (self.root / path).is_file():
                raise SnapshotError(f"UETD API declares a missing or unsafe registry path: {relative!r}")
        version = str(api.get("uetd_version") or version_metadata.get("version") or modules.get("uetd_version") or "")
        if not version:
            raise SnapshotError("UETD version is missing from api.json, version.json, and the module registry")
        declared_versions = {
            value for value in (modules.get("uetd_version"), api.get("uetd_version"), version_metadata.get("version"))
            if isinstance(value, str) and value
        }
        if declared_versions and declared_versions != {version}:
            raise SnapshotError(f"UETD version metadata disagrees with manifests: version.json={version}, manifests={sorted(declared_versions)}")
        actual = self.compute_hash(self.root)
        return UETDSnapshot(self.root, version, actual)

    @staticmethod
    def compute_hash(root: Path) -> str:
        """Fingerprint source, templates, release metadata, and manifests in stable order."""
        digest = hashlib.sha256()
        included_roots = ("include", "src", "templates", "third_party", "registry")
        excluded_parts = {".git", ".vs", "out", "build", "__pycache__"}
        candidates = [root / "version.json", root / "api.json"]
        for directory in included_roots:
            tree = root / directory
            if tree.is_dir():
                candidates.extend(tree.rglob("*"))
        paths = sorted(
            path
            for path in candidates
            if path.is_file() and not excluded_parts.intersection(path.relative_to(root).parts) and path.suffix.casefold() not in {".pyc", ".pyo"}
        )
        for path in paths:
            relative = path.relative_to(root).as_posix().encode("utf-8")
            digest.update(len(relative).to_bytes(4, "big"))
            digest.update(relative)
            content = path.read_bytes()
            digest.update(len(content).to_bytes(8, "big"))
            digest.update(content)
        return f"sha256:{digest.hexdigest()}"

    @staticmethod
    def copy_manifests(snapshot: UETDSnapshot, destination: Path) -> list[str]:
        """Copy UETD registry indexes and detail manifests into generated provenance."""
        manifest_dir = destination / "manifest"
        manifest_dir.mkdir(parents=True, exist_ok=True)
        provenance_root = manifest_dir / "uetd"
        written: list[str] = []
        root_files = (
            "api.json",
            "version.json",
            "third_party/FreeRTOS-Kernel-11.3.1/freertos-kernel.lock.json",
            "third_party/FreeRTOS-Kernel-11.3.1/uetd-integration.json",
        )
        for name in root_files:
            source = snapshot.root / name
            if not source.is_file():
                if name == "api.json":
                    raise SnapshotError("UETD provenance manifest is missing: api.json")
                continue
            # Keep all UETD-owned provenance under one self-contained root. The
            # API's repository-relative registry paths then resolve beside it.
            target = provenance_root / name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
            written.append(target.relative_to(destination).as_posix())
        # A copied registry must remain resolvable for later inspection, so include
        # each referenced detail manifest alongside its small canonical index.
        excluded_parts = {".git", ".vs", "out", "build", "__pycache__"}
        for directory in ("registry",):
            source_root = snapshot.root / directory
            if not source_root.is_dir():
                continue
            for source in sorted(source_root.rglob("*")):
                if not source.is_file() or excluded_parts.intersection(source.relative_to(snapshot.root).parts):
                    continue
                if source.suffix.casefold() in {".pyc", ".pyo"}:
                    continue
                relative = source.relative_to(snapshot.root)
                target = provenance_root / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
                written.append(target.relative_to(destination).as_posix())
        return written

    @staticmethod
    def copy_modules(snapshot: UETDSnapshot, selected: tuple[str, ...], module_manifest: dict, destination: Path) -> list[str]:
        """Copy selected module sources and their explicit public-header closure."""
        written: list[str] = []
        headers_root = str(module_manifest.get("headers_root", "include"))
        for name in selected:
            record = module_manifest["modules"][name]
            sources = record.get("sources", [])
            headers = record.get("headers")
            if not isinstance(sources, list) or any(not isinstance(path, str) or not path.strip() for path in sources):
                raise SnapshotError(f"selected UETD module {name} has invalid sources")
            if not isinstance(headers, list) or any(not isinstance(path, str) or not path.strip() for path in headers) or (sources and not headers):
                raise SnapshotError(f"selected UETD module {name} must declare its public headers explicitly")
            for kind, paths in (("source", sources), ("header", headers)):
                for relative in paths:
                    relative_path = Path(relative)
                    if relative_path.is_absolute() or ".." in relative_path.parts:
                        raise SnapshotError(f"selected UETD module {name} has an unsafe {kind} path {relative}")
                    normalized = relative_path.as_posix()
                    if kind == "header" and not normalized.startswith(headers_root.rstrip("/") + "/"):
                        raise SnapshotError(f"selected UETD module {name} header is outside {headers_root}: {relative}")
                    if kind == "source" and not normalized.startswith("src/"):
                        raise SnapshotError(f"selected UETD module {name} source is outside src/: {relative}")
                    source = snapshot.root / relative_path
                    if not source.is_file():
                        raise SnapshotError(f"selected UETD module {name} references missing {kind} {relative}")
                    target = destination / "uetd" / relative_path
                    if target.is_file():
                        continue
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(source, target)
                    written.append(target.relative_to(destination).as_posix())
        return written

    @staticmethod
    def copy_bundled_dependencies(snapshot: UETDSnapshot, selections: tuple[Any, ...], destination: Path) -> list[str]:
        """Copy only the source/include paths selected by UETD dependency metadata."""
        written: list[str] = []
        for selection in selections:
            package_root = Path(selection.root)
            for relative in selection.sources:
                relative_path = Path(relative)
                source = snapshot.root / package_root / relative_path
                target = destination / package_root / relative_path
                if not source.is_file():
                    raise SnapshotError(f"bundled dependency {selection.name} selected missing source {relative}")
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(source, target)
                written.append(target.relative_to(destination).as_posix())
            for relative in selection.include_directories:
                relative_path = Path(relative)
                source = snapshot.root / package_root / relative_path
                target = destination / package_root / relative_path
                if not source.is_dir():
                    raise SnapshotError(f"bundled dependency {selection.name} selected missing include directory {relative}")
                shutil.copytree(source, target, dirs_exist_ok=True)
                written.extend(path.relative_to(destination).as_posix() for path in target.rglob("*") if path.is_file())
            for relative in selection.license_files:
                relative_path = Path(relative)
                source = snapshot.root / package_root / relative_path
                target = destination / package_root / relative_path
                if not source.is_file():
                    raise SnapshotError(f"bundled dependency {selection.name} license file is missing: {relative}")
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(source, target)
                written.append(target.relative_to(destination).as_posix())
        return sorted(set(written))
