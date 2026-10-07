from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
from uef_gen.registry import UEFRegistryError, load_module_manifest

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
    bundled_dependency_names: tuple[str, ...]

class ModuleSelector:
    def __init__(self, snapshot_root: Path):
        try:
            document = load_module_manifest(snapshot_root)
        except UEFRegistryError as exc:
            raise ModuleSelectionError(str(exc)) from exc
        self._modules = document.get("modules", {})
        self._bundled_dependencies = document.get("bundled_dependencies", {})
        self._include_root = str(document.get("headers_root", "include"))
        if not isinstance(self._modules, dict) or not isinstance(self._bundled_dependencies, dict):
            raise ModuleSelectionError("UEF module and bundled dependency manifests must be objects")

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
                raise ModuleSelectionError(f"UEF module '{name}' is not present in the canonical module registry")
            if record.get("scaffold_only") is True or record.get("available_for_generation") is False:
                algorithm = record.get("algorithm")
                priority = record.get("priority")
                details = []
                if isinstance(algorithm, str) and algorithm:
                    details.append(f"algorithm {algorithm}")
                if isinstance(priority, str) and priority:
                    details.append(f"roadmap priority {priority}")
                suffix = f" ({', '.join(details)})" if details else ""
                raise ModuleSelectionError(
                    f"UEF module '{name}'{suffix} is a fail-closed development scaffold "
                    "and cannot be selected for generated firmware"
                )
            dependencies = record.get("dependencies", [])
            sources = record.get("sources", [])
            headers = record.get("headers")
            if not isinstance(dependencies, list) or any(
                not isinstance(item, str) or not item.strip() for item in dependencies
            ):
                raise ModuleSelectionError(f"UEF module '{name}' has invalid dependencies")
            if not isinstance(sources, list) or any(
                not isinstance(item, str) or not item.strip() for item in sources
            ):
                raise ModuleSelectionError(f"UEF module '{name}' has invalid sources")
            if not isinstance(headers, list) or any(
                not isinstance(item, str) or not item.strip() for item in headers
            ) or (sources and not headers):
                raise ModuleSelectionError(
                    f"UEF module '{name}' must declare its public header closure explicitly"
                )
            visiting.add(name)
            for dependency in dependencies:
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
        bundled: list[str] = []
        for name in sorted(closure):
            record = self._modules[name]
            sources.extend(record.get("sources", []))
            includes.extend(record.get("include_directories", [self._include_root]))
            defines.extend(record.get("defines", []))
            capabilities.extend(record.get("required_capabilities",
                                           record.get("requires_cap", [])))
            external.extend(record.get("external_dependencies", []))
            for dependency_name in record.get("bundled_dependencies", []):
                if dependency_name not in self._bundled_dependencies:
                    raise ModuleSelectionError(
                        f"UEF module '{name}' references unknown bundled dependency '{dependency_name}'"
                    )
                bundled.append(dependency_name)
        return ModuleSelection(tuple(sorted(closure)), tuple(dict.fromkeys(sources)),
                               tuple(dict.fromkeys(includes)), tuple(dict.fromkeys(defines)),
                               tuple(dict.fromkeys(capabilities)), tuple(dict.fromkeys(external)),
                               tuple(dict.fromkeys(bundled)))
