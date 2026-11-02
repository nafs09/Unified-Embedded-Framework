"""Human-readable CLI output and the dependency-light source formatter route."""

from __future__ import annotations

import sys
from pathlib import Path

from uef_gen.language.frontends.project.source_formatting import format_source


def format_response(response: dict, *, command: str, source_path: Path | None = None) -> str:
    diagnostics = response.get("diagnostics", [])
    errors = sum(item.get("severity") == "error" for item in diagnostics)
    warnings = sum(item.get("severity") == "warning" for item in diagnostics)
    status = "PASSED" if response.get("ok") else "FAILED"
    label = response.get("source") or source_path or ""
    lines = [f"{command.upper()} {status}  {label}", f"{errors} error(s), {warnings} warning(s)"]
    sources: dict[str, list[str]] = {}
    for item in diagnostics:
        raw_location = item.get("location", {})
        location = raw_location if isinstance(raw_location, dict) else {}
        path = location.get("path")
        line = location.get("line")
        column = location.get("column", 1)
        prefix = f"{path or label}:{line}:{column}: " if line else ""
        lines.extend(("", f"{prefix}{item.get('severity', 'error').upper()} [{item.get('code', 'unknown')}]: {item.get('message', '')}"))
        if isinstance(raw_location, str):
            lines.append(f"  field: {raw_location}")
        if path and isinstance(line, int) and line > 0:
            if path not in sources:
                try:
                    sources[path] = Path(path).read_text(encoding="utf-8-sig").splitlines()
                except (OSError, UnicodeError):
                    sources[path] = []
            source_lines = sources[path]
            if line <= len(source_lines):
                text = source_lines[line - 1]
                start = len(text[:max(0, column - 1)].expandtabs(4))
                end_column = location.get("end_column", column + 1) if location.get("end_line", line) == line else len(text) + 1
                end = len(text[:max(0, end_column - 1)].expandtabs(4))
                lines.extend((f"  {text.expandtabs(4)}", f"  {' ' * start}{'^' * max(1, end - start)}"))
        for related in item.get("related", []):
            lines.append(f"  related: {related.get('path')}:{related.get('line')}:{related.get('column')}: {related.get('message', '')}")
    if response.get("ok") and (response.get("application") is not None or response.get("build_plan") is not None):
        lines.extend(("", "Frontend validation passed. Firmware emission, build, and flash remain unavailable."))
    return "\n".join(lines)


def run_format_command(args) -> int:
    path = args.configuration.expanduser().resolve()
    try:
        if path.suffix.casefold() not in {".uef", ".uefs"}:
            raise ValueError("format supports .uef and .uefs source files")
        with path.open(encoding="utf-8", newline="") as handle:
            source = handle.read()
        bom = "\ufeff" if source.startswith("\ufeff") else ""
        output = bom + format_source(source[len(bom):], tab_size=args.indent, insert_spaces=not args.tabs,
                                     insert_final_newline=True, trim_final_newlines=True)
        if args.check:
            print(f"{'Needs formatting' if output != source else 'Formatted'}: {path}")
            return 1 if output != source else 0
        if args.write:
            if output != source:
                with path.open("w", encoding="utf-8", newline="") as handle:
                    handle.write(output)
            print(f"Formatted: {path}")
        else:
            # Text stdout can translate LF on Windows; avoid doubling source CRLF.
            if hasattr(sys.stdout, "buffer"):
                sys.stdout.buffer.write(output.encode("utf-8"))
            else:
                sys.stdout.write(output)
        return 0
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FORMAT FAILED: {exc}", file=sys.stderr)
        return 2
