"""Source diagnostics and compiler results for the .uef frontend."""

from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
from typing import Any
from uef_gen.language.frontends.project.ast import SourceSpan

@dataclass(frozen=True)
class LanguageDiagnostic:
    """Stable source-language diagnostic suitable for CLI and editor clients."""

    severity: str
    code: str
    message: str
    span: SourceSpan | None = None
    related: tuple[tuple[str, SourceSpan], ...] = ()

    def to_dict(self) -> dict[str, Any]:
        """Serialize source coordinates without exposing Python-only objects."""
        result: dict[str, Any] = {"severity": self.severity, "code": self.code, "message": self.message}
        if self.span is not None:
            result["location"] = {
                "path": str(self.span.path) if self.span.path else None,
                "line": self.span.start.line,
                "column": self.span.start.column,
                "end_line": self.span.end.line,
                "end_column": self.span.end.column,
            }
        if self.related:
            result["related"] = [
                {"message": message, "path": str(span.path) if span.path else None, "line": span.start.line, "column": span.start.column}
                for message, span in self.related
            ]
        return result

def source_error(code: str, message: str, span: SourceSpan | None = None) -> LanguageDiagnostic:
    """Create an error that blocks lowering or generation."""
    return LanguageDiagnostic("error", code, message, span)

def source_warning(code: str, message: str, span: SourceSpan | None = None) -> LanguageDiagnostic:
    """Create a non-blocking source or readiness warning."""
    return LanguageDiagnostic("warning", code, message, span)

def span_for(path: Path | None, start: Any, end: Any) -> SourceSpan:
    """Build a span using the lexer's one-based code-point coordinates.

    LSP adapters convert these internal positions to the client's negotiated
    character encoding. The parser therefore remains independent of editors.
    """
    return SourceSpan(path, start, end)
