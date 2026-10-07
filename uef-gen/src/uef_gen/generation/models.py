from __future__ import annotations
from dataclasses import dataclass, field
from pathlib import Path
from uef_gen.diagnostics import Diagnostic

@dataclass
class GenerationResult:
    ok: bool
    output_directory: Path
    manifest: dict = field(default_factory=dict)
    diagnostics: list[Diagnostic] = field(default_factory=list)
