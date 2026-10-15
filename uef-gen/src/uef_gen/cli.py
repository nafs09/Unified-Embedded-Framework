"""Standalone command line and JSON-contract entry points for uef-gen."""

from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path
from typing import Any
from uef_gen import __version__
from uef_gen.chips import UnknownChipError, find_chip_record, load_chip_catalogue, load_chip_database
from uef_gen.chips.target_catalogue import load_target_family_catalogue
from uef_gen.config.loader import load_configuration
from uef_gen.config.schema import validate_configuration
from uef_gen.contract import CONTRACT_VERSION, handle_request
from uef_gen.chips.development_boards import load_development_boards
from uef_gen.diagnostics import Diagnostic, error, warning
from uef_gen.uetd_adapter.locator import UETDLocator
from uef_gen.modules.planner import ProjectModulePlanner
from uef_gen.uetd_adapter.registry import load_module_manifest
from uef_gen.resolver import ResourceResolver
from uef_gen.resolver.resource_resolver import UnknownDevelopmentBoardError
from uef_gen.uetd_adapter.snapshot import SnapshotManager
from uef_gen.toolchains.cli import add_tool_commands, handle_tool_command

def _legacy_parser() -> argparse.ArgumentParser:
    """Keep the original bridge-facing options usable while the command CLI is adopted."""
    parser = argparse.ArgumentParser(prog="uef-gen", description="Assemble a UETD embedded project from hardware configuration.")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--request", type=Path, help="Versioned JSON contract request; stdin is used when omitted")
    mode.add_argument("--config", type=Path, help="Standalone project.yaml or project.json hardware configuration")
    parser.add_argument("--output", type=Path, help="Generated project output directory")
    parser.add_argument("--work", type=Path, help="Isolated staging/work directory")
    return parser

def _command_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="uef-gen", description="Compile declarative UETD projects and assemble UETD-based firmware.")
    commands = parser.add_subparsers(dest="command", required=True)
    check = commands.add_parser("check", help="parse and validate a .uef, YAML, or JSON project without writing output")
    check.add_argument("configuration", type=Path)
    check.add_argument("--uetd", type=Path, help="UETD checkout used for registry resolution")
    generate = commands.add_parser("generate", help="generate a firmware project")
    generate.add_argument("configuration", type=Path)
    generate.add_argument("--output", "-o", type=Path, help="generated-project directory (default: ./build/uef-gen/<project-name>)")
    generate.add_argument("--work", type=Path)
    generate.add_argument("--uetd", type=Path, help="UETD checkout used for registry resolution")
    generate.add_argument("--target", "-t", help="override target.chip")
    generate.add_argument("--dry-run", action="store_true", help="validate and resolve modules without writing project files")
    validate = commands.add_parser("validate", help="validate configuration and target resources")
    validate.add_argument("configuration", type=Path)
    targets = commands.add_parser("targets", help="inspect registered target records")
    target_commands = targets.add_subparsers(dest="targets_command", required=True)
    target_list = target_commands.add_parser("list")
    target_list.add_argument("--family", help="filter exact parts and family roadmap entries")
    target_list.add_argument("--include-scaffolds", action="store_true", help="also read UETD chip records, Phase 0/1 profiles, and later roadmap entries")
    target_list.add_argument("--uetd", type=Path, help="UETD checkout to read the canonical family catalogue from")
    target_show = target_commands.add_parser("show")
    target_show.add_argument("chip")
    target_show.add_argument("--uetd", type=Path, help="UETD checkout containing chip records")
    modules = commands.add_parser("modules", help="inspect UETD module metadata")
    module_commands = modules.add_subparsers(dest="modules_command", required=True)
    module_commands.add_parser("list")
    module_show = module_commands.add_parser("show")
    module_show.add_argument("module")
    module_resolve = module_commands.add_parser("resolve")
    module_resolve.add_argument("configuration", type=Path)
    snapshot = commands.add_parser("snapshot", help="validate or describe a UETD checkout")
    snapshot_commands = snapshot.add_subparsers(dest="snapshot_command", required=True)
    snapshot_commands.add_parser("verify")
    snapshot_commands.add_parser("info")
    add_tool_commands(commands)
    commands.add_parser("version", help="show uef-gen and located UETD versions")
    return parser

def _load_project(path: Path, target_override: str | None = None) -> dict[str, Any]:
    config = load_configuration(path)
    if target_override:
        target = config.setdefault("target", {})
        if not isinstance(target, dict):
            raise ValueError("target must be an object before --target can be applied")
        target["chip"] = target_override
    return config

def _compile_uetd_source(path: Path, uetd_root: Path | None = None):
    """Call the one project-language compiler API for every .uef frontend path."""
    from uef_gen.language.frontends.project import compile_project_file
    return compile_project_file(path, uetd_root=uetd_root)

def _compile_uefs_source(path: Path, uetd_root: Path | None = None):
    """Compile a multi-target system through its public frontend API."""
    from uef_gen.language.frontends.system.compiler import compile_system_file
    return compile_system_file(path, uetd_root=uetd_root)

def _diagnostic_response(diagnostics: list[Diagnostic]) -> dict[str, Any]:
    return {"ok": not any(item.severity == "error" for item in diagnostics), "diagnostics": [item.to_dict() for item in diagnostics]}

def _resolve_project(config: dict[str, Any], project_directory: Path | None = None) -> tuple[Any | None, list[Diagnostic]]:
    diagnostics = validate_configuration(config)
    if diagnostics:
        return None, diagnostics
    try:
        uetd_root = UETDLocator(project_directory or Path.cwd()).locate(config)
        database = load_chip_database(uetd_root)
        boards = load_development_boards(uetd_root)
        resolved = ResourceResolver(database, boards).resolve(config)
    except UnknownDevelopmentBoardError as exc:
        return None, [error("development_board_unknown", str(exc), "target.development_board")]
    except UnknownChipError as exc:
        return None, [error("chip_unknown", str(exc), "target.chip")]
    except ValueError as exc:
        return None, [error("uetd_target_registry_invalid", str(exc), "uetd.path")]
    except Exception as exc:
        return None, [error("uetd_target_registry_invalid", str(exc), "uetd.path")]
    return resolved, list(resolved.diagnostics)

def _uetd_snapshot(project_directory: Path, config: dict[str, Any] | None = None):
    locator = UETDLocator(project_directory)
    root = locator.locate(config or {})
    manager = SnapshotManager(root)
    snapshot = manager.verify()
    return locator, manager, snapshot

def _module_selection(config: dict[str, Any], configuration_path: Path) -> tuple[Any, Any, list[Diagnostic]]:
    resolved, diagnostics = _resolve_project(config, configuration_path.resolve().parent)
    if resolved is None or any(item.severity == "error" for item in diagnostics):
        return None, None, diagnostics
    try:
        locator, manager, snapshot = _uetd_snapshot(configuration_path.resolve().parent, config)
        selection = ProjectModulePlanner(snapshot.root).select(config, resolved)
        diagnostics.extend(warning("uetd_preversioned", message, "uetd.path") for message in locator.warnings + manager.warnings)
        return snapshot, selection, diagnostics
    except Exception as exc:
        return None, None, diagnostics + [error("uetd_source_or_modules", str(exc), "uetd.path")]

def _run_legacy(argv: list[str]) -> int:
    parser = _legacy_parser()
    args = parser.parse_args(argv)
    try:
        if args.config:
            if args.output is None or args.work is None:
                parser.error("--config requires --output and --work")
            config = load_configuration(args.config)
            request = {
                "contract_version": CONTRACT_VERSION,
                "operation": "generate",
                "hardware_configuration": config,
                "output_directory": str(args.output.resolve()),
                "work_directory": str(args.work.resolve()),
                "project_directory": str(args.config.resolve().parent),
            }
        else:
            raw = args.request.read_text(encoding="utf-8") if args.request else sys.stdin.read()
            request = json.loads(raw)
        response = handle_request(request)
    except Exception as exc:
        response = {
            "contract_version": CONTRACT_VERSION,
            "ok": False,
            "diagnostics": [{"severity": "error", "code": "request_failed", "message": str(exc)}],
        }
    print(json.dumps(response, separators=(",", ":"), ensure_ascii=False))
    return 0 if response.get("ok") else 2

def _run_command(argv: list[str]) -> int:
    parser = _command_parser()
    args = parser.parse_args(argv)
    try:
        if args.command == "tools":
            response, exit_code = handle_tool_command(args)
            _emit(response)
            return exit_code
        if args.command == "check":
            source_path = args.configuration.expanduser().resolve()
            if source_path.suffix.casefold() == ".uefs":
                compiled_system = _compile_uefs_source(source_path, args.uetd)
                _emit(compiled_system.to_dict())
                return 0 if compiled_system.ok else 2
            if source_path.suffix.casefold() == ".uef":
                compiled = _compile_uetd_source(source_path, args.uetd)
                if not compiled.ok:
                    _emit(compiled.to_dict())
                    return 2
                config = compiled.hardware_configuration
                application = compiled.to_dict().get("application")
            else:
                config = _load_project(source_path)
                application = None
                if args.uetd is not None:
                    config.setdefault("uetd", {})["path"] = str(args.uetd.expanduser().resolve())
            snapshot, selection, diagnostics = _module_selection(config, source_path)
            response = _diagnostic_response(diagnostics)
            if response["ok"] and selection is not None:
                target = config.get("target", {})
                response.update(
                    {
                        "project": config.get("project", {}).get("name", ""),
                        "target": (target.get("chip") or target.get("development_board") if isinstance(target, dict) else None),
                        "uetd": {"version": snapshot.version, "source_hash": snapshot.source_hash},
                        "selected_modules": list(selection.modules),
                        "sources": list(selection.sources),
                    }
                )
                if application is not None:
                    response["application"] = application
                    compiled_dict = compiled.to_dict()
                    response["hardware_configuration"] = compiled_dict.get("hardware_configuration")
                    response["backend_input"] = compiled_dict.get("backend_input")
            _emit(response)
            return 0 if response["ok"] else 2
        if args.command == "validate":
            config = _load_project(args.configuration)
            resolved, diagnostics = _resolve_project(config, args.configuration.resolve().parent)
            response = _diagnostic_response(diagnostics)
            if response["ok"] and resolved is not None:
                response["project"] = config.get("project", {}).get("name", "")
                response["target"] = resolved.chip.registry_id or resolved.chip.name
                if resolved.development_board is not None:
                    response["development_board"] = resolved.development_board["id"]
            _emit(response)
            return 0 if response["ok"] else 2
        if args.command == "generate":
            source_path = args.configuration.expanduser().resolve()
            if source_path.suffix.casefold() == ".uefs":
                compiled_system = _compile_uefs_source(source_path, args.uetd)
                if not compiled_system.ok:
                    _emit(compiled_system.to_dict())
                    return 2
                _emit({"ok": False, "diagnostics": [{"severity": "error", "code": "system_generation_backend_unavailable", "message": "The .uefs frontend validates and lowers a complete multi-target build input, but no firmware emitter consumes it yet."}], "build_plan": compiled_system.to_dict().get("build_plan"), "backend_input": compiled_system.to_dict().get("backend_input")})
                return 2
            if source_path.suffix.casefold() == ".uef":
                if args.target is not None:
                    _emit(
                        {
                            "ok": False,
                            "diagnostics": [
                                {
                                    "severity": "error",
                                    "code": "project_language_cli_override",
                                    "message": "A .uef file owns its target and project graph; put them in the source file instead of using --target.",
                                }
                            ],
                        }
                    )
                    return 2
                compiled = _compile_uetd_source(source_path, args.uetd)
                if not compiled.ok:
                    _emit(compiled.to_dict())
                    return 2
                if not args.dry_run:
                    compiled_dict = compiled.to_dict()
                    _emit({"ok": False, "diagnostics": [{"severity": "error", "code": "project_language_generation_backend_unavailable", "message": "The .uef frontend validates and lowers a complete standalone backend input, but the firmware generation pipeline still consumes hardware configuration."}], "backend_input": compiled_dict.get("backend_input"), "application": compiled_dict.get("application")})
                    return 2
                config = compiled.hardware_configuration
            else:
                config = _load_project(source_path, args.target)
                if args.uetd is not None:
                    config.setdefault("uetd", {})["path"] = str(args.uetd.expanduser().resolve())
            if args.dry_run:
                snapshot, selection, diagnostics = _module_selection(config, source_path)
                response = _diagnostic_response(diagnostics)
                if response["ok"] and selection is not None:
                    response["uetd"] = {"version": snapshot.version, "source_hash": snapshot.source_hash}
                    response["selected_modules"] = list(selection.modules)
                    response["sources"] = list(selection.sources)
                    if source_path.suffix.casefold() == ".uef":
                        compiled_dict = compiled.to_dict()
                        response["application"] = compiled_dict.get("application")
                        response["hardware_configuration"] = compiled_dict.get("hardware_configuration")
                        response["backend_input"] = compiled_dict.get("backend_input")
                _emit(response)
                return 0 if response["ok"] else 2
            output = args.output.expanduser().resolve() if args.output else source_path.parent / "build" / "uef-gen" / source_path.stem
            work = args.work.resolve() if args.work else output.parent / ".uef-gen-work"
            request = {
                "contract_version": CONTRACT_VERSION,
                "operation": "generate",
                "hardware_configuration": config,
                "output_directory": str(output),
                "work_directory": str(work),
                "project_directory": str(source_path.parent),
            }
            response = handle_request(request)
            _emit(response)
            return 0 if response.get("ok") else 2
        if args.command == "targets":
            if args.targets_command == "list":
                uetd_root = args.uetd.expanduser().resolve() if args.uetd else UETDLocator(Path.cwd()).locate()
                database = load_chip_database(uetd_root)
                chips = [database.get(name) for name in database.names()]
                if args.family:
                    chips = [chip for chip in chips if chip.family.name.casefold() == args.family.casefold()]
                response = {
                    "targets": [
                        {
                            "name": chip.name,
                            "family": chip.family.name,
                            "id": chip.registry_id,
                            "architecture": chip.family.arch.name,
                            "verified": chip.verified,
                            "generation_available": chip.generation_available,
                            "target_kind": chip.target_kind,
                        }
                        for chip in chips
                    ]
                }
                if args.include_scaffolds:
                    if args.uetd:
                        uetd_root = args.uetd.expanduser().resolve()
                    else:
                        uetd_root = UETDLocator(Path.cwd()).locate()
                    catalogue = load_target_family_catalogue(uetd_root)
                    entries = catalogue["entries"]
                    chip_catalogue = load_chip_catalogue(uetd_root)
                    chip_records = chip_catalogue["records"]
                    if args.family:
                        needle = args.family.casefold()
                        chip_records = [
                            item
                            for item in chip_records
                            if any(
                                needle in str(value).casefold()
                                for value in (
                                    item.get("id", ""),
                                    item.get("vendor", ""),
                                    item.get("family_id", ""),
                                    item.get("series", ""),
                                    item.get("part_number", ""),
                                    *item.get("aliases", []),
                                )
                            )
                        ]
                    if args.family:
                        needle = args.family.casefold()
                        entries = [
                            item
                            for item in entries
                            if any(
                                needle in str(value).casefold()
                                for value in (
                                    item.get("id", ""),
                                    item.get("vendor", ""),
                                    item.get("family", ""),
                                    item.get("series", ""),
                                    *item.get("aliases", []),
                                    *item.get("representative_parts", []),
                                )
                            )
                        ]
                    response.update(
                        {
                            "chip_records": chip_records,
                            "chip_catalogue": {
                                "schema_version": chip_catalogue["schema_version"],
                                "canonical_owner": chip_catalogue["canonical_owner"],
                                "records": chip_records,
                            },
                            "phase0_family_scaffolds": [
                                {
                                    **item,
                                    "key": item["id"],
                                    "phase": item.get("roadmap_phase"),
                                    "architecture": "; ".join(item.get("architectures", [])),
                                    "spec_reference": "UETD Specification Parts 3.2/15.1",
                                }
                                for item in entries
                                if item.get("roadmap_phase") == 0
                            ],
                            "phase1_family_scaffolds": [
                                {
                                    **item,
                                    "key": item["id"],
                                    "phase": 1,
                                    "architecture": "; ".join(item.get("architectures", [])),
                                    "spec_reference": "UETD Specification Parts 3.2/15.1",
                                }
                                for item in entries
                                if item.get("roadmap_phase") == 1 or 1 in item.get("additional_phase_targets", [])
                            ],
                            "target_family_catalogue": {
                                "schema_version": catalogue["schema_version"],
                                "canonical_owner": catalogue["canonical_owner"],
                                "entries": entries,
                            },
                        }
                    )
                _emit(response)
                return 0
            uetd_root = args.uetd.expanduser().resolve() if args.uetd else UETDLocator(Path.cwd()).locate()
            catalogue = load_chip_catalogue(uetd_root)
            record = find_chip_record(catalogue, args.chip)
            if record is None:
                _emit({"ok": False, "diagnostics": [error("chip_unknown", f"No chip named '{args.chip}' is registered in UETD", "target.chip").to_dict()]})
                return 2
            _emit(record)
            return 0
        if args.command == "modules":
            if args.modules_command == "resolve":
                config = _load_project(args.configuration)
                snapshot, selection, diagnostics = _module_selection(config, args.configuration)
                response = _diagnostic_response(diagnostics)
                if response["ok"] and selection is not None:
                    response.update(
                        {
                            "uetd_version": snapshot.version,
                            "modules": list(selection.modules),
                            "sources": list(selection.sources),
                            "include_directories": list(selection.include_directories),
                            "defines": list(selection.defines),
                            "external_dependencies": list(selection.external_dependencies),
                        }
                    )
                _emit(response)
                return 0 if response["ok"] else 2
            locator, manager, snapshot = _uetd_snapshot(Path.cwd())
            document = load_module_manifest(snapshot.root)
            modules = document.get("modules", {})
            if args.modules_command == "list":
                _emit(
                    {
                        "uetd_version": snapshot.version,
                        "modules": [
                            {
                                "name": name,
                                "dependencies": record.get("dependencies", []),
                                "source_count": len(record.get("sources", [])),
                                "external_dependencies": record.get("external_dependencies", []),
                            }
                            for name, record in sorted(modules.items())
                        ],
                    }
                )
                return 0
            record = modules.get(args.module.casefold())
            if record is None:
                _emit({"ok": False, "diagnostics": [error("uetd_module_unknown", f"UETD module is not registered: {args.module}").to_dict()]})
                return 2
            _emit({"uetd_version": snapshot.version, "name": args.module.casefold(), **record})
            return 0
        if args.command == "snapshot":
            locator, manager, snapshot = _uetd_snapshot(Path.cwd())
            modules = load_module_manifest(snapshot.root).get("modules", {})
            _emit(
                {
                    "ok": True,
                    "uetd_root": str(snapshot.root),
                    "uetd_version": snapshot.version,
                    "source_hash": snapshot.source_hash,
                    "module_count": len(modules),
                    "warnings": locator.warnings + manager.warnings,
                }
            )
            return 0
        if args.command == "version":
            response: dict[str, Any] = {"uef_gen_version": __version__}
            try:
                locator, manager, snapshot = _uetd_snapshot(Path.cwd())
                response.update({"uetd_version": snapshot.version, "uetd_root": str(snapshot.root)})
                response["warnings"] = locator.warnings + manager.warnings
            except Exception as exc:
                response["uetd_version"] = None
                response["uetd_status"] = str(exc)
            _emit(response)
            return 0
    except Exception as exc:
        _emit({"ok": False, "diagnostics": [error("cli_operation_failed", str(exc)).to_dict()]})
        return 2
    parser.error("unsupported command")
    return 2

def _emit(value: dict[str, Any]) -> None:
    """Emit one JSON document so the same CLI can be scripted by NEXUS."""
    print(json.dumps(value, indent=2, sort_keys=True, ensure_ascii=False))

def main(argv: list[str] | None = None) -> int:
    """Dispatch user-facing commands or the stable JSON request interface."""
    arguments = list(sys.argv[1:] if argv is None else argv)
    if arguments and arguments[0] in {"build", "deploy", "probe", "flash"}:
        from uef_gen.deploy.cli import main as deployment_main
        return deployment_main(arguments)
    if arguments and arguments[0] == "monitor":
        from uef_gen.deploy.cli import main as deployment_main
        return deployment_main(arguments)
    if arguments and arguments[0] in {"check", "generate", "validate", "targets", "modules", "snapshot", "tools", "version"}:
        return _run_command(arguments)
    return _run_legacy(arguments)


if __name__ == "__main__":
    raise SystemExit(main())
