from uef_gen.chips.database import DEFAULT_DATABASE
from uef_gen.chips.specs import ArchSpec, ChipSpec, FamilySpec

# This reference target contains no vendor pin, clock, DMA, or memory data.
# It is useful only for validating the project pipeline, never for board use.
_ARCH = ArchSpec(name="generic-word32", word_bits=32)
_FAMILY = FamilySpec(name="generic-reference", arch=_ARCH)
DEFAULT_DATABASE.register(ChipSpec(
    name="generic-reference", family=_FAMILY, package="unspecified", pin_count=0,
    flash_bytes=0, sram_bytes=0, peripherals={}, verified=False,
    source_note="Pipeline reference only. Replace with verified vendor data before target generation.",
))
