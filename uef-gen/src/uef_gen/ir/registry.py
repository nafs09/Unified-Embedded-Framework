from __future__ import annotations

# Legacy extension registry: algorithm builders currently live on the host
# side, but the UEF ownership migration is expected to replace this contract.

from abc import ABC, abstractmethod
from typing import Any, ClassVar


class UnknownNodeTypeError(ValueError):
    pass


class NodeIRBuilder(ABC):
    _registry: ClassVar[dict[tuple[str, str], type["NodeIRBuilder"]]] = {}

    def __init_subclass__(cls, *, node_type: str = "", node_subtype: str = "*", **kwargs: Any):
        super().__init_subclass__(**kwargs)
        if node_type:
            key = (node_type.upper(), node_subtype.upper())
            existing = NodeIRBuilder._registry.get(key)
            if existing is not None and existing is not cls:
                raise TypeError(f"duplicate NodeIRBuilder registration: {key}")
            NodeIRBuilder._registry[key] = cls

    @classmethod
    @abstractmethod
    def build(cls, node: dict[str, Any]) -> Any:
        raise NotImplementedError


def build_node_ir(node: dict[str, Any]) -> Any:
    node_type = str(node.get("type", "")).upper()
    subtype = str(node.get("subtype", "*")).upper()
    key = (node_type, subtype)
    builder = NodeIRBuilder._registry.get(key) or NodeIRBuilder._registry.get((node_type, "*"))
    if builder is None:
        raise UnknownNodeTypeError(f"no AlgorithmIR builder registered for node type {node_type!r}, subtype {subtype!r}")
    return builder.build(node)


def registered_builders() -> dict[tuple[str, str], type[NodeIRBuilder]]:
    return dict(NodeIRBuilder._registry)
