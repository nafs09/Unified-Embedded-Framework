"""Source model for `.uefs` multi-target compositions."""

from __future__ import annotations
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any
from uef_gen.language.frontends.project.ast import (
    EnumDefinition,
    FlagsDefinition,
    FixedPointDefinition,
    RecordBoundary,
    SourceSpan,
    TypeSpec,
    VariantDefinition,
)

@dataclass(frozen=True)
class SystemNodeSpec:
    """One independent target project and its system role."""

    name: str
    project_path: Path
    role: str
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class ExternalEndpointSpec:
    """External hardware/software endpoint outside UETD code generation."""

    name: str
    implementation: str
    role: str
    attached_to: str | None = None
    source_span: SourceSpan | None = None
    properties: dict[str, Any] = field(default_factory=dict)

@dataclass(frozen=True)
class SystemRecordSpec:
    """Fixed typed record used by one or more node link contracts."""

    name: str
    fields: tuple[tuple[str, TypeSpec, dict[str, Any]], ...]
    source_span: SourceSpan | None = None
    boundary: RecordBoundary | None = None

@dataclass(frozen=True)
class SystemProtocolSpec:
    """Project-owned wire layout used to generate MCU-side codecs."""

    name: str
    transport: str | None
    framing: dict[str, Any]
    header: dict[str, Any]
    sequence: dict[str, Any]
    checksum: dict[str, Any]
    messages: tuple[dict[str, Any], ...]
    receiver: dict[str, Any]
    properties: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class SystemLinkSpec:
    """Directed bounded message link between two declared endpoints."""

    name: str
    source_endpoint: str
    destination_endpoint: str
    contract_name: str | None
    protocol_name: str | None
    properties: dict[str, Any] = field(default_factory=dict)
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class HybridSystemSpec:
    """Cross-target timing/coupling relation; contains no executable code."""

    name: str
    properties: dict[str, Any]
    source_span: SourceSpan | None = None

@dataclass(frozen=True)
class SystemDocument:
    """Parsed multi-target composition before project/link verification."""

    name: str
    nodes: tuple[SystemNodeSpec, ...]
    external_endpoints: tuple[ExternalEndpointSpec, ...]
    records: tuple[SystemRecordSpec, ...]
    enums: tuple[EnumDefinition, ...]
    protocols: tuple[SystemProtocolSpec, ...]
    links: tuple[SystemLinkSpec, ...]
    hybrid_systems: tuple[HybridSystemSpec, ...]
    delivery_policies: tuple[dict[str, Any], ...] = ()
    source_path: Path | None = None
    source_span: SourceSpan | None = None
    variants: tuple[VariantDefinition, ...] = ()
    flags_types: tuple[FlagsDefinition, ...] = ()
    fixed_point_types: tuple[FixedPointDefinition, ...] = ()
