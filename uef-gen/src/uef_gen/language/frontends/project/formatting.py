"""Render source diagnostics for terminals without changing compiler spans."""

from __future__ import annotations
from collections.abc import Iterable
from pathlib import Path
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic

def format_diagnostic(diagnostic: LanguageDiagnostic, source: str | None = None, *, tab_width: int = 4) -> str:
    """Render one diagnostic and, when source is supplied, its underlined line."""
    prefix = ""
    span = diagnostic.span
    if span is not None:
        path = str(span.path) if span.path else "<memory>"
        prefix = f"{path}:{span.start.line}:{span.start.column}: "
    lines = source.splitlines() if source is not None else []
    output = [f"{prefix}{diagnostic.severity.upper()} [{diagnostic.code}]: {diagnostic.message}"]
    if span is not None and 1 <= span.start.line <= len(lines):
        line = lines[span.start.line - 1]
        expanded = line.expandtabs(tab_width)
        start = _expanded_column(line, span.start.column, tab_width)
        if span.start.line == span.end.line:
            end = _expanded_column(line, span.end.column, tab_width)
            width = max(1, end - start)
        else:
            width = max(1, len(expanded) - start)
        output.extend((f"  {expanded}", f"  {' ' * start}{'^' * width}"))
    for message, related_span in diagnostic.related:
        related_path = str(related_span.path) if related_span.path else "<memory>"
        output.append(f"  related: {related_path}:{related_span.start.line}:{related_span.start.column}: {message}")
    return "\n".join(output)

def format_diagnostics(diagnostics: Iterable[LanguageDiagnostic], source: str | None = None) -> str:
    """Render compiler diagnostics in their existing deterministic order."""
    return "\n".join(format_diagnostic(item, source) for item in diagnostics)

def format_diagnostic_file(diagnostics: Iterable[LanguageDiagnostic], path: Path) -> str:
    """Load source text once and render diagnostics against their source file."""
    try:
        source = path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError):
        source = None
    return format_diagnostics(diagnostics, source)

def _expanded_column(line: str, one_based_column: int, tab_width: int) -> int:
    prefix = line[:max(0, one_based_column - 1)]
    return len(prefix.expandtabs(tab_width))
