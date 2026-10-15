"""Public compiler API for declarative multi-target `.uefs` projects."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from uef_gen.uetd_adapter.locator import UETDLocator
from uef_gen.language.frontends.project.compiler import CompilationResult, compile_project_file
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic, source_error
from uef_gen.language.frontends.project.lexer import Lexer
from uef_gen.language.frontends.system.ast import SystemDocument
from uef_gen.language.frontends.system.lowering import SystemBuildPlan, lower_system_document
from uef_gen.language.frontends.system.parser import SystemParser
from uef_gen.language.frontends.system.validation import validate_system
from uef_gen.language.serialization import to_json_value


@dataclass
class SystemCompilationResult:
    """Aggregate diagnostics and per-node compilation/build planning data."""

    source_path: Path | None
    nodes: dict[str, CompilationResult] = field(default_factory=dict)
    links: dict[str, Any] = field(default_factory=dict)
    build_plan: SystemBuildPlan | None = None
    diagnostics: list[LanguageDiagnostic] = field(default_factory=list)
    source_map: dict[str, Any] = field(default_factory=dict)

    @property
    def ok(self) -> bool:
        """True only after every requested node/link is resolved and valid."""
        return self.build_plan is not None and not any(d.severity == "error" for d in self.diagnostics)

    def to_dict(self) -> dict[str, Any]:
        """Return JSON-compatible diagnostics and deterministic plan summary."""
        plan = self.build_plan
        return {
            "ok": self.ok,
            "source": str(self.source_path) if self.source_path else None,
            "nodes": {
                name: {
                    "ok": result.ok,
                    "target": result.application_spec.target_id if result.application_spec else None,
                    "diagnostics": [item.to_dict() for item in result.diagnostics],
                }
                for name, result in self.nodes.items()
            },
            "links": [link.name for link in plan.links] if plan else [],
            "build_plan": None if plan is None else {
                "system_name": plan.system_name,
                "nodes": [node.name for node in plan.nodes],
                "links": [link.name for link in plan.links],
            },
            "backend_input": None if plan is None else to_json_value({
                "kind": "multi_target_system",
                "schema_version": "1.0",
                "system_plan": plan,
                "source_map": self.source_map,
            }),
            "diagnostics": [item.to_dict() for item in self.diagnostics],
        }


def compile_system_file(path: Path, *, uetd_root: Path | None = None) -> SystemCompilationResult:
    """Read a saved `.uefs` source and run the common system compiler."""
    source_path = path.expanduser().resolve()
    try:
        source = source_path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError) as exc:
        return SystemCompilationResult(source_path, diagnostics=[source_error("system_source_read_error", f"Cannot read system source: {exc}")])
    return compile_system_text(source, source_path=source_path, uetd_root=uetd_root)


def compile_system_text(
    source: str,
    *,
    source_path: Path | None = None,
    uetd_root: Path | None = None,
) -> SystemCompilationResult:
    """Lex, parse, verify, compile node projects, and form an aggregate plan."""
    source_path = source_path.expanduser().resolve() if source_path else None
    tokens, lexical_diagnostics = Lexer(source, source_path).tokenize()
    document, parse_diagnostics = SystemParser(tokens, source_path).parse()
    diagnostics = [*lexical_diagnostics, *parse_diagnostics]
    if document is None or any(item.severity == "error" for item in diagnostics):
        return SystemCompilationResult(source_path, diagnostics=diagnostics)
    try:
        root = uetd_root.expanduser().resolve() if uetd_root else UETDLocator(source_path.parent if source_path else Path.cwd()).locate()
    except (OSError, ValueError) as exc:
        diagnostics.append(source_error("system_uetd_unavailable", str(exc), document.span))
        return SystemCompilationResult(source_path, diagnostics=diagnostics)
    result = compile_system_document(document, uetd_root=root, source_path=source_path)
    result.diagnostics[:0] = diagnostics
    return result


def compile_system_document(
    document: SystemDocument,
    *,
    uetd_root: Path,
    source_path: Path | None = None,
) -> SystemCompilationResult:
    """Compile each declared node and lower only a wholly valid system."""
    path = source_path or document.source_path
    result = SystemCompilationResult(path)
    root = uetd_root.expanduser().resolve()
    result.diagnostics.extend(validate_system(document, uetd_root=root))
    if any(item.severity == "error" for item in result.diagnostics):
        return result

    system_root = path.parent if path else Path.cwd()
    for node in document.nodes:
        project_path = (system_root / node.project_path).resolve(strict=False)
        node_result = compile_project_file(project_path, uetd_root=root)
        result.nodes[node.name] = node_result
        if not node_result.ok:
            result.diagnostics.extend(node_result.diagnostics)

    if any(item.severity == "error" for item in result.diagnostics):
        return result
    plan, lowering_diagnostics = lower_system_document(document, result.nodes, source_path=path)
    result.diagnostics.extend(lowering_diagnostics)
    result.build_plan = plan if not any(item.severity == "error" for item in result.diagnostics) else None
    if plan:
        result.links = {item.name: item for item in plan.links}
        result.source_map = dict(plan.source_map)
    return result
