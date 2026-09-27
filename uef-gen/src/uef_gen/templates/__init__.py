"""Self-registering template sets; add existing algorithm templates here."""
from uef_gen.templates.registry import GeneratedFile, TemplateSet, UnknownTemplateSetError
from uef_gen.templates.project_scaffold import render_project_scaffold
from uef_gen.templates.uef_library import UefTemplateError, render_uef_template

__all__ = ["GeneratedFile", "TemplateSet", "UnknownTemplateSetError",
           "UefTemplateError", "render_project_scaffold", "render_uef_template"]
