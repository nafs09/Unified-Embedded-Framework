from __future__ import annotations
from dataclasses import dataclass
from uef_gen.chips.specs import ChipSpec

class UnknownChipError(ValueError):
    pass

@dataclass
class ChipDatabase:
    """In-memory index materialized from UETD's canonical chip registry."""

    def __post_init__(self) -> None:
        self._chips: dict[str, ChipSpec] = {}
        self._canonical_names: dict[str, str] = {}

    def register(self, chip: ChipSpec, aliases: tuple[str, ...] = ()) -> None:
        names = tuple(dict.fromkeys((chip.name, chip.registry_id, *aliases)))
        for name in names:
            if not name:
                continue
            key = name.casefold()
            if key in self._chips:
                raise ValueError(f"chip name or alias is already registered: {name}")
        for name in names:
            if name:
                self._chips[name.casefold()] = chip
                self._canonical_names[name.casefold()] = chip.name

    def get(self, name: str) -> ChipSpec:
        try:
            return self._chips[name.casefold()]
        except KeyError as exc:
            raise UnknownChipError(name) from exc

    def names(self) -> tuple[str, ...]:
        return tuple(sorted({chip.name for chip in self._chips.values()}))
