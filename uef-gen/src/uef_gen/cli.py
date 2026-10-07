"""Standalone command line and JSON-contract entry points for uef-gen."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

from uef_gen import __version__
from uef_gen.chips import (
    DEFAULT_DATABASE, UnknownChipError, find_chip_base_record,
    load_chip_base_catalogue,
)
from uef_gen.chips.target_catalogue import load_target_family_catalogue
from uef_gen.config.loader import load_configuration
from uef_gen.config.schema import validate_configuration
from uef_gen.contract import CONTRACT_VERSION, handle_request
from uef_gen.diagnostics import Diagnostic, error, warning
from uef_gen.ir.control_ir import validate_control_ir
from uef_gen.locator import UEFLocator
from uef_gen.modules.planner import ProjectModulePlanner
from uef_gen.registry import load_module_manifest
from uef_gen.resolver import ResourceResolver
from uef_gen.snapshot import SnapshotManager
from uef_gen.uef_adapter.templates import UEFTemplateAdapter


def _read_json(path: Path) -> Any:
    """Read one UTF-8 JSON document from disk."""
    return json.loads(path.read_text(encoding="utf-8"))


def _legacy_parser() -> argparse.ArgumentParser:
    """Keep the original bridge-facing options usable while the command CLI is adopted."""
    parser = argparse.ArgumentParser(
        prog="uef-gen",
        description="Assemble a UEF embedded project from hardware configuration.",
    )
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--request", type=Path,
                      help="Versioned JSON contract request; stdin is used when omitted")
    mode.add_argument("--config", type=Path,
                      help="Standalone project.yaml or project.json hardware configuration")
    parser.add_argument("--control-ir", type=Path, help="NEXUS ControlIR JSON")
    parser.add_argument("--output", type=Path, help="Generated project output directory")
    parser.add_argument("--work", type=Path, help="Isolated staging/work directory")
    return parser


def _command_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="uef-gen",
        description="Resolve hardware configuration and assemble UEF-based firmware projects.",
    )
    commands = parser.add_subparsers(dest="command", required=True)

    generate = commands.add_parser("generate", help="generate a firmware project")
    generate.add_argument("configuration", type=Path)
    generate.add_argument("--output", "-o", required=True, type=Path)
    generate.add_argument("--work", type=Path)
    generate.add_argument("--control-ir", type=Path)
    generate.add_argument("--target", "-t", help="override target.chip")
    generate.add_argument("--dry-run", action="store_true",
                          help="validate and resolve modules without writing project files")

    validate = commands.add_parser("validate", help="validate configuration and target resources")
    validate.add_argument("configuration", type=Path)

    targets = commands.add_parser("targets", help="inspect registered target records")
    target_commands = targets.add_subparsers(dest="targets_command", required=True)
    target_list = target_commands.add_parser("list")
    target_list.add_argument("--family", help="filter exact parts and family roadmap entries")
    target_list.add_argument(
        "--include-scaffolds", action="store_true",
        help="also read UEF chip-base records, Phase 0/1 profiles, and later roadmap entries",
    )
    target_list.add_argument(
        "--uef", type=Path,
        help="UEF checkout to read the canonical family catalogue from",
    )
    target_show = target_commands.add_parser("show")
    target_show.add_argument("chip")
    target_show.add_argument("--uef", type=Path, help="UEF checkout containing chip-base records")

    modules = commands.add_parser("modules", help="inspect UEF module metadata")
    module_commands = modules.add_subparsers(dest="modules_command", required=True)
    module_commands.add_parser("list")
    module_show = module_commands.add_parser("show")
    module_show.add_argument("module")
    module_resolve = module_commands.add_parser("resolve")
    module_resolve.add_argument("configuration", type=Path)

    snapshot = commands.add_parser("snapshot", help="validate or describe a UEF checkout")
    snapshot_commands = snapshot.add_subparsers(dest="snapshot_command", required=True)
    snapshot_commands.add_parser("verify")
    snapshot_commands.add_parser("info")

    commands.add_parser("version", help="show uef-gen and located UEF versions")
    return parser


def _load_project(path: Path, target_override: str | None = None) -> dict[str, Any]:
    config = load_configuration(path)
    if target_override:
        target = config.setdefault("target", {})
        if not isinstance(target, dict):
            raise ValueError("target must be an object before --target can be applied")
        target["chip"] = target_override
    return config


def _diagnostic_response(diagnostics: list[Diagnostic]) -> dict[str, Any]:
    return {"ok": not any(item.severity == "error" for item in diagnostics),
            "diagnostics": [item.to_dict() for item in diagnostics]}


def _resolve_project(config: dict[str, Any]) -> tuple[Any | None, list[Diagnostic]]:
    diagnostics = validate_configuration(config)
    if diagnostics:
        return None, diagnostics
    try:
        resolved = ResourceResolver(DEFAULT_DATABASE).resolve(config)
    except Exception as exc:
        return None, [error("resource_config_invalid", str(exc), "hardware_configuration")]
    return resolved, list(resolved.diagnostics)


def _uef_snapshot(project_directory: Path, config: dict[str, Any] | None = None):
    locator = UEFLocator(project_directory)
    root = locator.locate(config or {})
    manager = SnapshotManager(root)
    snapshot = manager.verify()
    return locator, manager, snapshot


def _module_selection(
    config: dict[str, Any],
    configuration_path: Path,
    control_ir: dict[str, Any] | None = None,
) -> tuple[Any, Any, list[Diagnostic]]:
    resolved, diagnostics = _resolve_project(config)
    if resolved is None or any(item.severity == "error" for item in diagnostics):
        return None, None, diagnostics
    if control_ir is not None:
        diagnostics.extend(validate_control_ir(control_ir))
        if control_ir.get("schema_version") != "1.0":
            diagnostics.append(error("control_ir_version", "Expected ControlIR schema_version 1.0"))
        if any(item.severity == "error" for item in diagnostics):
            return None, None, diagnostics
    try:
        locator, manager, snapshot = _uef_snapshot(configuration_path.resolve().parent, config)
        additional_modules = ()
        if control_ir is not None:
            additional_modules = UEFTemplateAdapter(snapshot.root).required_modules(control_ir)
        selection = ProjectModulePlanner(snapshot.root).select(
            config, resolved, additional_modules=additional_modules
        )
        diagnostics.extend(warning("uef_preversioned", message, "uef.path")
                           for message in locator.warnings + manager.warnings)
        return snapshot, selection, diagnostics
    except Exception as exc:
        return None, None, diagnostics + [error("uef_source_or_modules", str(exc), "uef.path")]


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
                "control_ir": _read_json(args.control_ir) if args.control_ir else None,
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
        if args.command == "validate":
            config = _load_project(args.configuration)
            _, diagnostics = _resolve_project(config)
            response = _diagnostic_response(diagnostics)
            if response["ok"]:
                response["project"] = config.get("project", {}).get("name", "")
                response["target"] = config.get("target", {}).get("chip", "")
            _emit(response)
            return 0 if response["ok"] else 2

        if args.command == "generate":
            config = _load_project(args.configuration, args.target)
            if args.dry_run:
                control_ir = _read_json(args.control_ir) if args.control_ir else None
                snapshot, selection, diagnostics = _module_selection(
                    config, args.configuration, control_ir
                )
                response = _diagnostic_response(diagnostics)
                if response["ok"] and selection is not None:
                    response["uef"] = {"version": snapshot.version, "source_hash": snapshot.source_hash}
                    response["selected_modules"] = list(selection.modules)
                    response["sources"] = list(selection.sources)
                _emit(response)
                return 0 if response["ok"] else 2

            output = args.output.resolve()
            work = args.work.resolve() if args.work else output.parent / ".uef-gen-work"
            request = {
                "contract_version": CONTRACT_VERSION,
                "operation": "generate",
                "hardware_configuration": config,
                "control_ir": _read_json(args.control_ir) if args.control_ir else None,
                "output_directory": str(output),
                "work_directory": str(work),
                "project_directory": str(args.configuration.resolve().parent),
            }
            response = handle_request(request)
            _emit(response)
            return 0 if response.get("ok") else 2

        if args.command == "targets":
            if args.targets_command == "list":
                chips = [DEFAULT_DATABASE.get(name) for name in DEFAULT_DATABASE.names()]
                if args.family:
                    chips = [chip for chip in chips if chip.family.name.casefold() == args.family.casefold()]
                response = {"targets": [
                    {"name": chip.name, "family": chip.family.name,
                     "architecture": chip.family.arch.name, "verified": chip.verified}
                    for chip in chips
                ]}
                if args.include_scaffolds:
                    if args.uef:
                        uef_root = args.uef.expanduser().resolve()
                    else:
                        uef_root = UEFLocator(Path.cwd()).locate()
                    catalogue = load_target_family_catalogue(uef_root)
                    entries = catalogue["entries"]
                    chip_catalogue = load_chip_base_catalogue(uef_root)
                    chip_records = chip_catalogue["records"]
                    if args.family:
                        needle = args.family.casefold()
                        chip_records = [
                            item for item in chip_records
                            if any(needle in str(value).casefold() for value in (
                                item.get("id", ""), item.get("vendor", ""),
                                item.get("family_id", ""), item.get("series", ""),
                                item.get("part_number", ""), *item.get("aliases", [])
                            ))
                        ]
                    if args.family:
                        needle = args.family.casefold()
                        entries = [
                            item for item in entries
                            if any(needle in str(value).casefold() for value in (
                                item.get("id", ""), item.get("vendor", ""),
                                item.get("family", ""), item.get("series", ""),
                                *item.get("aliases", []), *item.get("representative_parts", [])
                            ))
                        ]
                    response.update({
                        "chip_base_records": chip_records,
                        "chip_base_catalogue": {
                            "schema_version": chip_catalogue["schema_version"],
                            "canonical_owner": chip_catalogue["canonical_owner"],
                            "records": chip_records
                        },
                        "phase0_family_scaffolds": [
                            {
                                **item,
                                "key": item["id"],
                                "phase": item.get("roadmap_phase"),
                                "architecture": "; ".join(item.get("architectures", [])),
                                "spec_reference": "UEF Specification Parts 3.2/15.1",
                            }
                            for item in entries if item.get("roadmap_phase") == 0
                        ],
                        "phase1_family_scaffolds": [
                            {
                                **item,
                                "key": item["id"],
                                "phase": 1,
                                "architecture": "; ".join(item.get("architectures", [])),
                                "spec_reference": "UEF Specification Parts 3.2/15.1",
                            }
                            for item in entries
                            if item.get("roadmap_phase") == 1
                            or 1 in item.get("additional_phase_targets", [])
                        ],
                        "target_family_catalogue": {
                            "schema_version": catalogue["schema_version"],
                            "canonical_owner": catalogue["canonical_owner"],
                            "entries": entries
                        }
                    })
                _emit(response)
                return 0
            try:
                chip = DEFAULT_DATABASE.get(args.chip)
            except UnknownChipError:
                uef_root = args.uef.expanduser().resolve() if args.uef else UEFLocator(Path.cwd()).locate()
                catalogue = load_chip_base_catalogue(uef_root)
                record = find_chip_base_record(catalogue, args.chip)
                if record is None:
                    raise
                _emit(record)
                return 0
            _emit({
                "name": chip.name,
                "family": chip.family.name,
                "architecture": {"name": chip.family.arch.name,
                                 "word_bits": chip.family.arch.word_bits,
                                 "fpu": chip.family.arch.fpu},
                "package": chip.package,
                "pin_count": chip.pin_count,
                "flash_bytes": chip.flash_bytes,
                "sram_bytes": chip.sram_bytes,
                "verified": chip.verified,
                "source_note": chip.source_note,
            })
            return 0

        if args.command == "modules":
            if args.modules_command == "resolve":
                config = _load_project(args.configuration)
                snapshot, selection, diagnostics = _module_selection(config, args.configuration)
                response = _diagnostic_response(diagnostics)
                if response["ok"] and selection is not None:
                    response.update({
                        "uef_version": snapshot.version,
                        "modules": list(selection.modules),
                        "sources": list(selection.sources),
                        "include_directories": list(selection.include_directories),
                        "defines": list(selection.defines),
                        "external_dependencies": list(selection.external_dependencies),
                    })
                _emit(response)
                return 0 if response["ok"] else 2

            locator, manager, snapshot = _uef_snapshot(Path.cwd())
            document = load_module_manifest(snapshot.root)
            modules = document.get("modules", {})
            if args.modules_command == "list":
                _emit({"uef_version": snapshot.version, "modules": [
                    {"name": name, "dependencies": record.get("dependencies", []),
                     "source_count": len(record.get("sources", [])),
                     "external_dependencies": record.get("external_dependencies", [])}
                    for name, record in sorted(modules.items())
                ]})
                return 0
            record = modules.get(args.module.casefold())
            if record is None:
                _emit({"ok": False, "diagnostics": [
                    error("uef_module_unknown", f"UEF module is not registered: {args.module}").to_dict()
                ]})
                return 2
            _emit({"uef_version": snapshot.version, "name": args.module.casefold(), **record})
            return 0

        if args.command == "snapshot":
            locator, manager, snapshot = _uef_snapshot(Path.cwd())
            modules = load_module_manifest(snapshot.root).get("modules", {})
            _emit({
                "ok": True,
                "uef_root": str(snapshot.root),
                "uef_version": snapshot.version,
                "source_hash": snapshot.source_hash,
                "module_count": len(modules),
                "warnings": locator.warnings + manager.warnings,
            })
            return 0

        if args.command == "version":
            response: dict[str, Any] = {"uef_gen_version": __version__}
            try:
                locator, manager, snapshot = _uef_snapshot(Path.cwd())
                response.update({"uef_version": snapshot.version, "uef_root": str(snapshot.root)})
                response["warnings"] = locator.warnings + manager.warnings
            except Exception as exc:
                response["uef_version"] = None
                response["uef_status"] = str(exc)
            _emit(response)
            return 0
    except Exception as exc:
        _emit({"ok": False, "diagnostics": [
            error("cli_operation_failed", str(exc)).to_dict()
        ]})
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
    if arguments and arguments[0] in {
        "generate", "validate", "targets", "modules", "snapshot", "version"
    }:
        return _run_command(arguments)
    return _run_legacy(arguments)


if __name__ == "__main__":
    raise SystemExit(main())
