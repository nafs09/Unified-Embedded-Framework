from __future__ import annotations

# Algorithm-specific TemplateSet dispatch is transitional. Reusable algorithm
# templates belong to UEF; keep this registry only as a bridge until the UEF
# generation contract replaces it.

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Any, ClassVar

from uef_gen.ir.algorithms import algorithm_type_for


@dataclass(frozen=True)
class GeneratedFile:
    path: str
    content: str


class UnknownTemplateSetError(ValueError):
    pass


class TemplateSet(ABC):
    _registry: ClassVar[dict[str, type["TemplateSet"]]] = {}

    def __init_subclass__(cls, *, algorithm_type: str = "", **kwargs: Any):
        super().__init_subclass__(**kwargs)
        if algorithm_type:
            key = algorithm_type.upper()
            previous = TemplateSet._registry.get(key)
            if previous is not None and previous is not cls:
                raise TypeError(f"duplicate TemplateSet registration: {key}")
            TemplateSet._registry[key] = cls

    @classmethod
    @abstractmethod
    def render(cls, algorithm_ir: Any, context: dict[str, Any]) -> tuple[GeneratedFile, ...]:
        raise NotImplementedError


def render_algorithm(algorithm_ir: Any, context: dict[str, Any]) -> tuple[GeneratedFile, ...]:
    kind = algorithm_type_for(algorithm_ir)
    template = TemplateSet._registry.get(kind)
    if template is None:
        raise UnknownTemplateSetError(f"no template set registered for algorithm type {kind!r}")
    return template.render(algorithm_ir, context)


def registered_template_sets() -> dict[str, type[TemplateSet]]:
    return dict(TemplateSet._registry)
