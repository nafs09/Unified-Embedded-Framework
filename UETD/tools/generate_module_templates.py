#!/usr/bin/env python3
"""Build renderable CMake routes from the UETD module registry."""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path, PurePosixPath, PureWindowsPath
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REGISTRY = ROOT / "registry" / "modules" / "registry.json"
OUTPUT_ROOT = ROOT / "templates" / "modules"
INDEX = OUTPUT_ROOT / "index.json"
CATALOG = OUTPUT_ROOT / "catalog.md"
LEGACY_GENERATOR_ID = "tools/generate_module_template_scaffolds.py"
MODULE_ID = re.compile(r"^[A-Za-z0-9_/-]+$")


def _read_json(path: Path) -> dict[str, Any]:
    document = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(document, dict):
        raise ValueError(f"expected a JSON object in {path.relative_to(ROOT)}")
    return document


def _safe_relative(value: object, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{label} must be a non-empty relative path")
    normalized = value.replace("\\", "/")
    path = PurePosixPath(normalized)
    windows_path = PureWindowsPath(normalized)
    if path.is_absolute() or windows_path.is_absolute() or windows_path.drive or ".." in path.parts or not path.parts:
        raise ValueError(f"unsafe {label}: {value!r}")
    return path.as_posix()


def _resource_paths(record: dict[str, Any], field: str) -> list[str]:
    raw = record.get(field, [])
    if not isinstance(raw, list):
        raise ValueError(f"module manifest {field!r} must be an array")
    return [_safe_relative(item, f"module {field} path") for item in raw]


def _is_future(module_id: str, record: dict[str, Any], resources: list[str]) -> bool:
    segments = module_id.split("/")
    return (
        "future" in segments
        or record.get("scaffold_stage") == "future"
        or any("future" in PurePosixPath(resource).parts for resource in resources)
    )


def _output_relative_path(module_id: str, future: bool) -> Path:
    if not MODULE_ID.fullmatch(module_id):
        raise ValueError(f"invalid module ID: {module_id!r}")
    parts = list(PurePosixPath(module_id).parts)
    if future and len(parts) > 1 and "future" not in parts:
        parts.insert(1, "future")
    return Path(*parts, "route.tmpl")


def _target_id(module_id: str) -> str:
    return re.sub(r"[^A-Za-z0-9_]+", "_", module_id).strip("_").lower()


def _template_source(
    module_id: str,
    sources: list[str],
    headers: list[str],
    dependencies: list[str],
) -> str:
    values = {
        "module_id": module_id,
        "target_id": _target_id(module_id),
        "sources": sources,
        "headers": headers,
        "dependencies": dependencies,
    }
    declarations = "\n".join(
        f"{{% set {name} = {json.dumps(value, ensure_ascii=False)} %}}"
        for name, value in values.items()
    )
    return f"""{declarations}

include_guard(GLOBAL)

if(NOT DEFINED UETD_ROOT OR NOT IS_DIRECTORY "${{UETD_ROOT}}/include")
  message(FATAL_ERROR "Set UETD_ROOT to the UETD checkout or copied source root")
endif()

set(_uetd_module_target "uetd_module_{{{{ target_id }}}}")
set(_uetd_module_sources)
set(_uetd_module_headers)
set(_uetd_module_dependencies)
{{% for source in sources %}}
list(APPEND _uetd_module_sources "${{UETD_ROOT}}/{{{{ source }}}}")
{{% endfor %}}
{{% for header in headers %}}
list(APPEND _uetd_module_headers "${{UETD_ROOT}}/{{{{ header }}}}")
{{% endfor %}}
{{% for dependency in dependencies %}}
list(APPEND _uetd_module_dependencies "uetd_module_{{{{ dependency | replace('/', '_') | replace('-', '_') | lower }}}}")
{{% endfor %}}

foreach(_uetd_module_file IN LISTS _uetd_module_sources _uetd_module_headers)
  if(NOT EXISTS "${{_uetd_module_file}}")
    message(FATAL_ERROR "UETD module file is missing: ${{_uetd_module_file}}")
  endif()
endforeach()

if(NOT TARGET ${{_uetd_module_target}})
  if(_uetd_module_sources)
    add_library(${{_uetd_module_target}} STATIC ${{_uetd_module_sources}})
    if(_uetd_module_headers)
      target_sources(${{_uetd_module_target}} PUBLIC ${{_uetd_module_headers}})
    endif()
    target_include_directories(${{_uetd_module_target}} PUBLIC "${{UETD_ROOT}}/include")
    target_compile_features(${{_uetd_module_target}} PUBLIC c_std_11)
    set(_uetd_module_link_scope PUBLIC)
  else()
    add_library(${{_uetd_module_target}} INTERFACE)
    if(_uetd_module_headers)
      target_sources(${{_uetd_module_target}} INTERFACE ${{_uetd_module_headers}})
    endif()
    target_include_directories(${{_uetd_module_target}} INTERFACE "${{UETD_ROOT}}/include")
    set(_uetd_module_link_scope INTERFACE)
  endif()
  if(UETD_MODULE_INCLUDE_DIRS)
    target_include_directories(${{_uetd_module_target}} ${{_uetd_module_link_scope}} ${{UETD_MODULE_INCLUDE_DIRS}})
  endif()
  if(UETD_MODULE_COMPILE_DEFINITIONS)
    target_compile_definitions(${{_uetd_module_target}} ${{_uetd_module_link_scope}} ${{UETD_MODULE_COMPILE_DEFINITIONS}})
  endif()
  if(UETD_MODULE_COMPILE_OPTIONS)
    target_compile_options(${{_uetd_module_target}} ${{_uetd_module_link_scope}} ${{UETD_MODULE_COMPILE_OPTIONS}})
  endif()
  if(UETD_MODULE_LINK_LIBRARIES)
    target_link_libraries(${{_uetd_module_target}} ${{_uetd_module_link_scope}} ${{UETD_MODULE_LINK_LIBRARIES}})
  endif()
  add_library(UETD::Module_{{{{ target_id }}}} ALIAS ${{_uetd_module_target}})
  foreach(_uetd_module_dependency IN LISTS _uetd_module_dependencies)
    if(NOT TARGET ${{_uetd_module_dependency}})
      message(FATAL_ERROR "Include the dependency route before {{{{ module_id }}}}: ${{_uetd_module_dependency}}")
    endif()
    target_link_libraries(${{_uetd_module_target}} ${{_uetd_module_link_scope}} ${{_uetd_module_dependency}})
  endforeach()
endif()
"""


def _resource_exists(path: str) -> bool:
    resolved = (ROOT / Path(*PurePosixPath(path).parts)).resolve()
    try:
        resolved.relative_to(ROOT.resolve())
    except ValueError as exc:
        raise ValueError(f"resource path escapes UETD root: {path!r}") from exc
    return resolved.is_file()


def _sha256(contents: str) -> str:
    return hashlib.sha256(contents.encode("utf-8")).hexdigest()


def _previous_index() -> dict[str, Any]:
    if not INDEX.is_file():
        return {}
    try:
        return _read_json(INDEX)
    except (OSError, UnicodeDecodeError, json.JSONDecodeError, ValueError):
        return {}


def _previous_template_hashes(index: dict[str, Any]) -> dict[str, str]:
    result: dict[str, str] = {}
    modules = index.get("modules")
    if not isinstance(modules, dict):
        return result
    for item in modules.values():
        if isinstance(item, dict):
            path = item.get("template")
            digest = item.get("template_sha256")
            if isinstance(path, str) and isinstance(digest, str):
                result[path] = digest
    return result


def _ensure_owned_file(path: Path, previous_hash: str | None, legacy_owned: bool) -> None:
    if not path.exists():
        return
    try:
        existing = path.read_text(encoding="utf-8")
    except (OSError, UnicodeDecodeError) as exc:
        raise ValueError(f"refusing to replace non-text template {path.relative_to(ROOT)}") from exc
    if previous_hash is not None and _sha256(existing) == previous_hash:
        return
    if legacy_owned:
        return
    raise ValueError(f"refusing to replace hand-maintained file {path.relative_to(ROOT)}")


def main() -> None:
    registry = _read_json(REGISTRY)
    modules = registry.get("modules")
    if not isinstance(modules, dict) or not modules:
        raise ValueError("registry/modules/registry.json must contain a non-empty modules object")

    old_index = _previous_index()
    legacy_owned = old_index.get("generated_by") == LEGACY_GENERATOR_ID
    if INDEX.exists() and not legacy_owned and old_index.get("template_kind") != "cmake-module-route":
        raise ValueError(f"refusing to overwrite hand-maintained {INDEX.relative_to(ROOT)}")
    legacy_catalog = legacy_owned
    previous_hashes = _previous_template_hashes(old_index)
    rendered: dict[Path, str] = {}
    records: dict[str, Any] = {}

    for module_id, module_ref in sorted(modules.items()):
        if not isinstance(module_ref, dict):
            raise ValueError(f"module registry entry {module_id!r} must be an object")
        manifest_path = _safe_relative(module_ref.get("manifest"), f"{module_id} manifest")
        manifest_file = ROOT / Path(*PurePosixPath(manifest_path).parts)
        if not manifest_file.is_file():
            raise FileNotFoundError(f"module manifest does not exist: {manifest_path}")
        record = _read_json(manifest_file)
        sources = _resource_paths(record, "sources")
        headers = _resource_paths(record, "headers")
        dependencies = record.get("dependencies", [])
        if not isinstance(dependencies, list):
            raise ValueError(f"module manifest dependencies must be an array: {manifest_path}")
        dependencies = [str(item) for item in dependencies]
        future = _is_future(module_id, record, sources + headers)
        relative_template = _output_relative_path(module_id, future)
        template_path = OUTPUT_ROOT / relative_template
        if template_path in rendered:
            raise ValueError(f"multiple module IDs map to {relative_template.as_posix()}")

        content = _template_source(module_id, sources, headers, dependencies)
        template_relative = (Path("templates") / "modules" / relative_template).as_posix()
        _ensure_owned_file(template_path, previous_hashes.get(template_relative), legacy_owned)
        rendered[template_path] = content
        missing = [resource for resource in sources + headers if not _resource_exists(resource)]
        authoring = record.get("authoring")
        generation = authoring.get("generation", {}) if isinstance(authoring, dict) else {}
        records[module_id] = {
            "template": template_relative,
            "template_sha256": _sha256(content),
            "detail_manifest": manifest_path,
            "sources": sources,
            "headers": headers,
            "dependencies": dependencies,
            "future_scope": future,
            "scaffold_only": bool(record.get("scaffold_only", False)),
            "available_for_generation": record.get("available_for_generation"),
            "authoring_status": authoring.get("status") if isinstance(authoring, dict) else None,
            "generation_status": generation.get("status") if isinstance(generation, dict) else None,
            "missing_resources": missing,
        }

    old_modules = old_index.get("modules")
    if isinstance(old_modules, dict):
        current_paths = {item["template"] for item in records.values()}
        for old in old_modules.values():
            if not isinstance(old, dict):
                continue
            old_path = old.get("template", old.get("template_scaffold"))
            if not isinstance(old_path, str) or old_path in current_paths:
                continue
            candidate = (ROOT / old_path).resolve()
            try:
                candidate.relative_to(OUTPUT_ROOT.resolve())
            except ValueError as exc:
                raise ValueError(f"stale template path escapes output directory: {old_path}") from exc
            expected_hash = previous_hashes.get(old_path)
            if candidate.is_file():
                existing = candidate.read_text(encoding="utf-8")
                if expected_hash and _sha256(existing) != expected_hash and not legacy_owned:
                    raise ValueError(f"refusing to remove hand-maintained file {old_path}")
                candidate.unlink()

    for path, content in rendered.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8", newline="\n")

    catalog_lines = [
        "# Module build routes",
        "",
        f"Module routes: **{len(records)}**; future-scoped routes: **{sum(item['future_scope'] for item in records.values())}**.",
        "Each route renders a CMake target from the module's declared source, header, and dependency lists.",
        "",
        "| Module ID | Scope | Availability | Authoring | Sources | Headers | Dependencies | Route |",
        "|---|---|---|---|---:|---:|---:|---|",
    ]
    for module_id, item in records.items():
        scope = "future" if item["future_scope"] else "catalog"
        availability = item["available_for_generation"]
        if availability is None:
            availability = "unspecified"
        authoring_status = item["authoring_status"] or "missing"
        route = item["template"].removeprefix("templates/modules/")
        catalog_lines.append(
            f"| `{module_id}` | {scope} | {str(availability).lower()} | {authoring_status} | "
            f"{len(item['sources'])} | {len(item['headers'])} | {len(item['dependencies'])} | [`route.tmpl`]({route}) |"
        )
    catalog = "\n".join(catalog_lines) + "\n"
    catalog_legacy_hash = old_index.get("catalog_sha256")
    if CATALOG.exists():
        existing_catalog = CATALOG.read_text(encoding="utf-8")
        if not legacy_catalog and not (isinstance(catalog_legacy_hash, str) and _sha256(existing_catalog) == catalog_legacy_hash):
            raise ValueError(f"refusing to overwrite hand-maintained {CATALOG.relative_to(ROOT)}")
    CATALOG.write_text(catalog, encoding="utf-8", newline="\n")

    index = {
        "schema_version": "1.0",
        "template_kind": "cmake-module-route",
        "module_count": len(records),
        "future_scope_count": sum(1 for item in records.values() if item["future_scope"]),
        "catalog_sha256": _sha256(catalog),
        "modules": records,
    }
    INDEX.parent.mkdir(parents=True, exist_ok=True)
    INDEX.write_text(json.dumps(index, indent=2, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n")
    print(f"Wrote {len(rendered)} CMake module routes to {OUTPUT_ROOT.relative_to(ROOT)}")
    print(f"Future-scoped routes: {index['future_scope_count']}")
    missing_count = sum(len(item["missing_resources"]) for item in records.values())
    print(f"Missing source/header references: {missing_count}")


if __name__ == "__main__":
    main()
