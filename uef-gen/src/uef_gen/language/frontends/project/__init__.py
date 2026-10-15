"""Declarative standalone UETD project source frontend.

This package parses `.uef` files and lowers them to hardware configuration and
the standalone application specification.
"""

from uef_gen.language.frontends.project.compiler import CompilationResult, compile_project_document, compile_project_file, compile_project_text

__all__ = ["CompilationResult", "compile_project_document", "compile_project_file", "compile_project_text"]
