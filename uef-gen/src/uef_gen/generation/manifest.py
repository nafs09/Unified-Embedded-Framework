from __future__ import annotations

from datetime import datetime, timezone
from typing import Any
from dataclasses import asdict

from uef_gen.generation.models import GenerationResult
from uef_gen.resolver.models import ResolvedProject
from uef_gen.snapshot import UefSnapshot


def build_manifest(project: ResolvedProject, snapshot: UefSnapshot, selected_modules: tuple[str, ...],
                   files: list[str], sources: list[str], includes: list[str],
                   defines: list[str], cflags: list[str], external_dependencies: tuple[str, ...],
                   signal_bindings: dict[str, tuple[Any, ...]],
                   input_hashes: dict[str, str | None]) -> dict[str, Any]:
    return {
        "manifest_version": "1.0",
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "project_name": project.name,
        "target": {"chip": project.chip.name, "arithmetic": project.arithmetic, "rtos": project.rtos},
        "uef": {"version": snapshot.version, "snapshot_hash": snapshot.content_hash,
                "modules": list(selected_modules)},
        "files": sorted(files),
        "input_hashes": input_hashes,
        "external_dependencies": list(external_dependencies),
        "build": {"sources": sources, "includes": includes, "defines": defines, "cflags": cflags},
        "resource_resolution": {
            "peripherals": [{"instance": p.instance, "type": p.type_name, "pins": p.pins,
                             "alternate_functions": p.alternate_functions, "dma_channels": p.dma_channels,
                             "irqs": list(p.irq_names)} for p in project.peripherals],
            "tasks": [{"name": t.name, "period_us": t.period_us, "priority": t.priority,
                       "stack_bytes": t.stack_bytes} for t in project.tasks],
        },
        "ucon_signal_bindings": {
            subsystem: [asdict(binding) for binding in bindings]
            for subsystem, bindings in sorted(signal_bindings.items())
        },
    }
