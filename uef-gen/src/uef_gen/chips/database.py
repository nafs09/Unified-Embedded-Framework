from __future__ import annotations
from dataclasses import dataclass
from uef_gen.chips.specs import ChipSpec

class UnknownChipError(ValueError):
    pass

@dataclass
class ChipDatabase:
    """Explicit registry for verified part data supplied by this distribution or extensions."""

    def __post_init__(self) -> None:
        self._chips: dict[str, ChipSpec] = {}

    def register(self, chip: ChipSpec) -> None:
        key = chip.name.casefold()
        if key in self._chips:
            raise ValueError(f"chip is already registered: {chip.name}")
        self._chips[key] = chip

    def get(self, name: str) -> ChipSpec:
        try:
            return self._chips[name.casefold()]
        except KeyError as exc:
            raise UnknownChipError(name) from exc

    def names(self) -> tuple[str, ...]:
        return tuple(sorted(chip.name for chip in self._chips.values()))

DEFAULT_DATABASE = ChipDatabase()
