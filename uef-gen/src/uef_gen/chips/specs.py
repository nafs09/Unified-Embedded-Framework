from __future__ import annotations
from dataclasses import dataclass, field
from typing import Any

@dataclass(frozen=True)
class ArchSpec:
    name: str
    word_bits: int
    fpu: str = "none"
    dsp_simd: bool = False
    dcache: bool = False
    icache: bool = False
    mpu: bool = False
    dwt: bool = False
    hw_divide: bool = False
    cmsis_dsp: bool = False
    capabilities: frozenset[str] = frozenset()
    compiler_flags: tuple[str, ...] = ()

@dataclass(frozen=True)
class DMASpec:
    controller_count: int = 0
    channels_per_controller: int = 0
    muxed: bool = False
    request_map: dict[str, str] = field(default_factory=dict)

@dataclass(frozen=True)
class PeripheralTypeSpec:
    type_name: str
    generic_signals: tuple[str, ...] = ()
    dma_directions: tuple[str, ...] = ()
    type_caps: dict[str, Any] = field(default_factory=dict)

@dataclass(frozen=True)
class PeripheralInstanceSpec:
    instance_name: str
    type: PeripheralTypeSpec
    clock_domain: str = ""
    pins: dict[str, tuple[str, ...]] = field(default_factory=dict)
    af: dict[str, int] = field(default_factory=dict)  # "PIN|SIGNAL" -> AF number
    dma_requests: dict[str, str] = field(default_factory=dict)
    irqs: tuple[str, ...] = ()
    instance_caps: dict[str, Any] = field(default_factory=dict)

@dataclass(frozen=True)
class FamilySpec:
    name: str
    arch: ArchSpec
    dma: DMASpec = field(default_factory=DMASpec)
    peripheral_types: tuple[PeripheralTypeSpec, ...] = ()
    hrtim: bool = False
    fdcan: bool = False
    cordic: bool = False
    fmac: bool = False
    opamp: bool = False
    comp: bool = False
    usb_fs: bool = False
    dma_mux: bool = False
    clock_domains: dict[str, dict[str, int]] = field(default_factory=dict)

@dataclass(frozen=True)
class ChipSpec:
    name: str
    family: FamilySpec
    package: str
    pin_count: int
    flash_bytes: int
    sram_bytes: int
    peripherals: dict[str, PeripheralInstanceSpec] = field(default_factory=dict)
    overrides: dict[str, Any] = field(default_factory=dict)
    verified: bool = False
    source_note: str = ""

class ChipCapabilities:
    """Unified capability view; templates never branch on a part number."""

    def __init__(self, chip: ChipSpec):
        self._chip = chip

    def supports(self, capability: str) -> bool:
        if capability in self._chip.overrides:
            return bool(self._chip.overrides[capability])
        if capability in self._chip.family.__dataclass_fields__:
            return bool(getattr(self._chip.family, capability))
        if capability in self._chip.family.arch.__dataclass_fields__:
            return bool(getattr(self._chip.family.arch, capability))
        return capability in self._chip.family.arch.capabilities

    @property
    def has_fpu(self) -> bool:
        return self._chip.family.arch.fpu != "none"

    @property
    def dcache(self) -> bool:
        return self._chip.family.arch.dcache

    @property
    def hrtim(self) -> bool:
        return self.supports("hrtim")

    @property
    def fdcan(self) -> bool:
        return self.supports("fdcan")

    @property
    def cordic(self) -> bool:
        return self.supports("cordic")

    @property
    def dma_mux(self) -> bool:
        return self.supports("dma_mux")
