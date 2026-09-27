from __future__ import annotations

import json
from pathlib import Path
from typing import Any


class ConfigurationError(ValueError):
    """Raised when a project configuration cannot be read or decoded."""

    pass


def load_configuration(path: Path) -> dict[str, Any]:
    """Load JSON or YAML and require a mapping at the document root."""
    if not path.is_file():
        raise ConfigurationError(f"hardware configuration does not exist: {path}")
    text = path.read_text(encoding="utf-8")
    try:
        if path.suffix.casefold() == ".json":
            value = json.loads(text)
        else:
            # Keep YAML optional for package consumers that use the JSON format only.
            import yaml

            value = yaml.safe_load(text)
    except Exception as exc:
        raise ConfigurationError(f"cannot parse configuration {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ConfigurationError("hardware configuration root must be a mapping")
    return value
