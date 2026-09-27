from __future__ import annotations

import json
import hashlib
import os
import shutil
import tempfile
from pathlib import Path
from typing import Any

from uef_gen.chips import DEFAULT_DATABASE, ChipDatabase, UnknownChipError
from uef_gen.config.schema import validate_configuration
from uef_gen.diagnostics import Diagnostic, error
from uef_gen.extensions import load_extensions
from uef_gen.generation.manifest import build_manifest
from uef_gen.generation.models import GenerationResult
from uef_gen.generation.validator import validate_generated_project
from uef_gen.ir.algorithms import validate_algorithm_ir
from uef_gen.ir.control_ir import validate_control_ir
from uef_gen.ir.registry import UnknownNodeTypeError, build_node_ir
from uef_gen.modules.selector import ModuleSelectionError, ModuleSelector
from uef_gen.resolver import ResourceResolver
from uef_gen.snapshot import SnapshotError, SnapshotManager
from uef_gen.templates.project_scaffold import render_project_scaffold, render_user_algorithms
from uef_gen.templates.registry import UnknownTemplateSetError
from uef_gen.ucon import UEFApiDatabase, resolve_signal_bindings
from uef_gen.ucon.api_database import UEFApiError


def _inside(parent: Path, child: Path) -> bool:
    """Return whether child is beneath parent after both paths are normalized."""
    try:
        child.relative_to(parent)
        return True
    except ValueError:
        return False


def _input_hash(document: Any | None) -> str | None:
    """Hash the canonical JSON form of one request input for provenance."""
    if document is None:
        return None
    payload = json.dumps(document, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    return f"sha256:{hashlib.sha256(payload).hexdigest()}"


def generate_project(config: dict[str, Any], control_ir: dict[str, Any] | None,
                     output_directory: Path, work_directory: Path,
                     database: ChipDatabase = DEFAULT_DATABASE,
                     snapshot_manager: SnapshotManager | None = None) -> GenerationResult:
    """Validate, resolve, assemble, validate again, then publish one project.

    All writes happen in a temporary stage until generated paths and references
    pass validation. The ControlIR/algorithm branch is a migration-only bridge:
    UEF is becoming the owner of UCON definitions and templates.
    """
    output = output_directory.expanduser().resolve()
    work = work_directory.expanduser().resolve()
    try:
        load_extensions()
    except Exception as exc:
        return GenerationResult(
            False,
            output,
            diagnostics=[error("extension_load_failed", str(exc))],
        )
    diagnostics = validate_configuration(config)
    if diagnostics:
        return GenerationResult(False, output, diagnostics=diagnostics)
    if output == work or _inside(output, work) or _inside(work, output):
        return GenerationResult(
            False,
            output,
            diagnostics=[
                error(
                    "output_work_overlap",
                    "Output and work directories must be separate and must not contain one another",
                )
            ],
        )
    if output.exists() and not output.is_dir():
        return GenerationResult(
            False,
            output,
            diagnostics=[error("output_not_directory", f"Output path is not a directory: {output}")],
        )
    if output.is_dir() and any(output.iterdir()):
        return GenerationResult(
            False,
            output,
            diagnostics=[error("output_not_empty", f"Output directory is not empty: {output}")],
        )
    if work.exists() and not work.is_dir():
        return GenerationResult(
            False,
            output,
            diagnostics=[error("work_not_directory", f"Work path is not a directory: {work}")],
        )

    try:
        resolved = ResourceResolver(database).resolve(config)
    except UnknownChipError as exc:
        return GenerationResult(
            False,
            output,
            diagnostics=[error("chip_unknown", str(exc), "target.chip")],
        )
    except (AttributeError, TypeError, ValueError, OverflowError) as exc:
        return GenerationResult(
            False,
            output,
            diagnostics=[
                error("resource_config_invalid", str(exc), "hardware_configuration")
            ],
        )
    diagnostics = list(resolved.diagnostics)
    if any(item.severity == "error" for item in diagnostics):
        return GenerationResult(False, output, diagnostics=diagnostics)

    if control_ir is not None:
        diagnostics.extend(validate_control_ir(control_ir))
        if control_ir.get("schema_version") != "1.0":
            diagnostics.append(error("control_ir_version", "Expected ControlIR schema_version 1.0"))
        if any(item.severity == "error" for item in diagnostics):
            return GenerationResult(False, output, diagnostics=diagnostics)

    snapshot_mgr = snapshot_manager or SnapshotManager()
    try:
        snapshot = snapshot_mgr.verify()
        module_manifest = json.loads((snapshot.root / "uef_modules.json").read_text(encoding="utf-8"))
        requested_modules: set[str] = set()
        architecture = str(getattr(resolved.chip.family.arch, "name", "")).casefold()

        def add_configured_module(name: str) -> None:
            """Expand friendly layer names into exact entries from the UEF manifest."""
            # The legacy generator treated UCON as generator-owned output. The
            # project direction is now for UEF to own UCON; until UEF publishes
            # a module/selection contract, ignore this old alias and do not
            # assemble duplicate generator-side algorithms.
            if name == "ucon":
                return
            if name == "uhal":
                requested_modules.add("uhal/arm_cm" if "cortex" in architecture or "arm" in architecture
                                      else "uhal/x86")
                return
            if name == "uos":
                requested_modules.add("uos/freertos" if resolved.rtos.casefold() == "freertos"
                                      else "uos/baremetal")
                return
            if name == "umid":
                requested_modules.update(("umid/ringbuf", "umid/health"))
                return
            if name == "upal":
                # The generic baseline is deliberately small. Peripheral
                # allocations below add the exact extra UPAL modules required.
                requested_modules.update(("upal/dma", "upal/uart", "upal/spi", "upal/i2c"))
                return
            if name == "uproto":
                return  # Protocols are selected explicitly or by configuration.
            requested_modules.add(name)

        for configured_module in resolved.modules:
            add_configured_module(str(configured_module).casefold())

        # The selected target backend and common scalar/time contract are
        # required even when a project requests only one higher-level module.
        requested_modules.add("ucore")
        requested_modules.add("uhal/arm_cm" if "cortex" in architecture or "arm" in architecture
                              else "uhal/x86")

        peripheral_providers = module_manifest.get("peripheral_provides", {})
        for peripheral in resolved.peripherals:
            for module_name in peripheral_providers.get(peripheral.type_name.upper(), []):
                requested_modules.add(str(module_name).casefold())

        protocol_providers = module_manifest.get("protocol_provides", {})
        configured_protocols = config.get("protocols", [])
        if isinstance(configured_protocols, list):
            for protocol in configured_protocols:
                for module_name in protocol_providers.get(str(protocol).upper(), []):
                    requested_modules.add(str(module_name).casefold())

        if resolved.rtos.casefold() == "freertos":
            requested_modules.add("uos/freertos")
        elif resolved.rtos.casefold() in {"", "none", "baremetal"}:
            requested_modules.add("uos/baremetal")
        selection = ModuleSelector(snapshot.root).select(tuple(sorted(requested_modules)))
    except (SnapshotError, ModuleSelectionError, OSError, json.JSONDecodeError) as exc:
        return GenerationResult(
            False,
            output,
            diagnostics=diagnostics + [error("uef_snapshot_or_modules", str(exc))],
        )

    signal_bindings: dict[str, tuple[Any, ...]] = {}
    api_database: UEFApiDatabase | None = None
    if config.get("ucon") is not None:
        try:
            api_database = UEFApiDatabase.from_file(snapshot.root / "uef_api.json")
            signal_bindings, binding_diagnostics = resolve_signal_bindings(config, resolved, api_database)
            diagnostics.extend(binding_diagnostics)
        except UEFApiError as exc:
            diagnostics.append(error("uef_api_database_invalid", str(exc), "ucon"))
        if any(item.severity == "error" for item in diagnostics):
            return GenerationResult(False, output, diagnostics=diagnostics)

    for capability in selection.required_capabilities:
        if not resolved.capabilities.supports(capability):
                diagnostics.append(
                    error(
                        "module_capability_unavailable",
                        f"Selected UEF module requires unavailable capability '{capability}'",
                    )
                )
    if resolved.rtos.casefold() == "safertos" and "uos/safertos" not in selection.modules:
        diagnostics.append(
            error(
                "rtos_backend_not_selected",
                "target.rtos is SAFERTOS but no uos_safertos backend module is registered",
                "target.rtos",
            )
        )
    if any(item.severity == "error" for item in diagnostics):
        return GenerationResult(False, output, diagnostics=diagnostics)

    algorithms: list[Any] = []
    if control_ir is not None:
        unregistered: set[tuple[str, str]] = set()
        for node_index, node in enumerate(control_ir.get("nodes", [])):
            try:
                algorithm = build_node_ir(node)
            except UnknownNodeTypeError as exc:
                unregistered.add((str(node.get("type", "")), str(node.get("subtype", "*"))))
                continue
            except Exception as exc:
                diagnostics.append(error(
                    "algorithm_builder_failed",
                    f"AlgorithmIR builder failed: {exc}",
                    f"nodes[{node_index}]",
                ))
                continue
            node_id = str(node.get("id", node_index))
            diagnostics.extend(validate_algorithm_ir(algorithm, f"nodes.{node_id}"))
            algorithms.append(algorithm)
        if unregistered:
            diagnostics.extend(
                error(
                    "algorithm_builder_missing",
                    f"No registered AlgorithmIR builder for {node_type}/{subtype}",
                )
                for node_type, subtype in sorted(unregistered)
            )
        if any(item.severity == "error" for item in diagnostics):
            return GenerationResult(False, output, diagnostics=diagnostics)

    try:
        work.mkdir(parents=True, exist_ok=True)
        if output.exists():
            output.rmdir()  # It was checked empty above.
        with tempfile.TemporaryDirectory(prefix="uef-gen-", dir=work) as temporary:
            stage = Path(temporary) / "project"
            stage.mkdir()
            copied = SnapshotManager.copy_modules(snapshot, selection.modules, module_manifest, stage)
            context = {
                "project": resolved,
                "target": resolved.chip,
                "capabilities": resolved.capabilities,
                "modules": selection.modules,
                "algorithms": tuple(algorithms),
                "control_ir": control_ir,
                "signal_bindings": signal_bindings,
                "uef_api_database": api_database,
                "snapshot": snapshot,
                "template_root": snapshot.root / "templates" / "ucon",
            }
            generated = list(render_project_scaffold(resolved, snapshot.content_hash))
            try:
                generated.extend(render_user_algorithms(tuple(algorithms), context))
            except UnknownTemplateSetError as exc:
                return GenerationResult(
                    False,
                    output,
                    diagnostics=diagnostics + [error("template_set_missing", str(exc))],
                )
            paths = [artifact.path for artifact in generated]
            if len(paths) != len(set(paths)):
                raise ValueError("multiple templates attempted to write the same generated path")
            for artifact in generated:
                relative = Path(artifact.path)
                if relative.is_absolute() or ".." in relative.parts:
                    raise ValueError(f"template attempted to write outside the project: {artifact.path}")
                path = stage / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(artifact.content, encoding="utf-8", newline="\n")

            uef_sources = [f"uef/{path}" for path in selection.sources]
            all_sources = [
                *uef_sources,
                *(
                    artifact.path
                    for artifact in generated
                    if artifact.path.endswith((".c", ".cpp"))
                ),
            ]
            includes = [f"uef/{directory}" for directory in selection.include_directories]
            includes.extend(["include"])
            defines = list(dict.fromkeys([*selection.defines, *resolved.defines]))
            cflags = list(resolved.compiler_flags)
            file_lists = {
                "project_sources.txt": all_sources,
                "project_includes.txt": includes,
                "project_defines.txt": defines,
                "project_cflags.txt": cflags,
                "uef_sources.txt": uef_sources,
            }
            for filename, entries in file_lists.items():
                (stage / filename).write_text(
                    "".join(entry + "\n" for entry in entries),
                    encoding="utf-8",
                    newline="\n",
                )
            config_dir = stage / "config"
            config_dir.mkdir()
            (config_dir / "project.mk").write_text(
                "# Optional GNU Make fragment. The .txt files remain canonical.\n"
                "PROJECT_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)\n"
                "PROJECT_SOURCES := $(addprefix $(PROJECT_ROOT)/,$(shell cat $(PROJECT_ROOT)/project_sources.txt))\n"
                "PROJECT_INCLUDES := $(addprefix -I$(PROJECT_ROOT)/,$(shell cat $(PROJECT_ROOT)/project_includes.txt))\n"
                "PROJECT_DEFINES := $(addprefix -D,$(shell cat $(PROJECT_ROOT)/project_defines.txt))\n"
                "PROJECT_CFLAGS := $(shell cat $(PROJECT_ROOT)/project_cflags.txt)\n",
                encoding="utf-8",
                newline="\n",
            )
            (config_dir / "project.cmake").write_text(
                "# Optional CMake fragment. Include from the generated project root.\n"
                "get_filename_component(UEF_PROJECT_ROOT \"${CMAKE_CURRENT_LIST_DIR}/..\" ABSOLUTE)\n"
                "file(STRINGS \"${UEF_PROJECT_ROOT}/project_sources.txt\" UEF_PROJECT_SOURCE_RELATIVE)\n"
                "file(STRINGS \"${UEF_PROJECT_ROOT}/project_includes.txt\" UEF_PROJECT_INCLUDE_RELATIVE)\n"
                "file(STRINGS \"${UEF_PROJECT_ROOT}/project_defines.txt\" UEF_PROJECT_DEFINES)\n"
                "file(STRINGS \"${UEF_PROJECT_ROOT}/project_cflags.txt\" UEF_PROJECT_CFLAGS)\n"
                "list(TRANSFORM UEF_PROJECT_SOURCE_RELATIVE PREPEND \"${UEF_PROJECT_ROOT}/\")\n"
                "list(TRANSFORM UEF_PROJECT_INCLUDE_RELATIVE PREPEND \"${UEF_PROJECT_ROOT}/\")\n"
                "function(uef_add_project target)\n"
                "  add_executable(${target} ${UEF_PROJECT_SOURCE_RELATIVE})\n"
                "  target_include_directories(${target} PRIVATE ${UEF_PROJECT_INCLUDE_RELATIVE})\n"
                "  target_compile_definitions(${target} PRIVATE ${UEF_PROJECT_DEFINES})\n"
                "  target_compile_options(${target} PRIVATE ${UEF_PROJECT_CFLAGS})\n"
                "endfunction()\n", encoding="utf-8", newline="\n")

            generated_files = [
                *copied,
                *(artifact.path for artifact in generated),
                *file_lists.keys(),
                "config/project.mk",
                "config/project.cmake",
            ]
            diagnostics.extend(validate_generated_project(stage, generated_files, all_sources))
            if any(item.severity == "error" for item in diagnostics):
                return GenerationResult(False, output, diagnostics=diagnostics)
            manifest = build_manifest(
                resolved,
                snapshot,
                selection.modules,
                [*generated_files, "uef-gen-manifest.json"],
                all_sources,
                includes,
                defines,
                cflags,
                selection.external_dependencies,
                signal_bindings,
                {
                    "hardware_configuration": _input_hash(config),
                    "control_ir": _input_hash(control_ir),
                },
            )
            (stage / "uef-gen-manifest.json").write_text(
                json.dumps(manifest, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
                newline="\n",
            )
            output.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.TemporaryDirectory(prefix="uef-gen-publish-", dir=output.parent) as publishing:
                complete = Path(publishing) / "project"
                shutil.copytree(stage, complete)
                os.replace(complete, output)
            return GenerationResult(True, output, manifest, diagnostics)
    except Exception as exc:
        return GenerationResult(
            False,
            output,
            diagnostics=diagnostics + [error("generation_failed", str(exc))],
        )
