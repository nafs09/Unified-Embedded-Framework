"""Abstract contract implemented by each hardware programming backend."""

from __future__ import annotations

from abc import ABC, abstractmethod
from collections.abc import Callable

from uef_gen.deploy.models import (
    ChipIdentity,
    DeploymentProgress,
    DiscoveredProbe,
    FirmwareArtifact,
)


ProgressCallback = Callable[[DeploymentProgress], None]


class IDeployment(ABC):
    """One transport-specific implementation of connect/program/verify/reset."""

    @abstractmethod
    def connect(self, probe: DiscoveredProbe) -> bool:
        """Connect to the selected probe and target with finite timeouts."""
        raise NotImplementedError

    @abstractmethod
    def identify(self) -> ChipIdentity:
        """Read the connected target's identity before any write operation."""
        raise NotImplementedError

    @abstractmethod
    def halt(self) -> bool:
        """Halt target execution when supported by the selected transport."""
        raise NotImplementedError

    @abstractmethod
    def unlock_flash(self) -> bool:
        """Check protection state; never silently lower irreversible protection levels."""
        raise NotImplementedError

    @abstractmethod
    def erase(
        self,
        start_addr: int,
        end_addr: int,
        cb: ProgressCallback | None = None,
    ) -> bool:
        """Erase only the verified artifact range after explicit target confirmation."""
        raise NotImplementedError

    @abstractmethod
    def program(
        self,
        artifact: FirmwareArtifact,
        cb: ProgressCallback | None = None,
    ) -> bool:
        """Program the artifact's validated format and address range."""
        raise NotImplementedError

    @abstractmethod
    def verify(
        self,
        artifact: FirmwareArtifact,
        cb: ProgressCallback | None = None,
    ) -> bool:
        """Read back or use transport verification and compare against the artifact hash."""
        raise NotImplementedError

    @abstractmethod
    def reset(self, run: bool = True) -> bool:
        """Reset the target, optionally allowing firmware execution."""
        raise NotImplementedError

    @abstractmethod
    def disconnect(self) -> None:
        """Release transport resources even when an earlier phase failed."""
        raise NotImplementedError

    @abstractmethod
    def read_chip_identity(self) -> ChipIdentity:
        """Open a short-lived read-only connection and return target identity."""
        raise NotImplementedError
