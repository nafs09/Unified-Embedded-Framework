from __future__ import annotations
from dataclasses import dataclass, field
from typing import Any
from uef_gen.chips.specs import ChipCapabilities, ChipSpec
from uef_gen.diagnostics import Diagnostic

@dataclass(frozen=True)
class ResolvedPeripheral:
    instance: str
    type_name: str
    pins: dict[str, str]
    alternate_functions: dict[str, int]
    dma_channels: dict[str, str]
    irq_names: tuple[str, ...]
    key: str = ""

@dataclass(frozen=True)
class ResolvedTask:
    name: str
    period_us: int
    priority: int
    stack_bytes: int
    execution_context: str = "task"

@dataclass(frozen=True)
class ResolvedProject:
    name: str
    chip: ChipSpec
    capabilities: ChipCapabilities
    arithmetic: str
    rtos: str
    modules: tuple[str, ...]
    peripherals: tuple[ResolvedPeripheral, ...]
    tasks: tuple[ResolvedTask, ...]
    defines: tuple[str, ...]
    compiler_flags: tuple[str, ...]
    diagnostics: tuple[Diagnostic, ...] = field(default_factory=tuple)
    user_configuration: dict[str, Any] = field(default_factory=dict)

    @property
    def valid(self) -> bool:
        return not any(d.severity == "error" for d in self.diagnostics)
