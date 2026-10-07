"""Normalize the documented YAML shape into the resolver's canonical model."""

from __future__ import annotations

from copy import deepcopy
from math import ceil
from typing import Any


def normalize_configuration(config: dict[str, Any]) -> dict[str, Any]:
    """Accept the spec's named maps and frequency-based fields without losing input data."""
    normalized = deepcopy(config)
    target = normalized.get("target", {})
    if not isinstance(target, dict):
        return normalized

    # The user-facing schema keeps RTOS settings next to the selected target;
    # the resolver consumes one normalized top-level RTOS object.
    if "rtos" not in normalized:
        rtos_name = target.get("rtos", "baremetal")
        freertos = target.get("freertos", {})
        if isinstance(freertos, dict) and freertos.get("tick_hz"):
            tick_hz = freertos.get("tick_hz")
            if isinstance(tick_hz, (int, float)) and tick_hz > 0:
                normalized["rtos"] = {
                    "name": rtos_name,
                    "tick_us": max(1, ceil(1_000_000 / tick_hz)),
                    "tick_hz": tick_hz,
                    "freertos": freertos,
                }
            else:
                normalized["rtos"] = {"name": rtos_name, "tick_us": 0, "freertos": freertos}
        else:
            normalized["rtos"] = rtos_name

    # The specification uses names as keys for both peripherals and tasks.
    peripherals = normalized.get("peripherals")
    if isinstance(peripherals, dict):
        normalized["peripherals"] = [
            _normalize_peripheral(name, value)
            for name, value in peripherals.items()
        ]

    tasks = normalized.get("tasks")
    if isinstance(tasks, dict):
        normalized_tasks: list[dict[str, Any]] = []
        for name, value in tasks.items():
            task = dict(value) if isinstance(value, dict) else {}
            task.setdefault("name", str(name))
            task.setdefault("context", task.get("execution_context", "task"))
            rate_hz = task.get("rate_hz")
            if "period_us" not in task and isinstance(rate_hz, (int, float)) and rate_hz > 0:
                task["period_us"] = max(1, round(1_000_000 / rate_hz))
            normalized_tasks.append(task)
        normalized["tasks"] = normalized_tasks
    elif isinstance(tasks, list):
        normalized["tasks"] = [
            {**task, "context": task.get("context", task.get("execution_context", "task"))}
            if isinstance(task, dict) else task
            for task in tasks
        ]

    return normalized


def _normalize_peripheral(name: str, value: Any) -> dict[str, Any]:
    """Convert the spec's `tx_pin`/`dma_tx` convenience fields to resolver inputs."""
    if not isinstance(value, dict):
        return {"_config_key": str(name), "instance": str(name)}

    peripheral = dict(value)
    peripheral.setdefault("_config_key", str(name))
    peripheral.setdefault("instance", str(name))

    pins = dict(peripheral.get("pins", {})) if isinstance(peripheral.get("pins"), dict) else {}
    dma = dict(peripheral.get("dma", {})) if isinstance(peripheral.get("dma"), dict) else {}
    for field, pin in tuple(peripheral.items()):
        if field.endswith("_pin") and isinstance(pin, str):
            signal = field[:-4].replace("_", "").upper()
            pins.setdefault(signal, pin)
        elif field.startswith("dma_") and isinstance(pin, bool):
            direction = field[4:].upper()
            if direction in {"TX", "RX", "TX1", "RX1"}:
                dma.setdefault(direction, pin)
    if pins:
        peripheral["pins"] = pins
    if dma:
        peripheral["dma"] = dma

    # Clock and peripheral-specific settings are written directly in the
    # architecture's YAML example, not nested under a separate `config` key.
    config = dict(peripheral.get("config", {})) if isinstance(peripheral.get("config"), dict) else {}
    reserved = {"instance", "type", "_config_key", "key", "name", "pins", "dma", "config"}
    reserved.update(key for key in peripheral if key.endswith("_pin") or key.startswith("dma_"))
    for key, item in peripheral.items():
        if key not in reserved:
            config.setdefault(key, item)
    if config:
        peripheral["config"] = config
    return peripheral
