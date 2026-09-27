from __future__ import annotations

from dataclasses import asdict, dataclass
from typing import Any


@dataclass(frozen=True)
class Diagnostic:
    """One stable diagnostic emitted across CLI, generation, and app bridges."""

    severity: str
    code: str
    message: str
    location: str = ""

    def to_dict(self) -> dict[str, Any]:
        """Serialize optional location only when the caller supplied one."""
        value = asdict(self)
        if not value["location"]:
            del value["location"]
        return value


def error(code: str, message: str, location: str = "") -> Diagnostic:
    """Create an error that prevents generation from being published."""
    return Diagnostic("error", code, message, location)


def warning(code: str, message: str, location: str = "") -> Diagnostic:
    """Create a warning that is visible without blocking generation."""
    return Diagnostic("warning", code, message, location)
