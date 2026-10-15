"""Read-only package listing and the future UETD-pinned bootstrap command."""

from __future__ import annotations
import argparse
from pathlib import Path
from typing import Any
from uef_gen.uetd_adapter.locator import UETDLocator
from uef_gen.toolchains.bootstrap import ToolchainBootstrapper, detect_host_platform
from uef_gen.toolchains.catalog import load_toolchain_catalogue

def add_tool_commands(commands: Any) -> None:
    """Add ``uef-gen tools`` commands to the main CLI parser."""
    tools = commands.add_parser("tools", help="inspect or provision UETD-pinned host tools")
    subcommands = tools.add_subparsers(dest="tools_command", required=True)
    subcommands.add_parser("list", help="list UETD tool package records and readiness")
    install = subcommands.add_parser("install", help="install one ready UETD-pinned package")
    install.add_argument("package_id")
    install.add_argument("--root", type=Path, help="local install root; defaults to the user tool cache")
    install.add_argument("--host", help="override host key for packaging/bootstrap development")
    install.add_argument("--plan", action="store_true", help="print the package plan without installing")

def handle_tool_command(args: Any) -> tuple[dict[str, Any], int]:
    """Run a registry listing or a future install operation without shell side effects."""
    uetd_root = UETDLocator(Path.cwd()).locate()
    records = load_toolchain_catalogue(uetd_root)
    if args.tools_command == "list":
        return (
            {
                "uetd_root": str(uetd_root),
                "host_platform": detect_host_platform(),
                "packages": [
                    {"id": key, "name": value["name"], "purpose": value["purpose"], "status": value["status"], "host_platforms": value["host_platforms"]}
                    for key, value in sorted(records.items())
                ],
            },
            0,
        )
    root = args.root or Path.home() / ".uef" / "tools"
    bootstrapper = ToolchainBootstrapper(records, root)
    plan = bootstrapper.plan(args.package_id, args.host)
    if args.tools_command == "install" and args.plan:
        return ({"install_root": str(root.resolve()), "plan": [step.__dict__ for step in plan]}, 0)
    raise NotImplementedError(
        "TODO(tools-install-cli): render a plan summary, request explicit consent for installer/license "
        "steps, invoke ToolchainBootstrapper.install, and report each installed executable and receipt."
    )
