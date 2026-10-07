"""Transport type for files rendered from project scaffolds or UEF templates."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class GeneratedFile:
    """One generated relative path and its UTF-8 content."""

    path: str
    content: str
