"""Command-line surface for future build and target operations.

The parser is intentionally present before the operations are enabled so the
CLI shape can be reviewed independently of compiler or hardware side effects.
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
    build.add_argument("project", help="project.yaml or generated project directory")
    build.add_argument("--output", required=True, help="artifact output directory")

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
    flash.add_argument("project", help="project configuration or generated project directory")
    flash.add_argument("--output", required=True, help="artifact output directory")
    flash.add_argument("--backend", default="auto")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    """Return a machine-readable pending-operation result until implementations land."""
    args = _parser().parse_args(argv)
    operation = args.command
    if operation == "probe":
        operation = f"probe {args.probe_command}"

    # This dispatcher makes the documented CLI visible, but does not pretend its
    # compiler, discovery, artifact, or deployment operations are implemented.
    response = {
        "ok": False,
        "operation": operation,
        "diagnostics": [
            {
                "severity": "error",
                "code": "operation_not_implemented",
                "message": (
                    f"{operation} is scaffolded but unavailable; see TODO.md before using "
                    "a compiler or physical target."
                ),
            }
        ],
    }
    print(json.dumps(response, separators=(",", ":")))
    return 2
