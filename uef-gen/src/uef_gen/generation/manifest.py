from __future__ import annotations
from datetime import datetime, timezone
import platform
from typing import Any
from uef_gen import __version__
from uef_gen.generation.models import GenerationResult
from uef_gen.resolver.models import ResolvedProject
from uef_gen.snapshot import UefSnapshot
from uef_gen.modules.bundled import BundledDependencySelection

def build_manifest(project: ResolvedProject, snapshot: UefSnapshot, selected_modules: tuple[str, ...],
                   files: list[str], sources: list[str], includes: list[str],
                   defines: list[str], cflags: list[str], libraries: list[str],
                   external_dependencies: tuple[str, ...],
                   bundled_dependencies: tuple[BundledDependencySelection, ...],
                   manual_actions: list[dict[str, Any]],
                   input_hashes: dict[str, str | None]) -> dict[str, Any]:
    return {
        "manifest_version": "1.0",
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "project_name": project.name,
        "target": {"chip": project.chip.name, "arithmetic": project.arithmetic, "rtos": project.rtos},
        "uef": {"version": snapshot.version, "source_hash": snapshot.source_hash,
                "modules": list(selected_modules)},
        "files": sorted(files),
        "input_hashes": input_hashes,
        "external_dependencies": list(external_dependencies),
        "bundled_dependencies": [
            {
                "name": item.name,
                "version": item.version,
                "revision": item.revision,
                "profile": item.profile,
                "sources": list(item.sources),
                "include_directories": list(item.include_directories),
                "defines": list(item.defines),
                "license_files": list(item.license_files),
            }
            for item in bundled_dependencies
        ],
        "manual_actions": manual_actions,
        "build": {"sources": sources, "includes": includes, "defines": defines,
                  "cflags": cflags, "libraries": libraries},
        "resource_resolution": {
            "peripherals": [{"instance": p.instance, "type": p.type_name, "pins": p.pins,
                             "alternate_functions": p.alternate_functions, "dma_channels": p.dma_channels,
                             "irqs": list(p.irq_names)} for p in project.peripherals],
            "tasks": [{"name": t.name, "period_us": t.period_us, "priority": t.priority,
                       "stack_bytes": t.stack_bytes} for t in project.tasks],
        },
    }

def build_manifest_documents(manifest: dict[str, Any]) -> dict[str, dict[str, Any]]:
    """Split provenance into the stable project/resource/module records and run metadata."""
    generated_at = str(manifest.get("generated_at_utc", datetime.now(timezone.utc).isoformat()))
    return {
        "manifest/project.json": {
            "schema_version": "1.0",
            "project_name": manifest.get("project_name"),
            "target": manifest.get("target", {}),
            "uef": manifest.get("uef", {}),
            "build": manifest.get("build", {}),
            "input_hashes": manifest.get("input_hashes", {}),
        },
        "manifest/resources.json": {
            "schema_version": "1.0",
            **manifest.get("resource_resolution", {}),
        },
        "manifest/modules.json": {
            "schema_version": "1.0",
            **manifest.get("uef", {}),
            "external_dependencies": manifest.get("external_dependencies", []),
            "bundled_dependencies": manifest.get("bundled_dependencies", []),
        },
        "manifest/manual_actions.json": {
            "schema_version": "1.0",
            "actions": manifest.get("manual_actions", []),
        },
        "manifest/generation_meta.json": {
            "schema_version": "1.0",
            "generated_at_utc": generated_at,
            "uef_gen_version": __version__,
            "host": {
                "platform": platform.platform(),
                "python": platform.python_version(),
            },
        },
    }
