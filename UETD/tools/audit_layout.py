"""Check UETD's authored folder layout and module-route ownership.

Generated build/IDE state, Python caches and intact third-party packages have
their own layouts. U* layer roots may contain a single file; other authored
folders must group multiple files or act purely as parent folders.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
LAYERS = frozenset({"uapp", "uactuator", "ucon", "ucore", "uhal", "umath",
                    "umid", "uos", "upal", "uphysics", "uproto", "usec"})
EXCLUDED = frozenset({".git", ".vs", "out", "build", "dist", "third_party",
                      "__pycache__", "node_modules", ".pytest_cache"})


def audit_layout(root: Path = ROOT, excluded_paths: tuple[str, ...] = ()) -> list[str]:
    def excluded(relative: str) -> bool:
        return any(relative == value or relative.startswith(value + "/") for value in excluded_paths)

    errors = []
    for folder, directories, files in os.walk(root):
        path = Path(folder)
        directories[:] = sorted(name for name in directories if name not in EXCLUDED
                                and not excluded((path / name).relative_to(root).as_posix()))
        if path != root and len(files) == 1 and path.name not in LAYERS:
            errors.append(f"single-file folder: {path.relative_to(root).as_posix()} ({files[0]})")
        if path != root and not directories and not files:
            errors.append(f"empty authored folder: {path.relative_to(root).as_posix()}")

    module_file = root / "registry/modules/registry.json"
    route_file = root / "templates/modules/index.json"
    if module_file.is_file() and route_file.is_file():
        modules = json.loads(module_file.read_text(encoding="utf-8"))["modules"]
        routes = json.loads(route_file.read_text(encoding="utf-8"))["modules"]
        for name, reference in modules.items():
            manifest = PurePosixPath(reference["manifest"])
            relative = manifest.relative_to("registry/modules/manifest").with_suffix(".cmake.tmpl")
            expected = (PurePosixPath("templates/modules") / relative).as_posix()
            if excluded(manifest.as_posix()) or excluded(expected):
                continue
            route = routes.get(name, {})
            if route.get("template") != expected:
                errors.append(f"{name}: route must follow its manifest topic folder ({expected})")
            target = root / expected
            if not target.is_file():
                errors.append(f"{name}: route is missing ({expected})")
            elif hashlib.sha256(target.read_bytes()).hexdigest() != route.get("template_sha256"):
                errors.append(f"{name}: generated route differs from its ownership hash")
    return errors


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exclude", action="append", default=[], metavar="RELATIVE_PATH",
                        help="exclude a separately maintained subtree from this audit")
    args = parser.parse_args()
    findings = audit_layout(excluded_paths=tuple(value.replace("\\", "/").rstrip("/") for value in args.exclude))
    for finding in findings:
        print(f"ERROR: {finding}")
    print(f"Folder layout errors: {len(findings)}")
    raise SystemExit(bool(findings))
