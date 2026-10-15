"""Public orchestration API for standalone ``.uef`` compilation."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from uef_gen.uetd_adapter.locator import UETDLocator, UETDNotFoundError, UETDVersionIncompatible
from uef_gen.language.frontends.project.ast import ProjectDocument, SourceMap
from uef_gen.language.frontends.project.contract_imports import load_contract_imports
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic, source_error
from uef_gen.language.frontends.project.lexer import Lexer
from uef_gen.language.frontends.project.lowering import lower_project
from uef_gen.language.frontends.project.parser import Parser
from uef_gen.language.frontends.project.registry_contracts import load_authoring_catalogue
from uef_gen.language.serialization import to_json_value
from uef_gen.language.frontends.project.validation import validate_project
from uef_gen.language.registry.loader import load_language_registry
from uef_gen.uetd_adapter.registry import UETDRegistryError


@dataclass
class CompilationResult:
    """Standalone inputs plus diagnostics and source mappings."""

    source_path: Path | None
    hardware_configuration: dict[str, Any] | None = None
    application_spec: Any | None = None
    diagnostics: list[LanguageDiagnostic] = field(default_factory=list)
    source_map: SourceMap = field(default_factory=SourceMap)

    @property
    def ok(self) -> bool:
        return self.hardware_configuration is not None and self.application_spec is not None and not any(d.severity == "error" for d in self.diagnostics)

    def to_dict(self) -> dict[str, Any]:
        app = self.application_spec
        summary = None if app is None else {
            "project_name": app.project_name,
            "project_parameters": [item.name for item in app.project_parameters],
            "components": [item.name for item in app.components],
            "records": [item.name for item in app.records],
            "enums": [{"name": item.name, "storage_type": item.storage_type} for item in app.enums.values()],
            "contract_imports": [item.source_path for item in app.contract_imports],
            "variants": [item.definition.name for item in app.variants],
            "flags_types": [item.definition.name for item in app.flags_types],
            "fixed_point_types": [item.definition.name for item in app.fixed_point_types],
            "hybrid_algorithms": [item.name for item in app.hybrid_algorithms],
            "models": [item.name for item in app.models],
            "custom_algorithms": [item.name for item in app.custom_algorithms],
            "subsystems": [item.name for item in app.subsystems],
            "project_routines": [item.qualified_name for item in app.project_routines],
            "operatingsystem": {"name": app.operating_system.name, "state_machines": [x.name for x in app.operating_system.state_machines], "tasks": [x.name for x in app.operating_system.tasks], "tasksets": [x.name for x in app.operating_system.tasksets]},
        }
        backend_input = None
        if app is not None and self.hardware_configuration is not None:
            backend_input = to_json_value({
                "kind": "standalone_application",
                "schema_version": "1.0",
                "project_source": self.source_path,
                "uetd_root": self.hardware_configuration.get("uetd", {}).get("path"),
                "hardware_configuration": self.hardware_configuration,
                "application": app,
                "source_map": self.source_map.entries,
            })
        return {
            "ok": self.ok,
            "source": str(self.source_path) if self.source_path else None,
            "hardware_configuration": self.hardware_configuration,
            "application": to_json_value(app) if app is not None else None,
            "application_summary": summary,
            "backend_input": backend_input,
            "diagnostics": [d.to_dict() for d in self.diagnostics],
        }


def compile_project_file(path: Path, *, uetd_root: Path | None = None) -> CompilationResult:
    """Read one project file and pass it through the shared text compiler."""
    source_path = path.expanduser().resolve()
    try:
        source = source_path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError) as exc:
        return CompilationResult(source_path, diagnostics=[source_error("project_source_read_error", f"Cannot read project source: {exc}")])
    return compile_project_text(source, source_path=source_path, uetd_root=uetd_root)


def compile_project_text(source: str, *, source_path: Path | None = None, uetd_root: Path | None = None) -> CompilationResult:
    """Run lexer, parser, safe imports, UETD lookup, verification, and lowering."""
    lexer = Lexer(source, source_path)
    tokens, lexical_diagnostics = lexer.tokenize()
    document, parse_diagnostics = Parser(tokens, source_path).parse()
    diagnostics = [*lexical_diagnostics, *parse_diagnostics]
    if document is None or any(item.severity == "error" for item in diagnostics):
        return CompilationResult(source_path, diagnostics=diagnostics)
    result = compile_project_document(document, uetd_root=uetd_root, source_path=source_path)
    result.diagnostics[:0] = diagnostics
    return result


def compile_project_document(document: ProjectDocument, *, uetd_root: Path | None = None, source_path: Path | None = None) -> CompilationResult:
    """Compile an already parsed project AST without re-lexing it."""
    path = source_path or document.source_path
    result = CompilationResult(path)
    try:
        root = uetd_root.expanduser().resolve() if uetd_root else UETDLocator(path.parent if path else Path.cwd()).locate()
        # Loading this registry validates that the parser/lowerer use the
        # syntax version shipped with this uef-gen installation.
        load_language_registry()
        catalogue = load_authoring_catalogue(root)
        imported_contracts = ()
        if document.contract_imports:
            if path is None:
                result.diagnostics.append(source_error("contract_import_requires_file", "A saved .uef file is required to resolve contract imports."))
                return result
            imported_contracts, import_diagnostics = load_contract_imports(document.contract_imports, source_path=path)
            result.diagnostics.extend(import_diagnostics)
        semantic_diagnostics, source_map, resolved_math_bindings, resolved_math_shape_bindings = validate_project(
            document, catalogue, uetd_root=root, imported_contracts=imported_contracts,
        )
        result.diagnostics.extend(semantic_diagnostics)
        result.source_map = source_map
        if any(item.severity == "error" for item in result.diagnostics):
            return result
        hardware, application, lowered_map = lower_project(
            document,
            catalogue,
            uetd_root=root,
            imported_contracts=imported_contracts,
            resolved_math_bindings=resolved_math_bindings,
            resolved_math_shape_bindings=resolved_math_shape_bindings,
        )
        result.hardware_configuration = hardware
        result.application_spec = application
        result.source_map.entries.update(lowered_map.entries)
    except (UETDNotFoundError, UETDVersionIncompatible, UETDRegistryError, ValueError, OSError) as exc:
        result.diagnostics.append(source_error("project_compile_error", str(exc)))
    return result
