from __future__ import annotations

import hashlib
import json
import shutil
from dataclasses import dataclass
from pathlib import Path


class SnapshotError(ValueError):
    """Raised when the packaged UEF release snapshot is incomplete or altered."""

    pass


@dataclass(frozen=True)
class UefSnapshot:
    root: Path
    version: str
    content_hash: str


class SnapshotManager:
    """Verify and copy a content-addressed UEF release into a generated project."""

    def __init__(self, root: Path | None = None):
        self.root = root or Path(__file__).resolve().parent / "uef_source"

    def verify(self) -> UefSnapshot:
        """Verify metadata, content hash, and required manifests before use."""
        metadata_path = self.root / "snapshot.json"
        if not metadata_path.is_file():
            raise SnapshotError("UEF snapshot metadata is missing")
        metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
        expected = str(metadata.get("content_hash", ""))
        actual = self.compute_hash(self.root)
        if not expected or actual != expected:
            raise SnapshotError(
                f"UEF snapshot hash mismatch: expected {expected or '<missing>'}, got {actual}"
            )
        required_manifests = (self.root / "uef_modules.json", self.root / "uef_api.json")
        if any(not path.is_file() for path in required_manifests):
            raise SnapshotError("UEF snapshot is incomplete: uef_modules.json and uef_api.json are required")
        return UefSnapshot(self.root, str(metadata.get("version", "")), actual)

    @staticmethod
    def compute_hash(root: Path) -> str:
        """Hash path names and file bytes in stable order, excluding only the hash record."""
        digest = hashlib.sha256()
        for path in sorted(p for p in root.rglob("*") if p.is_file() and p.name != "snapshot.json"):
            relative = path.relative_to(root).as_posix().encode("utf-8")
            digest.update(len(relative).to_bytes(4, "big"))
            digest.update(relative)
            content = path.read_bytes()
            digest.update(len(content).to_bytes(8, "big"))
            digest.update(content)
        return f"sha256:{digest.hexdigest()}"

    @staticmethod
    def copy_modules(
        snapshot: UefSnapshot,
        selected: tuple[str, ...],
        module_manifest: dict,
        destination: Path,
    ) -> list[str]:
        """Copy the shared headers and selected module sources into project-local `uef/`."""
        written: list[str] = []
        # Public headers form a shared include tree and cross module boundaries.
        # Copy it once; copying only each module's primary header misses nested
        # type dependencies such as UPAL DMA and UHAL target contracts.
        headers_root = str(module_manifest.get("headers_root", "include"))
        source_headers = snapshot.root / headers_root
        if not source_headers.is_dir():
            raise SnapshotError(f"UEF snapshot public include tree is missing: {headers_root}")
        target_headers = destination / "uef" / headers_root
        shutil.copytree(source_headers, target_headers, dirs_exist_ok=True)
        written.extend(
            path.relative_to(destination).as_posix()
            for path in target_headers.rglob("*")
            if path.is_file()
        )

        for name in selected:
            record = module_manifest["modules"][name]
            for relative in (*record.get("sources", []), *record.get("headers", [])):
                relative_path = Path(relative)
                if relative_path.is_absolute() or ".." in relative_path.parts:
                    raise SnapshotError(
                        f"selected UEF module {name} has an unsafe snapshot path {relative}"
                    )
                source = snapshot.root / relative_path
                if not source.is_file():
                    raise SnapshotError(
                        f"selected UEF module {name} references missing snapshot file {relative}"
                    )
                target = destination / "uef" / relative_path
                if target.is_file():
                    # A legacy per module header may also be inside headers_root.
                    continue
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(source, target)
                written.append(target.relative_to(destination).as_posix())
        return written
