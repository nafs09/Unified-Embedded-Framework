"""Check catalogue integrity and export a source-evidence inventory (stdlib only).

Source markers are evidence of scaffolding, never proof of implementation quality.
Run after registry maintenance and regenerate the two derived catalogues separately.
"""
from __future__ import annotations

import argparse
import collections
import csv
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def audit_public_declarations(root: Path) -> list[str]:
    """Find orphan public entry points; existence is not an ABI/readiness proof."""
    prefix = r"(?:ucore|uhal|upal|uos|umid|uapp|ucon|umath|uproto|uactuator|usec)_\w+"
    signature = (r"^[ \t]*(?:(?:static|inline|extern|const|UCORE_WEAK|UHAL_WEAK|UETD_WEAK)\s+)*"
                 r"(?:[A-Za-z_]\w*(?:[ \t]+|[ \t]*\*[ \t]*))+?(" + prefix + r")\s*\([^;{}]*\)\s*")
    declarations = re.compile(signature + r";", re.MULTILINE)
    definitions = re.compile(signature + r"\{", re.MULTILINE)
    declared: dict[str, set[str]] = collections.defaultdict(set)
    defined: set[str] = set()
    for path in [*(root / "src").rglob("*.c"), *(root / "include").rglob("*.h")]:
        text = path.read_text(encoding="utf-8-sig")
        # Function-pointer typedefs can resemble prototypes to the inventory
        # scanner; they declare a type, not another external function.
        text = re.sub(r"\btypedef\s+[^;{}]+;", "", text)
        defined.update(definitions.findall(text))
        if path.suffix == ".h":
            for symbol in declarations.findall(text):
                declared[symbol].add(path.relative_to(root).as_posix())
    return [f"public declaration {symbol}: no source/inline definition ({', '.join(sorted(declared[symbol]))})"
            for symbol in sorted(declared.keys() - defined)]


def audit(root: Path) -> tuple[list[dict], list[str]]:
    index = read(root / "registry/modules/registry.json")
    modules = {name: read(root / entry["manifest"]) for name, entry in index["modules"].items()}
    errors: list[str] = audit_public_declarations(root)
    rows: list[dict] = []
    descriptors: dict[str, str] = {}

    def closure(name: str, visiting: tuple[str, ...] = ()) -> set[str]:
        if name in visiting:
            raise ValueError(f"dependency cycle: {' -> '.join((*visiting, name))}")
        if name not in modules:
            raise ValueError(f"unknown module dependency: {name}")
        result = {name}
        for dependency in modules[name].get("dependencies", []):
            result.update(closure(dependency, (*visiting, name)))
        return result

    for name, record in modules.items():
        entry = index["modules"][name]
        descriptor = entry.get("DSL_descriptor", "")
        if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", descriptor):
            errors.append(f"{name}: missing/invalid DSL descriptor")
        if descriptor in descriptors:
            errors.append(f"duplicate descriptor {descriptor}: {descriptors[descriptor]}, {name}")
        descriptors[descriptor] = name
        sources = record.get("sources", [])
        headers = record.get("headers", [])
        for resource in [*sources, *headers]:
            path = (root / resource).resolve()
            if not path.is_relative_to(root.resolve()) or not path.is_file():
                errors.append(f"{name}: missing/unsafe resource {resource}")
        contract = record.get("design_contract")
        if contract:
            path = (root / contract.split("#", 1)[0]).resolve()
            if not path.is_relative_to(root.resolve()) or not path.is_file():
                errors.append(f"{name}: missing/unsafe design contract {contract}")
        texts = [(root / source).read_text(encoding="utf-8-sig") for source in sources if (root / source).is_file()]
        source_scaffold = any("Source scaffold" in text for text in texts)
        stub_returns = sum(len(re.findall(r"return\s+(?:UMATH_NOT_IMPLEMENTED|UCON_NOT_IMPLEMENTED)\s*;", text)) for text in texts)
        blocked = record.get("scaffold_only") is True or record.get("available_for_generation") is False
        if source_scaffold and not blocked:
            errors.append(f"{name}: source is a scaffold but module selection is not blocked")
        evidence = "source_scaffold" if source_scaffold else "fail_closed_operations" if stub_returns else "source_present" if sources else "metadata_or_header_only"
        if any("Validated dispatch" in text or "Validated provider" in text for text in texts):
            evidence = "provider_dispatch"
        selection = "blocked_by_manifest" if blocked else "allowed_by_module_flags"
        try:
            selected = closure(name)
            declared_headers = {header for module in selected for header in modules[module].get("headers", [])}
            pending = [*sources, *headers]
            included: set[str] = set()
            while pending:
                resource = pending.pop()
                if resource in included or not (root / resource).is_file():
                    continue
                included.add(resource)
                text = (root / resource).read_text(encoding="utf-8-sig")
                for include in re.findall(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', text, re.MULTILINE):
                    header = f"include/{include}"
                    if (root / header).is_file():
                        pending.append(header)
            absent = sorted(header for header in included if header.startswith("include/") and header not in declared_headers)
            if absent:
                errors.append(f"{name}: incomplete header closure: {', '.join(absent)}")
            if not blocked and any(modules[x].get("scaffold_only") is True or modules[x].get("available_for_generation") is False for x in selected):
                selection = "blocked_by_dependency"
        except ValueError as exc:
            errors.append(f"{name}: {exc}")
            selection = "invalid_dependency_graph"
        authoring = record.get("authoring", {})
        rows.append({
            "module_id": name, "descriptor": descriptor, "layer": name.split("/")[0],
            "source_evidence": evidence, "not_implemented_returns": stub_returns,
            "scaffold_only": record.get("scaffold_only", "unspecified"),
            "available_for_generation": record.get("available_for_generation", "unspecified"),
            "module_selection": selection, "contract_status": record.get("contract_status", "unspecified"),
            "authoring_status": authoring.get("status", "missing"),
            "authoring_generation": authoring.get("generation", {}).get("status", "missing"),
            "priority": record.get("priority", ""), "algorithm": record.get("algorithm", ""),
            "sources": ";".join(sources), "headers": ";".join(headers),
            "dependencies": ";".join(record.get("dependencies", [])),
            "external_dependencies": ";".join(record.get("external_dependencies", [])),
            "manifest": entry["manifest"], "reason": record.get("scaffold_reason", ""),
        })

    routes = read(root / "templates/modules/index.json")
    if set(routes["modules"]) != set(modules):
        errors.append("module build routes differ from the canonical module index; regenerate_module_templates")
    for name, route in routes["modules"].items():
        if name not in modules:
            continue
        for field in ("sources", "headers", "dependencies"):
            if route.get(field, []) != modules[name].get(field, []):
                errors.append(f"{name}: generated route has stale {field}")
        if not (root / route["template"]).is_file():
            errors.append(f"{name}: missing generated route")

    for kind in ("families", "chips", "development_boards", "toolchains"):
        target_index = read(root / f"registry/targets/{kind}/registry.json")
        seen: set[str] = set()
        for entry in target_index["entries"]:
            if entry["id"] in seen:
                errors.append(f"duplicate {kind} ID: {entry['id']}")
            seen.add(entry["id"])
            path = root / entry["path"]
            if not path.is_file() or read(path).get("id") != entry["id"]:
                errors.append(f"{kind}: missing/mismatched record {entry['id']}")

    planning_ids: set[str] = set()
    for scope in ("first_pass", "generation", "future"):
        catalogue = read(root / f"registry/{scope}/registry.json")
        for name, entry in catalogue["items"].items():
            if name in planning_ids:
                errors.append(f"design item {name} appears in both scope catalogues")
            planning_ids.add(name)
            if entry["status"] != "design_only" or entry["generation"] != "blocked":
                errors.append(f"{scope} item {name} must remain design_only and generation-blocked")
            if scope != "future" and (entry.get("priority") != "FIRST_PASS" or entry.get("phase") != 0):
                errors.append(f"{scope} item {name} must declare FIRST_PASS and phase 0")
            for schema_id, relative in catalogue.get("schemas", {}).items():
                if not (root / relative).is_file():
                    errors.append(f"{scope} schema {schema_id}: missing {relative}")
            for field in ("contract_file", "api_outline"):
                reference = entry.get(field)
                if reference:
                    base = root.parent if reference.startswith("@repo/") else root
                    relative = reference.removeprefix("@repo/").split("#", 1)[0]
                    if not (base / relative).is_file():
                        errors.append(f"{scope} item {name}: missing {field} {reference}")
    return rows, errors


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory", type=Path, help="write one CSV row per module")
    args = parser.parse_args()
    rows, errors = audit(ROOT)
    if args.inventory:
        args.inventory.parent.mkdir(parents=True, exist_ok=True)
        with args.inventory.open("w", encoding="utf-8", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
    print(f"Modules: {len(rows)}; layers: {dict(collections.Counter(row['layer'] for row in rows))}")
    print(f"Selection: {dict(collections.Counter(row['module_selection'] for row in rows))}")
    for error in errors:
        print(f"ERROR: {error}")
    print(f"Catalogue integrity errors: {len(errors)}")
    raise SystemExit(bool(errors))


if __name__ == "__main__":
    main()
