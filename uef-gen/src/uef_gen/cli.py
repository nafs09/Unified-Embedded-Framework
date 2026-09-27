"""Command-line entry point for standalone and NEXUS contract requests."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

from uef_gen.config.loader import load_configuration
from uef_gen.contract import handle_request


CONTRACT_VERSION = "1.0"


def _read_json(path: Path) -> Any:
    """Read one UTF-8 JSON document from disk."""
    return json.loads(path.read_text(encoding="utf-8"))


def _argument_parser() -> argparse.ArgumentParser:
    """Build CLI options without performing file or hardware operations."""
    parser = argparse.ArgumentParser(
        prog="uef-gen",
        description="Assemble a UEF embedded project from hardware configuration.",
    )
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument(
        "--request",
        type=Path,
        help="Versioned JSON contract request; stdin is used when omitted",
    )
    mode.add_argument(
        "--config",
        type=Path,
        help="Standalone project.yaml or project.json hardware configuration",
    )
    parser.add_argument("--control-ir", type=Path, help="Legacy transitional ControlIR JSON")
    parser.add_argument("--output", type=Path, help="Generated project output directory")
    parser.add_argument("--work", type=Path, help="Isolated staging/work directory")
    return parser


def main() -> int:
    """Parse one request and emit exactly one machine-readable JSON response."""
    # Keep the original generation entry point stable while exposing the
    # separately scaffolded Part XIX build/deployment commands.
    if len(sys.argv) > 1 and sys.argv[1] in {"build", "deploy", "probe", "flash"}:
        from uef_gen.deploy.cli import main as deployment_main

        return deployment_main(sys.argv[1:])

    parser = _argument_parser()
    args = parser.parse_args()

    try:
        if args.config:
            if args.output is None or args.work is None:
                parser.error("--config requires --output and --work")

            request = {
                "contract_version": CONTRACT_VERSION,
                "operation": "generate",
                "hardware_configuration": load_configuration(args.config),
                "control_ir": _read_json(args.control_ir) if args.control_ir else None,
                "output_directory": str(args.output.resolve()),
                "work_directory": str(args.work.resolve()),
            }
        else:
            raw = args.request.read_text(encoding="utf-8") if args.request else sys.stdin.read()
            request = json.loads(raw)

        response = handle_request(request)
    except Exception as exc:
        # Keep stdout protocol-only so an app bridge can always parse one response.
        response = {
            "contract_version": CONTRACT_VERSION,
            "ok": False,
            "diagnostics": [
                {
                    "severity": "error",
                    "code": "request_failed",
                    "message": str(exc),
                }
            ],
        }

    print(json.dumps(response, separators=(",", ":"), ensure_ascii=False))
    return 0 if response.get("ok") else 2


if __name__ == "__main__":
    raise SystemExit(main())
