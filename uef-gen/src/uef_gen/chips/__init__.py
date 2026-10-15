"""Architecture/family/part chip capability database."""

from uef_gen.chips.database import ChipDatabase, UnknownChipError
from uef_gen.chips.registry_records import ChipCatalogueError, find_chip_record, load_chip_catalogue, load_chip_database
from uef_gen.chips.specs import ArchSpec, ChipCapabilities, ChipSpec, DMASpec, FamilySpec, PeripheralInstanceSpec, PeripheralTypeSpec

__all__ = [
    "ArchSpec",
    "ChipCatalogueError",
    "ChipCapabilities",
    "ChipDatabase",
    "ChipSpec",
    "DMASpec",
    "FamilySpec",
    "PeripheralInstanceSpec",
    "PeripheralTypeSpec",
    "UnknownChipError",
    "find_chip_record",
    "load_chip_catalogue",
    "load_chip_database",
]
