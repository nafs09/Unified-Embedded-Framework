"""UETD-owned host package registry and tool installation scaffolds."""

from uef_gen.toolchains.catalog import ToolchainCatalogueError, load_toolchain_catalogue

__all__ = ["ToolchainCatalogueError", "load_toolchain_catalogue"]
