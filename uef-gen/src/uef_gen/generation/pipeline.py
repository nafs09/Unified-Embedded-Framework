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
from uef_gen.diagnostics import Diagnostic, error, warning
from uef_gen.extensions import load_extensions
from uef_gen.generation.manifest import build_manifest, build_manifest_documents
from uef_gen.generation.models import GenerationResult
from uef_gen.generation.validator import validate_generated_project
from uef_gen.ir.control_ir import validate_control_ir
from uef_gen.modules.planner import ProjectModulePlanner
from uef_gen.modules.bundled import BundledDependencyError, resolve_bundled_dependencies
from uef_gen.modules.selector import ModuleSelectionError
from uef_gen.resolver import ResourceResolver
from uef_gen.locator import UEFNotFoundError, UEFVersionIncompatible, UEFLocator
from uef_gen.snapshot import SnapshotError, SnapshotManager
from uef_gen.templates.project_scaffold import render_project_scaffold
from uef_gen.uef_adapter.templates import UEFTemplateAdapter, UEFTemplateContractError

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
                     snapshot_manager: SnapshotManager | None = None,
                     project_directory: Path | None = None) -> GenerationResult:
    """Validate, resolve, assemble, validate again, then publish one project.

    All writes happen in a temporary stage until generated paths and references
    pass validation. ControlIR remains NEXUS-authored input; the UEF adapter
    renders UEF-owned UCON/UPROTO templates and surfaces declared manual actions.
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

    locator = UEFLocator(project_directory)
    template_adapter: UEFTemplateAdapter | None = None
    try:
        if snapshot_manager is not None and snapshot_manager.root is not None:
            snapshot_mgr = snapshot_manager
        else:
            snapshot_mgr = SnapshotManager(locator.locate(config))
        snapshot = snapshot_mgr.verify()
        diagnostics.extend(warning("uef_preversioned", message, "uef.path")
                           for message in locator.warnings + snapshot_mgr.warnings)
        from uef_gen.registry import load_module_manifest

        module_manifest = load_module_manifest(snapshot.root)
        template_modules: tuple[str, ...] = ()
        if control_ir is not None:
            template_adapter = UEFTemplateAdapter(snapshot.root)
            template_modules = template_adapter.required_modules(control_ir)
        selection = ProjectModulePlanner(snapshot.root).select(
            config, resolved, additional_modules=template_modules
        )
        bundled_dependencies = resolve_bundled_dependencies(
            snapshot.root,
            selection.bundled_dependency_names,
            config,
            resolved,
        )
    except BundledDependencyError as exc:
        return GenerationResult(
            False,
            output,
            diagnostics=diagnostics + [error("bundled_dependency_unavailable", str(exc), "uef.bundled_dependencies")],
        )
    except UEFTemplateContractError as exc:
        return GenerationResult(
            False,
            output,
            diagnostics=diagnostics + [error(exc.diagnostic_code, str(exc), "control_ir")],
        )
    except (SnapshotError, ModuleSelectionError, UEFNotFoundError, UEFVersionIncompatible,
            OSError, json.JSONDecodeError, TypeError) as exc:
        return GenerationResult(
            False,
            output,
            diagnostics=diagnostics + [error("uef_source_or_modules", str(exc), "uef.path")],
        )

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

    try:
        work.mkdir(parents=True, exist_ok=True)
        if output.exists():
            output.rmdir()  # It was checked empty above.
        with tempfile.TemporaryDirectory(prefix="uef-gen-", dir=work) as temporary:
            stage = Path(temporary) / "project"
            stage.mkdir()
            copied = SnapshotManager.copy_modules(snapshot, selection.modules, module_manifest, stage)
            copied.extend(
                SnapshotManager.copy_bundled_dependencies(
                    snapshot, bundled_dependencies, stage
                )
            )
            copied.extend(SnapshotManager.copy_manifests(snapshot, stage))
            context = {
                "project": resolved,
                "target": resolved.chip,
                "capabilities": resolved.capabilities,
                "modules": selection.modules,
                "control_ir": control_ir,
                "uef_api_database": (
                    template_adapter.api_document if template_adapter is not None else None
                ),
                "snapshot": snapshot,
                "template_root": snapshot.root / "templates",
            }
            generated = list(render_project_scaffold(
                resolved, snapshot.source_hash, selection.modules
            ))
            manual_actions: list[dict[str, Any]] = []
            if template_adapter is not None:
                try:
                    rendered_control = template_adapter.render_control_ir(context)
                    generated.extend(rendered_control.files)
                    manual_actions.extend(rendered_control.manual_actions)
                except UEFTemplateContractError as exc:
                    return GenerationResult(
                        False,
                        output,
                        diagnostics=diagnostics + [error(exc.diagnostic_code, str(exc), "control_ir")],
                    )
            diagnostics.extend(
                warning(
                    "manual_implementation_required",
                    f"{action['code']}: {action['message']}",
                    action["node_path"] or "control_ir",
                )
                for action in manual_actions
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
            bundled_sources = [
                f"{dependency.root}/{source}"
                for dependency in bundled_dependencies
                for source in dependency.sources
            ]
            all_sources = [
                *uef_sources,
                *bundled_sources,
                *(
                    artifact.path
                    for artifact in generated
                    if artifact.path.endswith((".c", ".cpp"))
                ),
            ]
            includes = [f"uef/{directory}" for directory in selection.include_directories]
            includes.extend(
                f"{dependency.root}/{directory}"
                for dependency in bundled_dependencies
                for directory in dependency.include_directories
            )
            includes.extend([".", "include", "uos"])
            bundled_defines = [
                define
                for dependency in bundled_dependencies
                for define in dependency.defines
            ]
            defines = list(dict.fromkeys([*selection.defines, *bundled_defines, *resolved.defines]))
            cflags = list(resolved.compiler_flags)
            # UEF's portable UMATH kernels call the C math library. GCC/Clang
            # firmware links it as `m`; MSVC supplies these routines through its CRT.
            libraries = ["m"] if any(name.startswith("umath/") for name in selection.modules) else []
            file_lists = {
                "project_sources.txt": all_sources,
                "project_includes.txt": includes,
                "project_defines.txt": defines,
                "project_cflags.txt": cflags,
                "project_libraries.txt": libraries,
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
                "PROJECT_CFLAGS := $(shell cat $(PROJECT_ROOT)/project_cflags.txt)\n"
                "PROJECT_LDLIBS := $(addprefix -l,$(shell cat $(PROJECT_ROOT)/project_libraries.txt))\n",
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
                "file(STRINGS \"${UEF_PROJECT_ROOT}/project_libraries.txt\" UEF_PROJECT_LIBRARIES)\n"
                "list(TRANSFORM UEF_PROJECT_SOURCE_RELATIVE PREPEND \"${UEF_PROJECT_ROOT}/\")\n"
                "list(TRANSFORM UEF_PROJECT_INCLUDE_RELATIVE PREPEND \"${UEF_PROJECT_ROOT}/\")\n"
                "function(uef_add_project target)\n"
                "  add_executable(${target} ${UEF_PROJECT_SOURCE_RELATIVE})\n"
                "  target_include_directories(${target} PRIVATE ${UEF_PROJECT_INCLUDE_RELATIVE})\n"
                "  target_compile_definitions(${target} PRIVATE ${UEF_PROJECT_DEFINES})\n"
                "  target_compile_options(${target} PRIVATE ${UEF_PROJECT_CFLAGS})\n"
                "  if(MSVC)\n"
                "    list(REMOVE_ITEM UEF_PROJECT_LIBRARIES m)\n"
                "  endif()\n"
                "  if(UEF_PROJECT_LIBRARIES)\n"
                "    target_link_libraries(${target} PRIVATE ${UEF_PROJECT_LIBRARIES})\n"
                "  endif()\n"
                "endfunction()\n", encoding="utf-8", newline="\n")

            generated_files = [
                *copied,
                *(artifact.path for artifact in generated),
                *file_lists.keys(),
                "config/project.mk",
                "config/project.cmake",
                "manifest/project.json",
                "manifest/resources.json",
                "manifest/modules.json",
                "manifest/manual_actions.json",
                "manifest/generation_meta.json",
                "uef-gen-manifest.json",
            ]
            manifest = build_manifest(
                resolved,
                snapshot,
                selection.modules,
                generated_files,
                all_sources,
                includes,
                defines,
                cflags,
                libraries,
                selection.external_dependencies,
                bundled_dependencies,
                manual_actions,
                {
                    "hardware_configuration": _input_hash(config),
                    "control_ir": _input_hash(control_ir),
                },
            )
            for relative, document in build_manifest_documents(manifest).items():
                path = stage / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(
                    json.dumps(document, indent=2, sort_keys=True) + "\n",
                    encoding="utf-8",
                    newline="\n",
                )
            (stage / "uef-gen-manifest.json").write_text(
                json.dumps(manifest, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
                newline="\n",
            )
            diagnostics.extend(validate_generated_project(stage, generated_files, all_sources))
            if any(item.severity == "error" for item in diagnostics):
                return GenerationResult(False, output, diagnostics=diagnostics)
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
