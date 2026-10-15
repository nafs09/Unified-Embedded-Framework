"""Packaged syntax and DSL-builtin registry for standalone `.uef` projects.

UETD descriptor names and all component/target facts remain in the UETD checkout.
"""

from uef_gen.language.registry.loader import (
    LanguageBuiltin,
    LanguageConstruct,
    LanguageRegistry,
    LanguageRegistryError,
    load_language_registry,
    resolve_builtin,
    resolve_construct,
)

__all__ = [
    "LanguageConstruct",
    "LanguageRegistry",
    "LanguageRegistryError",
    "LanguageBuiltin",
    "load_language_registry",
    "resolve_builtin",
    "resolve_construct",
]
