"""Project scaffold and UETD-owned template integration."""

from uef_gen.templates.models import GeneratedFile
from uef_gen.templates.project_scaffold import render_project_scaffold
from uef_gen.templates.uetd_library import UETDTemplateError, render_uetd_template

__all__ = ["GeneratedFile", "UETDTemplateError", "render_project_scaffold", "render_uetd_template"]
