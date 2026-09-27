"""Render a reviewed Jinja template shipped in the verified UEF snapshot."""

from __future__ import annotations

from pathlib import Path
from typing import Any


class UefTemplateError(ValueError):
    """Raised when a UEF template is missing, unsafe, or cannot be rendered."""


def render_uef_template(template_root: Path, template_name: str,
                        values: dict[str, Any]) -> str:
    """Render one relative path beneath ``templates/ucon``.

    Template names are relative to the UCON catalogue. Resolving the candidate
    before loading prevents an extension from escaping the content-verified
    snapshot with ``..`` path segments.
    """
    root = template_root.resolve()
    candidate = (root / template_name).resolve()
    try:
        candidate.relative_to(root)
    except ValueError as exc:
        raise UefTemplateError(f"template path escapes UEF catalogue: {template_name}") from exc
    if candidate.suffix != ".tmpl" or not candidate.is_file():
        raise UefTemplateError(f"UEF template is missing: {template_name}")

    try:
        from jinja2 import Environment, FileSystemLoader, StrictUndefined
    except ImportError as exc:
        raise UefTemplateError(
            "Jinja2 is required to render UEF template files; install uef-gen[templates]"
        ) from exc

    environment = Environment(
        loader=FileSystemLoader(str(root)),
        autoescape=False,
        undefined=StrictUndefined,
        keep_trailing_newline=True,
        trim_blocks=True,
        lstrip_blocks=True,
    )
    try:
        return environment.get_template(template_name.replace("\\", "/")).render(values)
    except Exception as exc:
        raise UefTemplateError(f"could not render UEF template {template_name}: {exc}") from exc
