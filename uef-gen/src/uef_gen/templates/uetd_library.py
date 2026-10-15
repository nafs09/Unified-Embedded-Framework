"""Render a reviewed Jinja template directly from the located UETD checkout."""

from __future__ import annotations
from pathlib import Path
from typing import Any

class UETDTemplateError(ValueError):
    """Raised when a UETD template is missing, unsafe, or cannot be rendered."""

def render_uetd_template(template_root: Path, template_name: str, values: dict[str, Any]) -> str:
    """Render one relative path beneath the UETD ``templates`` directory.

    Template names are relative to UETD's template root. Resolving the candidate
    before loading prevents a UETD manifest entry from escaping that tree with
    ``..`` path segments.
    """
    root = template_root.resolve()
    candidate = (root / template_name).resolve()
    try:
        candidate.relative_to(root)
    except ValueError as exc:
        raise UETDTemplateError(f"template path escapes UETD catalogue: {template_name}") from exc
    if candidate.suffix != ".tmpl" or not candidate.is_file():
        raise UETDTemplateError(f"UETD template is missing: {template_name}")
    try:
        from jinja2 import Environment, FileSystemLoader, StrictUndefined
    except ImportError as exc:
        raise UETDTemplateError("Jinja2 is required to render UETD template files; install uef-gen[templates]") from exc
    environment = Environment(
        loader=FileSystemLoader(str(root)), autoescape=False, undefined=StrictUndefined, keep_trailing_newline=True, trim_blocks=True, lstrip_blocks=True
    )
    try:
        return environment.get_template(template_name.replace("\\", "/")).render(values)
    except Exception as exc:
        raise UETDTemplateError(f"could not render UETD template {template_name}: {exc}") from exc
