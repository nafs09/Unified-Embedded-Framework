from __future__ import annotations

from importlib.metadata import entry_points

_loaded = False


def load_extensions() -> None:
    """Load explicit installed uef-gen extensions once per process."""
    global _loaded
    if _loaded:
        return
    _loaded = True
    for extension in entry_points(group="uef_gen.extensions"):
        register = extension.load()
        register()
