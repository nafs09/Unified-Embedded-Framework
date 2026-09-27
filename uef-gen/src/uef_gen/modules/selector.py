from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path


class ModuleSelectionError(ValueError):
    pass


@dataclass(frozen=True)
class ModuleSelection:
    modules: tuple[str, ...]
    sources: tuple[str, ...]
    include_directories: tuple[str, ...]
    defines: tuple[str, ...]
    required_capabilities: tuple[str, ...]
    external_dependencies: tuple[str, ...]


class ModuleSelector:
    def __init__(self, snapshot_root: Path):
        manifest_path = snapshot_root / "uef_modules.json"
        if not manifest_path.is_file():
            raise ModuleSelectionError(f"UEF module manifest is missing: {manifest_path}")
        document = json.loads(manifest_path.read_text(encoding="utf-8"))
        self._modules = document.get("modules", {})
        self._include_root = str(document.get("headers_root", "include"))

    def select(self, requested: tuple[str, ...] | list[str]) -> ModuleSelection:
        closure: set[str] = set()
        visiting: set[str] = set()

        def visit(name: str) -> None:
            if name in closure:
                return
            if name in visiting:
                raise ModuleSelectionError(f"cyclic UEF module dependency at {name}")
            record = self._modules.get(name)
            if record is None:
                raise ModuleSelectionError(f"UEF module '{name}' is not present in uef_modules.json")
            visiting.add(name)
            for dependency in record.get("dependencies", []):
                visit(dependency)
            visiting.remove(name)
            closure.add(name)

        requested = tuple(str(name).casefold() for name in requested)
        for name in requested:
            visit(name)
        sources: list[str] = []
        includes: list[str] = []
        defines: list[str] = []
        capabilities: list[str] = []
        external: list[str] = []
        for name in sorted(closure):
            record = self._modules[name]
            sources.extend(record.get("sources", []))
            includes.extend(record.get("include_directories", [self._include_root]))
            defines.extend(record.get("defines", []))
            capabilities.extend(record.get("required_capabilities",
                                           record.get("requires_cap", [])))
            external.extend(record.get("external_dependencies", []))
        return ModuleSelection(tuple(sorted(closure)), tuple(dict.fromkeys(sources)),
                               tuple(dict.fromkeys(includes)), tuple(dict.fromkeys(defines)),
                               tuple(dict.fromkeys(capabilities)), tuple(dict.fromkeys(external)))
