from __future__ import annotations
import hashlib
import json
import os
import shutil
import tempfile
from pathlib import Path
from typing import Any
from uef_gen.chips import UnknownChipError, load_chip_database
from uef_gen.config.schema import validate_configuration
from uef_gen.chips.development_boards import load_development_boards
from uef_gen.diagnostics import Diagnostic, error, warning
from uef_gen.extensions import load_extensions
from uef_gen.generation.manifest import build_manifest, build_manifest_documents
from uef_gen.generation.models import GenerationResult
from uef_gen.generation.validator import validate_generated_project
from uef_gen.uetd_adapter.locator import UETDLocator, UETDNotFoundError, UETDVersionIncompatible
from uef_gen.modules.bundled import BundledDependencyError, resolve_bundled_dependencies
from uef_gen.modules.planner import ProjectModulePlanner
from uef_gen.modules.selector import ModuleSelectionError
from uef_gen.resolver import ResourceResolver
from uef_gen.resolver.resource_resolver import UnknownDevelopmentBoardError
from uef_gen.uetd_adapter.snapshot import SnapshotError, SnapshotManager
from uef_gen.templates.project_scaffold import render_project_scaffold

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

def generate_project(
    config: dict[str, Any],
    output_directory: Path,
    work_directory: Path,
    snapshot_manager: SnapshotManager | None = None,
    project_directory: Path | None = None,
) -> GenerationResult:
    """Validate, resolve, assemble, validate again, then publish one project.

    All writes happen in a temporary stage until generated paths and references
    pass validation.
    """
    output = output_directory.expanduser().resolve()
    work = work_directory.expanduser().resolve()
    try:
        load_extensions()
    except Exception as exc:
        return GenerationResult(False, output, diagnostics=[error("extension_load_failed", str(exc))])
    diagnostics = validate_configuration(config)
    if diagnostics:
        return GenerationResult(False, output, diagnostics=diagnostics)
    if output == work or _inside(output, work) or _inside(work, output):
        return GenerationResult(
            False, output, diagnostics=[error("output_work_overlap", "Output and work directories must be separate and must not contain one another")]
        )
    if output.exists() and not output.is_dir():
        return GenerationResult(False, output, diagnostics=[error("output_not_directory", f"Output path is not a directory: {output}")])
    if output.is_dir() and any(output.iterdir()):
        return GenerationResult(False, output, diagnostics=[error("output_not_empty", f"Output directory is not empty: {output}")])
    if work.exists() and not work.is_dir():
        return GenerationResult(False, output, diagnostics=[error("work_not_directory", f"Work path is not a directory: {work}")])
    locator = UETDLocator(project_directory)
    try:
        if snapshot_manager is not None and snapshot_manager.root is not None:
            snapshot_mgr = snapshot_manager
        else:
            snapshot_mgr = SnapshotManager(locator.locate(config))
        snapshot = snapshot_mgr.verify()
        database = load_chip_database(snapshot.root)
        development_boards = load_development_boards(snapshot.root)
        resolved = ResourceResolver(database, development_boards).resolve(config)
    except UnknownDevelopmentBoardError as exc:
        return GenerationResult(False, output, diagnostics=[error("development_board_unknown", str(exc), "target.development_board")])
    except UnknownChipError as exc:
        return GenerationResult(False, output, diagnostics=[error("chip_unknown", str(exc), "target.chip")])
    except (SnapshotError, UETDNotFoundError, UETDVersionIncompatible, OSError, json.JSONDecodeError) as exc:
        return GenerationResult(False, output, diagnostics=[error("uetd_target_registry_invalid", str(exc), "uetd.path")])
    except (AttributeError, TypeError, ValueError, OverflowError) as exc:
        return GenerationResult(False, output, diagnostics=[error("resource_config_invalid", str(exc), "hardware_configuration")])
    diagnostics = list(resolved.diagnostics)
    if any(item.severity == "error" for item in diagnostics):
        return GenerationResult(False, output, diagnostics=diagnostics)
    try:
        diagnostics.extend(warning("uetd_preversioned", message, "uetd.path") for message in locator.warnings + snapshot_mgr.warnings)
        from uef_gen.uetd_adapter.registry import load_module_manifest
        module_manifest = load_module_manifest(snapshot.root)
        selection = ProjectModulePlanner(snapshot.root).select(config, resolved)
        bundled_dependencies = resolve_bundled_dependencies(snapshot.root, selection.bundled_dependency_names, config, resolved)
    except BundledDependencyError as exc:
        return GenerationResult(False, output, diagnostics=diagnostics + [error("bundled_dependency_unavailable", str(exc), "uetd.bundled_dependencies")])
    except (SnapshotError, ModuleSelectionError, UETDNotFoundError, UETDVersionIncompatible, OSError, json.JSONDecodeError, TypeError) as exc:
        return GenerationResult(False, output, diagnostics=diagnostics + [error("uetd_source_or_modules", str(exc), "uetd.path")])
    for capability in selection.required_capabilities:
        if not resolved.capabilities.supports(capability):
            diagnostics.append(error("module_capability_unavailable", f"Selected UETD module requires unavailable capability '{capability}'"))
    if resolved.rtos.casefold() == "safertos" and "uos/safertos" not in selection.modules:
        diagnostics.append(error("rtos_backend_not_selected", "target.rtos is SAFERTOS but no uos_safertos backend module is registered", "target.rtos"))
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
            copied.extend(SnapshotManager.copy_bundled_dependencies(snapshot, bundled_dependencies, stage))
            copied.extend(SnapshotManager.copy_manifests(snapshot, stage))
            generated = list(render_project_scaffold(resolved, snapshot.source_hash, selection.modules))
            manual_actions: list[dict[str, Any]] = []
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
            uetd_sources = [f"uetd/{path}" for path in selection.sources]
            bundled_sources = [f"{dependency.root}/{source}" for dependency in bundled_dependencies for source in dependency.sources]
            all_sources = [*uetd_sources, *bundled_sources, *(artifact.path for artifact in generated if artifact.path.endswith((".c", ".cpp")))]
            includes = [f"uetd/{directory}" for directory in selection.include_directories]
            includes.extend(f"{dependency.root}/{directory}" for dependency in bundled_dependencies for directory in dependency.include_directories)
            includes.extend([".", "include", "uos"])
            bundled_defines = [define for dependency in bundled_dependencies for define in dependency.defines]
            defines = list(dict.fromkeys([*selection.defines, *bundled_defines, *resolved.defines]))
            cflags = list(resolved.compiler_flags)
            # UETD's portable UMATH kernels call the C math library. GCC/Clang
            # firmware links it as `m`; MSVC supplies these routines through its CRT.
            libraries = ["m"] if any(name.startswith("umath/") for name in selection.modules) else []
            file_lists = {
                "project_sources.txt": all_sources,
                "project_includes.txt": includes,
                "project_defines.txt": defines,
                "project_cflags.txt": cflags,
                "project_libraries.txt": libraries,
                "uetd_sources.txt": uetd_sources,
            }
            for filename, entries in file_lists.items():
                (stage / filename).write_text("".join(entry + "\n" for entry in entries), encoding="utf-8", newline="\n")
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
                'get_filename_component(UETD_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)\n'
                'file(STRINGS "${UETD_PROJECT_ROOT}/project_sources.txt" UETD_PROJECT_SOURCE_RELATIVE)\n'
                'file(STRINGS "${UETD_PROJECT_ROOT}/project_includes.txt" UETD_PROJECT_INCLUDE_RELATIVE)\n'
                'file(STRINGS "${UETD_PROJECT_ROOT}/project_defines.txt" UETD_PROJECT_DEFINES)\n'
                'file(STRINGS "${UETD_PROJECT_ROOT}/project_cflags.txt" UETD_PROJECT_CFLAGS)\n'
                'file(STRINGS "${UETD_PROJECT_ROOT}/project_libraries.txt" UETD_PROJECT_LIBRARIES)\n'
                'list(TRANSFORM UETD_PROJECT_SOURCE_RELATIVE PREPEND "${UETD_PROJECT_ROOT}/")\n'
                'list(TRANSFORM UETD_PROJECT_INCLUDE_RELATIVE PREPEND "${UETD_PROJECT_ROOT}/")\n'
                "function(uetd_add_project target)\n"
                "  add_executable(${target} ${UETD_PROJECT_SOURCE_RELATIVE})\n"
                "  target_include_directories(${target} PRIVATE ${UETD_PROJECT_INCLUDE_RELATIVE})\n"
                "  target_compile_definitions(${target} PRIVATE ${UETD_PROJECT_DEFINES})\n"
                "  target_compile_options(${target} PRIVATE ${UETD_PROJECT_CFLAGS})\n"
                "  if(MSVC)\n"
                "    list(REMOVE_ITEM UETD_PROJECT_LIBRARIES m)\n"
                "  endif()\n"
                "  if(UETD_PROJECT_LIBRARIES)\n"
                "    target_link_libraries(${target} PRIVATE ${UETD_PROJECT_LIBRARIES})\n"
                "  endif()\n"
                "endfunction()\n",
                encoding="utf-8",
                newline="\n",
            )
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
                {"hardware_configuration": _input_hash(config)},
            )
            for relative, document in build_manifest_documents(manifest).items():
                path = stage / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(json.dumps(document, indent=2, sort_keys=True) + "\n", encoding="utf-8", newline="\n")
            (stage / "uef-gen-manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8", newline="\n")
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
        return GenerationResult(False, output, diagnostics=diagnostics + [error("generation_failed", str(exc))])
