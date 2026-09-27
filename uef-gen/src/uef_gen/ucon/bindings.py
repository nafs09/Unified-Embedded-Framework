"""Transitional project-to-UPAL bindings from the earlier V8 UCON contract.

Do not treat this module as the long-term owner of UCON signal semantics. UEF's
future contract should define the reusable signal and algorithm descriptions;
uef-gen will consume them while assembling a target project.
"""

from __future__ import annotations

import math
import re
from dataclasses import dataclass
from typing import Any, Literal

from uef_gen.diagnostics import Diagnostic, error
from uef_gen.resolver.models import ResolvedProject
from uef_gen.ucon.api_database import UEFApiDatabase


@dataclass(frozen=True)
class SignalSource:
    peripheral: str
    index: int | None = None
    output_id: str | None = None
    transform: str | None = None


@dataclass(frozen=True)
class UconSignal:
    name: str
    direction: Literal["input", "output"]
    units: str
    type: str
    range_min: float | None
    range_max: float | None
    binding: SignalSource


@dataclass(frozen=True)
class SignalBinding:
    """Resolved form after target resource and UEF API lookup."""

    signal: str
    direction: Literal["input", "output"]
    peripheral_key: str
    peripheral_type: str
    resource_type: str
    index: int | None
    output_id: str | None
    transform_expr: str | None
    upal_handle: str
    upal_function: str


_C_IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
_ALLOWED_SIGNAL_TYPES = {"float32", "q31"}


def _finite_number(value: Any) -> bool:
    return isinstance(value, (int, float)) and not isinstance(value, bool) and math.isfinite(value)


def _as_peripheral_map(config: dict[str, Any], resolved: ResolvedProject) -> dict[str, Any]:
    by_instance = {item.instance: item for item in resolved.peripherals}
    by_key = dict(by_instance)
    for item in resolved.peripherals:
        if item.key:
            by_key[item.key] = item
    raw = config.get("peripherals", [])
    if isinstance(raw, dict):
        for key, request in raw.items():
            if isinstance(request, dict):
                physical = by_instance.get(str(request.get("instance", key)))
                if physical is not None:
                    by_key[str(key)] = physical
    elif isinstance(raw, list):
        for request in raw:
            if isinstance(request, dict):
                key = str(request.get("key", request.get("name", request.get("instance", ""))))
                physical = by_instance.get(str(request.get("instance", "")))
                if physical is not None:
                    by_key[key] = physical
    return {key: value for key, value in by_key.items() if value is not None}


def _read_signal(
    raw: Any, direction: Literal["input", "output"], location: str, diagnostics: list[Diagnostic]
) -> UconSignal | None:
    if not isinstance(raw, dict):
        diagnostics.append(error("ucon_signal_invalid", "Signal declaration must be an object", location))
        return None
    name, units, signal_type = raw.get("name"), raw.get("units"), raw.get("type")
    if not isinstance(name, str) or not name.strip():
        diagnostics.append(error("ucon_signal_name", "Signal name must be a non-empty string", f"{location}.name"))
        return None
    if not isinstance(units, str) or not units:
        diagnostics.append(error("ucon_signal_units", "Signal units must be a non-empty string", f"{location}.units"))
    if signal_type not in _ALLOWED_SIGNAL_TYPES:
        diagnostics.append(error("ucon_signal_type", "Signal type must be float32 or q31", f"{location}.type"))

    binding_field = "source" if direction == "input" else "sink"
    raw_binding = raw.get(binding_field)
    if not isinstance(raw_binding, dict) or not isinstance(raw_binding.get("peripheral"), str) or not raw_binding["peripheral"]:
        diagnostics.append(error("ucon_signal_binding", f"{binding_field}.peripheral is required", f"{location}.{binding_field}"))
        return None
    index = raw_binding.get("index")
    if index is not None and (not isinstance(index, int) or isinstance(index, bool) or index < 0):
        diagnostics.append(error("ucon_signal_index", "Signal index must be a non-negative integer", f"{location}.{binding_field}.index"))
        index = None
    output_id = raw_binding.get("output_id", raw_binding.get("output"))
    if output_id is not None and (not isinstance(output_id, str) or not output_id):
        diagnostics.append(error("ucon_signal_output_id", "Signal output identifier must be a non-empty string", f"{location}.{binding_field}.output"))
        output_id = None
    transform = raw_binding.get("transform")
    if transform is not None and (not isinstance(transform, str) or not transform):
        diagnostics.append(error("ucon_signal_transform", "Transform must name a configured transform", f"{location}.{binding_field}.transform"))
        transform = None
    if direction == "output" and output_id is None:
        diagnostics.append(error("ucon_signal_output_id", "Output binding requires sink.output", f"{location}.sink.output"))

    low = raw.get("range_min")
    high = raw.get("range_max")
    value_range = raw.get("range")
    if value_range is not None:
        if isinstance(value_range, list) and len(value_range) == 2:
            low, high = value_range
        else:
            diagnostics.append(error("ucon_signal_range", "range must contain exactly [minimum, maximum]", f"{location}.range"))
    if low is not None and not _finite_number(low):
        diagnostics.append(error("ucon_signal_range", "range minimum must be finite numeric data", f"{location}.range_min"))
        low = None
    if high is not None and not _finite_number(high):
        diagnostics.append(error("ucon_signal_range", "range maximum must be finite numeric data", f"{location}.range_max"))
        high = None
    if low is not None and high is not None and low >= high:
        diagnostics.append(error("ucon_signal_range", "range minimum must be less than maximum", location))

    return UconSignal(
        name=name,
        direction=direction,
        units=units if isinstance(units, str) else "",
        type=signal_type if isinstance(signal_type, str) else "",
        range_min=float(low) if low is not None else None,
        range_max=float(high) if high is not None else None,
        binding=SignalSource(
            peripheral=raw_binding["peripheral"],
            index=index,
            output_id=output_id,
            transform=transform,
        ),
    )


def resolve_signal_bindings(
    config: dict[str, Any],
    resolved: ResolvedProject,
    api_database: UEFApiDatabase,
) -> tuple[dict[str, tuple[SignalBinding, ...]], list[Diagnostic]]:
    """Resolve every explicitly named UCON source/sink before templates execute."""

    ucon = config.get("ucon")
    if ucon is None:
        return {}, []
    if not isinstance(ucon, dict):
        return {}, [error("ucon_config_invalid", "ucon must be an object", "ucon")]
    subsystems = ucon.get("subsystems", [])
    transforms = ucon.get("transforms", {})
    diagnostics: list[Diagnostic] = []
    if not isinstance(subsystems, list):
        return {}, [error("ucon_subsystems_invalid", "ucon.subsystems must be an array", "ucon.subsystems")]
    if not isinstance(transforms, dict):
        diagnostics.append(error("ucon_transforms_invalid", "ucon.transforms must map names to C expressions", "ucon.transforms"))
        transforms = {}
    clean_transforms: dict[str, str] = {}
    for key, expression in transforms.items():
        location = f"ucon.transforms.{key}"
        if not isinstance(key, str) or not _C_IDENTIFIER.fullmatch(key):
            diagnostics.append(error("ucon_transform_name", "Transform names must be C identifiers", location))
        elif not isinstance(expression, str) or not expression.strip() or any(
            token in expression for token in (";", "{", "}", "#", "\n", "\r")
        ):
            diagnostics.append(error("ucon_transform_expression", "Transform must be a single C expression without statements or preprocessor directives", location))
        else:
            clean_transforms[key] = expression.strip()

    peripherals = _as_peripheral_map(config, resolved)
    result: dict[str, tuple[SignalBinding, ...]] = {}
    seen_subsystems: set[str] = set()
    for subsystem_index, subsystem in enumerate(subsystems):
        location = f"ucon.subsystems[{subsystem_index}]"
        if not isinstance(subsystem, dict):
            diagnostics.append(error("ucon_subsystem_invalid", "Subsystem must be an object", location))
            continue
        name = subsystem.get("name")
        if not isinstance(name, str) or not _C_IDENTIFIER.fullmatch(name):
            diagnostics.append(error("ucon_subsystem_name", "Subsystem name must be a C identifier", f"{location}.name"))
            continue
        if name in seen_subsystems:
            diagnostics.append(error("ucon_subsystem_duplicate", f"Duplicate subsystem name {name}", f"{location}.name"))
            continue
        seen_subsystems.add(name)
        rate = subsystem.get("rate_hz")
        if not _finite_number(rate) or rate <= 0:
            diagnostics.append(error("ucon_rate_invalid", "rate_hz must be positive finite numeric data", f"{location}.rate_hz"))
        if not isinstance(subsystem.get("execution_context"), str) or not subsystem["execution_context"]:
            diagnostics.append(error("ucon_execution_context", "execution_context must be a non-empty string", f"{location}.execution_context"))
        signals: list[SignalBinding] = []
        names_by_direction: set[tuple[str, str]] = set()
        for direction, key in (("input", "inputs"), ("output", "outputs")):
            declarations = subsystem.get(key, [])
            if not isinstance(declarations, list):
                diagnostics.append(error("ucon_signal_list", f"{key} must be an array", f"{location}.{key}"))
                continue
            for signal_index, raw_signal in enumerate(declarations):
                signal_location = f"{location}.{key}[{signal_index}]"
                signal = _read_signal(raw_signal, direction, signal_location, diagnostics)
                if signal is None:
                    continue
                identity = (direction, signal.name)
                if identity in names_by_direction:
                    diagnostics.append(error("ucon_signal_duplicate", f"Duplicate {direction} signal {signal.name}", signal_location))
                    continue
                names_by_direction.add(identity)
                handle = signal.binding.peripheral
                if not _C_IDENTIFIER.fullmatch(handle):
                    diagnostics.append(error("ucon_peripheral_key", "Peripheral binding key must be a C identifier", f"{signal_location}.{direction}.peripheral"))
                    continue
                peripheral = peripherals.get(handle)
                if peripheral is None:
                    diagnostics.append(error("ucon_peripheral_unresolved", f"Peripheral key {handle!r} is not declared by the hardware configuration", signal_location))
                    continue
                transform_expression = None
                if signal.binding.transform is not None:
                    transform_expression = clean_transforms.get(signal.binding.transform)
                    if transform_expression is None:
                        diagnostics.append(error("ucon_transform_unresolved", f"Transform {signal.binding.transform!r} is not declared in ucon.transforms", signal_location))
                        continue
                operation, problem = api_database.resolve_signal_operation(peripheral.type_name, direction)
                if operation is None:
                    diagnostics.append(error("ucon_upal_operation_unavailable", problem or "No UPAL operation resolves this signal", signal_location))
                    continue
                resource_type = operation.resource_type or (
                    "dma_buffer" if direction == "input" and signal.binding.index is not None
                    else "register" if direction == "output"
                    else "callback"
                )
                signals.append(SignalBinding(
                    signal=signal.name,
                    direction=direction,
                    peripheral_key=handle,
                    peripheral_type=peripheral.type_name,
                    resource_type=resource_type,
                    index=signal.binding.index,
                    output_id=signal.binding.output_id,
                    transform_expr=transform_expression,
                    upal_handle=handle,
                    upal_function=operation.c_function,
                ))
        result[name] = tuple(signals)
    return result, diagnostics
