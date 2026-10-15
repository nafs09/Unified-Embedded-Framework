"""Strict JSON projection for compiler outputs passed to downstream backends."""

from __future__ import annotations

from dataclasses import fields, is_dataclass
from enum import Enum
from pathlib import Path
from typing import Any


def to_json_value(value: Any) -> Any:
    """Convert compiler data to JSON primitives without silently dropping nodes.

    Dataclasses carry an explicit ``$type`` tag so backend adapters can
    distinguish, for example, a state-machine action from a channel property.
    An unknown object is an error instead of falling back to ``repr``.
    """
    if value is None or isinstance(value, (str, int, float, bool)):
        return value
    if isinstance(value, Path):
        return str(value)
    if isinstance(value, Enum):
        return value.value
    if isinstance(value, dict):
        if not all(isinstance(key, (str, int, float, bool, Enum)) for key in value):
            raise TypeError("Compiler output dictionaries require scalar keys")
        return {str(key.value if isinstance(key, Enum) else key): to_json_value(item) for key, item in value.items()}
    if isinstance(value, (tuple, list)):
        return [to_json_value(item) for item in value]
    if isinstance(value, (set, frozenset)):
        return [to_json_value(item) for item in sorted(value, key=repr)]
    if is_dataclass(value) and not isinstance(value, type):
        return {"$type": type(value).__name__, **{item.name: to_json_value(getattr(value, item.name)) for item in fields(value)}}
    raise TypeError(f"Compiler output contains unsupported value {type(value).__name__}")
