"""Validate generated project paths, source lists, encodings, and includes."""

from __future__ import annotations
import re
from pathlib import Path
from uef_gen.diagnostics import Diagnostic, error

_SOURCE_SUFFIXES = {".c", ".h", ".cc", ".cpp", ".hpp"}
_INCLUDE_PATTERN = re.compile(r"^\s*#\s*include\s*([<\"])([^>\"]+)[>\"]", re.MULTILINE)

def validate_generated_project(root: Path, files: list[str], sources: list[str], *, include_directories: list[str] | None = None) -> list[Diagnostic]:
    """Return diagnostics for missing/escaping artifacts and unresolved local includes."""
    diagnostics: list[Diagnostic] = []
    resolved_root = root.resolve()
    safe_files: list[str] = []
    for relative in dict.fromkeys([*files, *sources]):
        path = (root / relative).resolve()
        try:
            path.relative_to(resolved_root)
        except ValueError:
            diagnostics.append(error("generated_path_escape", f"Generated path escapes project root: {relative}"))
            continue
        if not path.is_file():
            diagnostics.append(error("generated_file_missing", f"Expected generated file was not written: {relative}"))
        else:
            safe_files.append(relative)
    for relative in sources:
        if not (root / relative).is_file():
            diagnostics.append(error("source_list_entry_missing", f"Source list references missing file: {relative}"))
    include_dirs = []
    for directory in include_directories if include_directories is not None else [".", "include", "uetd/include", "uos"]:
        candidate = (root / directory).resolve()
        if not candidate.is_relative_to(resolved_root):
            diagnostics.append(error("generated_include_escape", f"Include directory escapes project root: {directory}"))
        else:
            include_dirs.append(candidate)
    for relative in safe_files:
        source = root / relative
        if source.suffix.casefold() not in _SOURCE_SUFFIXES or not source.is_file():
            continue
        try:
            text = source.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            diagnostics.append(error("generated_source_encoding", f"Generated source is not UTF-8 text: {relative}"))
            continue
        for delimiter, include in _INCLUDE_PATTERN.findall(text):
            candidate = Path(include)
            if candidate.is_absolute() or ".." in candidate.parts:
                diagnostics.append(error("generated_include_escape", f"Unsafe include path {include!r} in {relative}"))
                continue
            local = source.parent / candidate
            known = local.is_file() or any((directory / candidate).is_file() for directory in include_dirs)
            # Angle-bracket headers may come from a declared external dependency.
            if not known and (delimiter == '"' or include.startswith("uetd/") or include.startswith("generated/")):
                diagnostics.append(error("generated_include_missing", f"Cannot resolve project include {include!r} from {relative}"))
    return diagnostics
