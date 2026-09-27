"""Build, artifact, target discovery, and deployment interfaces.

Importing this package has no USB, process, or hardware side effects.
"""

from uef_gen.deploy.artifact_store import ArtifactStore, ArtifactStoreError
from uef_gen.deploy.backends import (
    DFUDeployment,
    JLinkDeployment,
    OpenOCDDeployment,
    PyOCDDeployment,
    STLinkDeployment,
    TIUniFlashDeployment,
    UARTBootDeployment,
)
from uef_gen.deploy.build_manager import (
    BuildManager,
    ToolchainNotFoundError,
    UnsupportedArchitectureError,
    detect_toolchain,
    toolchain_for_arch,
)
from uef_gen.deploy.backend_factory import DeploymentBackendFactory
from uef_gen.deploy.ideployment import IDeployment, ProgressCallback
from uef_gen.deploy.models import (
    BuildConfig,
    BuildResult,
    ChipIdentity,
    DeploymentProgress,
    DeploymentResult,
    DeploymentState,
    DiscoveredProbe,
    FirmwareArtifact,
    ProbeType,
    Toolchain,
    ToolchainFamily,
)
from uef_gen.deploy.orchestrator import DeploymentOrchestrator
from uef_gen.deploy.target_discovery import TargetDiscovery

__all__ = [
    "ArtifactStore",
    "ArtifactStoreError",
    "BuildConfig",
    "BuildManager",
    "BuildResult",
    "ChipIdentity",
    "DeploymentBackendFactory",
    "DeploymentOrchestrator",
    "DeploymentProgress",
    "DeploymentResult",
    "DeploymentState",
    "DiscoveredProbe",
    "FirmwareArtifact",
    "IDeployment",
    "DFUDeployment",
    "JLinkDeployment",
    "OpenOCDDeployment",
    "PyOCDDeployment",
    "ProbeType",
    "ProgressCallback",
    "STLinkDeployment",
    "TargetDiscovery",
    "TIUniFlashDeployment",
    "Toolchain",
    "ToolchainFamily",
    "ToolchainNotFoundError",
    "UnsupportedArchitectureError",
    "UARTBootDeployment",
    "detect_toolchain",
    "toolchain_for_arch",
]
