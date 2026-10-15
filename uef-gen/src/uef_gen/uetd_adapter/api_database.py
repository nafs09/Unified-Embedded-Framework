"""Read UPAL operation contracts from the separately maintained UETD checkout.

The catalogue is descriptive metadata owned by UETD. Looking up a declaration
does not implement the driver or establish that target behavior is verified.
"""

from __future__ import annotations
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Literal

class UETDApiError(ValueError):
    pass

@dataclass(frozen=True)
class ApiOperation:
    module: str
    operation: str
    c_function: str
    signature: str
    header: str
    resource_type: str

class UETDApiDatabase:
    """Validated view over api.json; no peripheral APIs are invented."""

    def __init__(self, document: dict[str, Any]):
        if not isinstance(document, dict):
            raise UETDApiError("UETD API document root must be an object")
        modules = document.get("upal_api")
        if not isinstance(modules, dict):
            raise UETDApiError("UETD API document must contain a upal_api object")
        self._document = document
        self._modules = modules

    @classmethod
    def from_file(cls, path: Path) -> "UETDApiDatabase":
        try:
            document = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            raise UETDApiError(f"cannot load UETD API database {path}: {exc}") from exc
        return cls(document)

    def resolve_signal_operation(self, peripheral_type: str, direction: Literal["input", "output"]) -> tuple[ApiOperation | None, str | None]:
        module_name = f"upal/{peripheral_type.casefold()}"
        module = self._modules.get(module_name)
        if not isinstance(module, dict):
            return None, f"UETD API has no {module_name} module"
        explicit = module.get("signal_bindings", {})
        binding = explicit.get(direction) if isinstance(explicit, dict) else None
        if binding is not None:
            if not isinstance(binding, dict) or not isinstance(binding.get("operation"), str):
                return None, f"{module_name}.signal_bindings.{direction} must name an operation"
            operation_name = binding["operation"]
            operation = self._operation(module_name, module, operation_name, binding)
            if operation is None:
                return None, f"{module_name} declares a missing {direction} operation {operation_name!r}"
            return operation, None
        return None, f"{module_name} does not declare signal_bindings.{direction} in UETD API metadata"

    @staticmethod
    def _operation(module_name: str, module: dict[str, Any], operation_name: str, binding: dict[str, Any]) -> ApiOperation | None:
        raw: Any = module.get(operation_name)
        if raw is None:
            raw = module.get(f"{operation_name}_function")
        if raw is None and operation_name.endswith("_function"):
            raw = module.get(operation_name.removesuffix("_function"))
        if isinstance(raw, str):
            function = raw
            signature = ""
            header = str(module.get("header", ""))
            resource_type = str(binding.get("resource_type", ""))
        elif isinstance(raw, dict):
            function = raw.get("c_function")
            signature = raw.get("signature", "")
            header = raw.get("header", module.get("header", ""))
            resource_type = str(binding.get("resource_type", raw.get("resource_type", "")))
        else:
            return None
        if not isinstance(function, str) or not function:
            return None
        if not isinstance(signature, str) or not isinstance(header, str):
            return None
        return ApiOperation(
            module=module_name, operation=operation_name, c_function=function, signature=signature, header=header, resource_type=resource_type
        )
