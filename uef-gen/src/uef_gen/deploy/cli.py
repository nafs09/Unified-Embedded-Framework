"""Build, probe, flash, and wired-monitor command scaffolds.

The parser is intentionally usable before compiler or hardware side effects
are enabled. Each operation has a named handler so the intended lifecycle and
implementation order are visible without pretending the operation works.
"""

from __future__ import annotations
import argparse
import json
from collections.abc import Sequence

def _parser() -> argparse.ArgumentParser:
    """Describe build/deploy/probe commands without opening files or devices."""
    parser = argparse.ArgumentParser(prog="uef-gen")
    commands = parser.add_subparsers(dest="command", required=True)
    build = commands.add_parser("build", help="build a generated firmware project")
    build.add_argument("project", help=".uef source, legacy project.yaml/json, or generated project directory")
    build.add_argument("--output", help="artifact output directory (default: <project>/build/artifacts)")
    deploy = commands.add_parser("deploy", help="deploy a previously built artifact")
    deploy.add_argument("project", nargs="?", help="project.yaml used to identify/build the artifact")
    deploy.add_argument("--artifact", help="existing artifact metadata sidecar")
    deploy.add_argument("--output", help="artifact output directory when building first")
    deploy.add_argument("--backend", default="auto")
    deploy.add_argument("--probe-serial", default="")
    deploy.add_argument("--no-verify", action="store_true")
    probe = commands.add_parser("probe", help="inspect connected programming targets")
    probe_commands = probe.add_subparsers(dest="probe_command", required=True)
    probe_commands.add_parser("list", help="list discoverable probes")
    identify = probe_commands.add_parser("identify", help="read chip identity from a selected probe")
    identify.add_argument("--probe-serial", required=True)
    flash = commands.add_parser("flash", help="build and deploy a project")
    flash.add_argument("project", help=".uef source, legacy project.yaml/json, or generated project directory")
    flash.add_argument("--output", help="artifact output directory (default: <project>/build/artifacts)")
    flash.add_argument("--backend", default="auto")
    monitor = commands.add_parser("monitor", help="read a board's wired debug console")
    monitor.add_argument("project", help=".uef source, legacy project.yaml/json, or generated project directory")
    monitor.add_argument("--board", help="UETD development-board ID when not selected in project.yaml")
    monitor.add_argument("--port", help="explicit host serial port, such as COM5 or /dev/ttyUSB0")
    monitor.add_argument("--log-file", help="append received output to this UTF-8 log file")
    monitor.add_argument("--timestamps", action="store_true")
    return parser

def _pending(operation: str, todo: str, implementation: str) -> int:
    """Emit a stable diagnostic while an operation remains a source scaffold."""
    response = {
        "ok": False,
        "operation": operation,
        "diagnostics": [{"severity": "error", "code": "operation_not_implemented", "todo": todo, "message": implementation}],
    }
    print(json.dumps(response, separators=(",", ":")))
    return 2

def _handle_build(args: argparse.Namespace) -> int:
    """Resolve project lists, validate a target toolchain, then build artifacts.

    TODO(cli-build): compile .uef input through project_language when supplied;
    otherwise load the project/generated-project manifest; resolve its board
    and compiler profile from UETD; validate all paths/flags before process
    launch; default output under the generated project's build/artifacts;
    call BuildManager; persist BuildResult through ArtifactStore; and
    return stable JSON diagnostics with build output paths and budgets.
    """
    del args
    return _pending("build", "build-pipeline", "Build is scaffolded but unavailable.")

def _handle_probe_list(args: argparse.Namespace) -> int:
    """Perform read-only enumeration of probes and explicitly supported bootloaders."""
    del args
    return _pending("probe list", "target-enumeration", "Probe discovery is scaffolded but unavailable; enumeration must remain read-only.")

def _handle_probe_identify(args: argparse.Namespace) -> int:
    """Read silicon identity over the selected probe without changing target state."""
    del args
    return _pending("probe identify", "target-identification", "Target identification is scaffolded but unavailable; no erase/reset is performed.")

def _handle_deploy(args: argparse.Namespace) -> int:
    """Load a non-stale artifact, require exact target selection, and deploy safely."""
    del args
    return _pending("deploy", "deployment-sequence", "Deployment is scaffolded but unavailable; no target write operation is performed.")

def _handle_flash(args: argparse.Namespace) -> int:
    """Compile/generate/build, verify artifact and target identity, then deploy.

    TODO(cli-flash): compile .uef input through project_language; use the same
    generation/build lifecycle; verify artifact source/UETD/toolchain hashes and
    exact target identity; then select the UETD-declared backend. Do not erase or
    program before every check passes.
    """
    del args
    return _pending("flash", "flash-build-deploy-sequence", "Flash is scaffolded but unavailable; build and deployment are not run.")

def _handle_monitor(args: argparse.Namespace) -> int:
    """Attach to a UETD-described wired console and stream output without resetting it."""
    del args
    return _pending("monitor", "console-run", "The wired monitor is scaffolded but unavailable; no serial port is opened.")

def main(argv: Sequence[str] | None = None) -> int:
    """Dispatch named operation scaffolds and preserve a machine-readable CLI contract."""
    args = _parser().parse_args(argv)
    if args.command == "build":
        return _handle_build(args)
    if args.command == "deploy":
        return _handle_deploy(args)
    if args.command == "probe" and args.probe_command == "list":
        return _handle_probe_list(args)
    if args.command == "probe" and args.probe_command == "identify":
        return _handle_probe_identify(args)
    if args.command == "flash":
        return _handle_flash(args)
    if args.command == "monitor":
        return _handle_monitor(args)
    raise AssertionError(f"unhandled deployment CLI operation: {args.command}")
