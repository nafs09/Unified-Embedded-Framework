"""Artifact provenance and storage boundary.

An artifact must remain tied to the exact project, chip description, linker map,
and UEF snapshot that produced it. This module does not yet persist artifacts.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

from uef_gen.deploy.models import BuildResult, FirmwareArtifact


class ArtifactStoreError(RuntimeError):
    """Raised for invalid, stale, or unreadable firmware artifacts."""


def sha256_file(path: Path) -> str:
    """Hash one file incrementally so large firmware images do not load into memory."""
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return f"sha256:{digest.hexdigest()}"


class ArtifactStore:
    """Persist artifact metadata and verify it before a later deployment."""

    def create(
        self,
        result: BuildResult,
        resolved: object,
        project_yaml_path: Path,
        destination: Path,
    ) -> FirmwareArtifact:
        """Create an artifact record and its durable metadata sidecar."""
        raise NotImplementedError(
            "TODO(artifact-store-create): require a successful build and existing output files; "
            "compute image/project/chip/snapshot hashes; include linker-derived flash range; "
            "write a versioned metadata sidecar atomically; and never overwrite an unrelated "
            "artifact directory."
        )

    @classmethod
    def load(cls, metadata_path: Path) -> FirmwareArtifact:
        """Load only a supported metadata version whose referenced files still hash correctly."""
        raise NotImplementedError(
            "TODO(artifact-store-load): parse a versioned sidecar, reject paths escaping its "
            "artifact root, verify every image hash, reconstruct the result summary, and report "
            "staleness instead of silently accepting changed files."
        )

    def verify(self, artifact: FirmwareArtifact) -> None:
        """Raise ArtifactStoreError when image provenance or content no longer matches."""
        raise NotImplementedError(
            "TODO(artifact-store-verify): check ELF/HEX/BIN existence and stored content hashes, "
            "ensure BuildResult.success is true, and reject missing target or snapshot identity."
        )
