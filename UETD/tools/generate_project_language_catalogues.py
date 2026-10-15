#!/usr/bin/env python3
"""Generate UEF-PL discovery indexes and registry reference documentation.

This is an indexer, not a compiler or general C parser. It preserves public
declaration facts for editor/reference discovery and maintains UETD-owned UEF-PL
operation names and binding contracts. The shared project frontend performs
registry-driven, type-directed overload selection. This tool updates generated
type and UMATH indexes plus catalogue/unit reference pages; it does not rewrite
the human-maintained user handbook or technical guide.
"""

from __future__ import annotations
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
UETD = ROOT / "UETD"
INCLUDE = UETD / "include"
REGISTRY = UETD / "registry"

def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))

def strip_comments_and_directives(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    lines: list[str] = []
    skip_continuation = False
    for line in text.splitlines():
        if skip_continuation:
            skip_continuation = line.rstrip().endswith("\\")
            continue
        if line.lstrip().startswith("#"):
            skip_continuation = line.rstrip().endswith("\\")
            continue
        lines.append(line)
    return "\n".join(lines)

def typedef_statements(text: str):
    """Yield balanced typedef declarations, including anonymous structs/enums."""
    for match in re.finditer(r"\btypedef\b", text):
        start = match.start()
        brace = paren = bracket = 0
        for index in range(match.end(), len(text)):
            char = text[index]
            if char == "{":
                brace += 1
            elif char == "}":
                brace -= 1
            elif char == "(":
                paren += 1
            elif char == ")":
                paren -= 1
            elif char == "[":
                bracket += 1
            elif char == "]":
                bracket -= 1
            elif char == ";" and brace == 0 and paren == 0 and bracket == 0:
                yield text[start : index + 1].strip()
                break

def typedef_name(statement: str) -> tuple[str | None, str]:
    compound = re.search(r"\b(struct|union|enum)\b(?:\s+[A-Za-z_]\w*)?\s*\{.*\}\s*([A-Za-z_]\w*)\s*;\s*$", statement, flags=re.S)
    if compound:
        return compound.group(2), compound.group(1)
    pointer = re.search(r"\(\s*\*\s*([A-Za-z_]\w*)\s*\)", statement)
    if pointer:
        return pointer.group(1), "function_pointer"
    body = statement.removeprefix("typedef").removesuffix(";").strip()
    candidate = re.search(r"([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*$", body)
    if not candidate:
        return None, "unknown"
    return candidate.group(1), "alias"

def enum_members(statement: str) -> list[str]:
    body = re.search(r"\benum\b(?:\s+[A-Za-z_]\w*)?\s*\{(.*)\}", statement, flags=re.S)
    if not body:
        return []
    members: list[str] = []
    for item in body.group(1).split(","):
        match = re.match(r"\s*([A-Za-z_]\w*)", item)
        if match:
            members.append(match.group(1))
    return members

def collect_types(
    headers: list[Path], previous_entries: dict[tuple[str, str], dict[str, Any]] | None = None
) -> list[dict[str, Any]]:
    """Index C declarations and retain UETD-authored language type contracts.

    The C declaration remains the ABI source of truth. ``language_contract``
    only states how the DSL may refer to that declaration; it does not copy or
    reinterpret struct fields. Manual DSL names/contracts survive regeneration
    when the exact C type and header are unchanged.
    """
    previous_entries = previous_entries or {}
    found: dict[tuple[str, str, str], dict[str, Any]] = {}
    fixed_aliases = {
        "uetd_f32_t",
        "uetd_f64_t",
        "uetd_q15_t",
        "uetd_q31_t",
        "uetd_q63_t",
        "uetd_q16_t",
        "uetd_u8_t",
        "uetd_u16_t",
        "uetd_u32_t",
        "uetd_u64_t",
        "uetd_i8_t",
        "uetd_i16_t",
        "uetd_i32_t",
        "uetd_i64_t",
    }
    for header in headers:
        relative = header.relative_to(UETD).as_posix()
        clean = strip_comments_and_directives(header.read_text(encoding="utf-8-sig"))
        for statement in typedef_statements(clean):
            name, kind = typedef_name(statement)
            if not name:
                continue
            status = "future_header" if "/future/" in relative else "public_header"
            previous = previous_entries.get((name, relative), {})
            if status == "future_header" or kind == "function_pointer":
                default_contract = None
                visibility = "future-only type; unavailable to UEF-PL" if status == "future_header" else "function pointer; unavailable to UEF-PL"
            elif name in fixed_aliases:
                default_contract = "numeric"
                visibility = "registered numeric type"
            elif kind == "enum":
                default_contract = "enum"
                visibility = "registered enum; enumerators are available by qualified name"
            else:
                default_contract = "opaque"
                visibility = "registered opaque type; C fields and constructors are not exposed"
            key = (name, relative, statement)
            type_entry = {
                "c_name": name,
                "kind": kind,
                "header": relative,
                "declaration": re.sub(r"\s+", " ", statement).strip(),
                "enum_members": enum_members(statement) if kind == "enum" else [],
                "language_visibility": visibility,
                "source_status": status,
            }
            contract = previous.get("language_contract", default_contract) if default_contract is not None else None
            if contract is not None:
                type_entry["language_contract"] = contract
            if name in fixed_aliases or isinstance(previous.get("dsl_name"), str):
                # Keep the DSL spelling beside the C declaration it names;
                # the authoring tool must not reconstruct it from convention.
                type_entry["dsl_name"] = previous.get("dsl_name") or name[4:-2]
            vector_dimension = previous.get("vector_dimension")
            if isinstance(vector_dimension, int) and not isinstance(vector_dimension, bool):
                # Vector shape is an explicit UETD-authored representation
                # contract, not something inferred from C member spelling.
                type_entry["vector_dimension"] = vector_dimension
            found[key] = type_entry
    return sorted(found.values(), key=lambda item: (item["c_name"], item["header"]))

def collect_functions(
    headers: list[Path], module_index: dict[str, Any], api: dict[str, Any], previous_entries: dict[tuple[str, str], dict[str, Any]],
    type_entries: list[dict[str, Any]] | None = None,
) -> list[dict[str, Any]]:
    header_to_modules: dict[str, list[str]] = {}
    for module_id, reference in module_index.get("modules", {}).items():
        manifest_path = UETD / reference["manifest"]
        if not manifest_path.exists():
            continue
        manifest = load_json(manifest_path)
        for item in manifest.get("headers", []):
            header_to_modules.setdefault(item.replace("\\", "/"), []).append(module_id)
    umath_api = api.get("umath_api", {})
    implemented = set(umath_api.get("implemented_modules", []))
    phase0 = set(umath_api.get("phase0_scaffold_modules", []))
    phase1 = set(umath_api.get("phase1_scaffold_modules", []))
    future = set(umath_api.get("future_scaffold_modules", []))
    pattern = re.compile(
        r"(?m)^[ \t]*(?!typedef\b)(?P<signature>(?:(?:static|inline|extern)\s+)*"
        r"(?:const\s+)?[A-Za-z_]\w*(?:\s+const)?(?:\s*\*)?\s+"
        r"(?P<name>umath_[A-Za-z0-9_]+)\s*\([^;{}]*?\))(?P<terminator>\s*;|\s*\{)"
    )
    functions: dict[tuple[str, str], dict[str, Any]] = {}
    family_prefixes: set[str] = set()
    for item in type_entries or []:
        c_name = item.get("c_name", "")
        if item.get("kind") in {"alias", "struct"}:
            match = re.fullmatch(r"(?:umath|uetd)_([A-Za-z0-9_]+)_t", c_name)
            if match:
                family_prefixes.add(match.group(1))
    discovered_names = []
    for header in headers:
        relative = header.relative_to(UETD).as_posix()
        if "/internal/" in relative:
            continue
        clean = strip_comments_and_directives(header.read_text(encoding="utf-8-sig"))
        discovered_names.extend(re.findall(r"\b(umath_[A-Za-z0-9_]+)\s*\(", clean))
    prefix_counts: dict[str, int] = {}
    for name in discovered_names:
        base = name.removeprefix("umath_")
        first = base.split("_", 1)[0]
        prefix_counts[first] = prefix_counts.get(first, 0) + 1
    family_prefixes.update(name for name, count in prefix_counts.items() if count > 1)
    for header in headers:
        relative = header.relative_to(UETD).as_posix()
        if "/internal/" in relative:
            continue
        clean = strip_comments_and_directives(header.read_text(encoding="utf-8-sig"))
        for match in pattern.finditer(clean):
            name = match.group("name")
            declaration = re.sub(r"\s+", " ", match.group("signature")).strip()
            terminator = match.group("terminator").strip()
            signature = declaration + (";" if terminator == ";" else " { ... }")
            candidates = header_to_modules.get(relative, [])
            module_id = None
            path_stem = relative.removeprefix("include/").removesuffix(".h")
            exact = [candidate for candidate in candidates if candidate == path_stem]
            if exact:
                module_id = exact[0]
            elif "umath/core" in candidates and (relative.endswith("/scalar.h") or relative.endswith("/status.h")):
                module_id = "umath/core"
            elif candidates:
                module_id = sorted(candidates, key=lambda value: (value.count("/"), value))[0]
            if module_id in implemented:
                implementation_status = "implemented"
            elif module_id in phase0 or module_id in phase1:
                implementation_status = "scaffold"
            elif module_id in future or "/future/" in relative:
                implementation_status = "future"
            else:
                implementation_status = "registry_review_required"
            parameters = declaration[declaration.find("(") + 1 : declaration.rfind(")")].strip()
            previous = previous_entries.get((name, relative), {})
            # The canonical UEF-PL operation is the symbol with its registered
            # family/type prefix removed. Family-qualified spellings remain aliases
            # so existing projects keep compiling. The compiler resolves
            # same-name entries as typed overloads from the registry; it does
            # not contain per-family dispatch rules.
            qualified_name = name.removeprefix("umath_")
            dsl_name = _umath_operation_name(name, family_prefixes)
            aliases = previous.get("dsl_aliases", [])
            aliases = aliases if isinstance(aliases, list) else []
            legacy_name = previous.get("dsl_name")
            legacy_suffix = legacy_name == name.removeprefix("umath_").rsplit("_", 1)[-1]
            dsl_aliases = list(dict.fromkeys([
                *[value for value in aliases if isinstance(value, str) and value != dsl_name],
                *([qualified_name] if qualified_name != dsl_name else []),
                *([legacy_name] if isinstance(legacy_name, str) and legacy_name != dsl_name and not legacy_suffix else []),
            ]))
            binding_status = previous.get("dsl_binding_status", "signature_only")
            binding_contract = previous.get("dsl_binding_contract")
            binding_ready = (
                binding_status == "ready"
                and isinstance(previous.get("dsl_signature"), str)
                and bool(previous.get("dsl_signature"))
                and isinstance(binding_contract, dict)
                and all(key in binding_contract for key in ("signature", "parameters", "result", "failure", "availability", "bounded_cost"))
                and isinstance(binding_contract.get("parameters"), list)
                and all(
                    isinstance(parameter, dict)
                    and isinstance(parameter.get("name"), str)
                    and isinstance(parameter.get("type"), (str, dict))
                    and parameter.get("mode") in {"value", "borrow", "mutable_borrow"}
                    for parameter in binding_contract["parameters"]
                )
                and (
                    binding_contract.get("result") is None
                    or isinstance(binding_contract.get("result"), dict) and "type" in binding_contract["result"]
                )
            )
            functions[(name, relative)] = {
                "c_name": name,
                "dsl_name": dsl_name,
                "dsl_aliases": dsl_aliases,
                "dsl_signature": previous.get("dsl_signature"),
                "module_id": module_id,
                "header": relative,
                "c_signature": signature,
                "parameters": parameters,
                "declaration_kind": "inline_definition" if terminator == "{" else "prototype",
                "implementation_status": implementation_status,
                "dsl_binding_status": binding_status,
                "dsl_binding_contract": binding_contract,
                "available_for_generation": implementation_status == "implemented" and binding_ready,
                "callable_contexts": ["custom_algorithm", "model_equation"],
            }
    values = sorted(functions.values(), key=lambda item: (item["module_id"] or "", item["c_name"], item["header"]))
    return values

def _umath_operation_name(c_name: str, family_prefixes: set[str]) -> str:
    """Remove a registered UMATH type/family prefix, preserving the full operation verb."""
    base = c_name.removeprefix("umath_")
    matches = [prefix for prefix in family_prefixes if base.startswith(prefix + "_")]
    if not matches:
        return base
    family = max(matches, key=len)
    operation = base[len(family) + 1 :]
    return operation or base

def escape_cell(value: str) -> str:
    return value.replace("|", r"\|").replace("\n", " ")

def format_unit_scale_latex(unit: dict[str, Any]) -> str:
    """Render exact unit mappings with Markdown math for the unit reference."""
    exact = unit.get("exact_scale")
    if isinstance(exact, dict):
        numerator = exact.get("numerator", "1")
        denominator = exact.get("denominator", "1")
        scale = str(numerator) if str(denominator) == "1" else rf"\frac{{{numerator}}}{{{denominator}}}"
        power = int(exact.get("pi_power", 0))
        if power == 1:
            scale += r"\,\pi"
        elif power == -1:
            scale += r"/\pi"
        elif power:
            scale += rf"\,\pi^{{{power}}}"
    else:
        scale = str(unit.get("scale_to_si", "unspecified"))
    offset = unit.get("offset_to_si")
    if unit.get("kind") == "affine" or offset is not None:
        return rf"\(x_{{\mathrm{{SI}}}} = x \cdot {scale} + {offset or '0'}\)"
    return rf"\(\times {scale}\)"


_DIMENSION_LATEX = {
    "length": r"\mathrm{L}",
    "mass": r"\mathrm{M}",
    "time": r"\mathrm{T}",
    "current": r"\mathrm{I}",
    "temperature": r"\Theta",
    "amount": r"\mathrm{N}",
    "luminous_intensity": r"\mathrm{J}",
    "angle": r"\mathrm{A}_{\mathrm{angle}}",
    "count": r"\mathrm{N}_{\mathrm{count}}",
    "information": r"\mathrm{B}_{\mathrm{info}}",
    "symbol": r"\mathrm{S}_{\mathrm{symbol}}",
}

_DIMENSION_LABELS = {
    (("length", 1),): "Length",
    (("mass", 1),): "Mass",
    (("time", 1),): "Time",
    (("current", 1),): "Electric current",
    (("temperature", 1),): "Temperature",
    (("amount", 1),): "Amount of substance",
    (("luminous_intensity", 1),): "Luminous intensity",
    (("angle", 1),): "Angle",
    (("count", 1),): "Count",
    (("information", 1),): "Information",
    (("symbol", 1),): "Symbol count",
    (("time", -1),): "Frequency",
    (("symbol", 1), ("time", -1)): "Symbol rate",
    (("current", 1), ("time", 1)): "Electric charge",
    (("length", 2), ("mass", 1), ("time", -2)): "Energy and work",
    (("length", 1), ("mass", 1), ("time", -2)): "Force",
    (("length", -1), ("mass", 1), ("time", -2)): "Pressure",
    (("current", -1), ("mass", 1), ("time", -2)): "Magnetic flux density",
    (("current", -1), ("length", 2), ("mass", 1), ("time", -3)): "Electric potential",
    (("current", -2), ("length", 2), ("mass", 1), ("time", -3)): "Electrical resistance",
    (("length", 2), ("mass", 1), ("time", -3)): "Power",
    (("current", -1), ("length", 2), ("mass", 1), ("time", -2)): "Magnetic flux",
    (("current", -2), ("length", 2), ("mass", 1), ("time", -2)): "Inductance",
    (("current", 2), ("length", -2), ("mass", -1), ("time", 4)): "Capacitance",
    (("angle", 1), ("time", -1)): "Angular rate",
}
_DIMENSION_ORDER = {
    "Length": 0,
    "Mass": 1,
    "Time": 2,
    "Electric current": 3,
    "Temperature": 4,
    "Amount of substance": 5,
    "Luminous intensity": 6,
    "Angle": 7,
    "Count": 8,
    "Information": 9,
    "Symbol count": 10,
    "Frequency": 11,
    "Angular rate": 12,
    "Symbol rate": 13,
    "Electric charge": 14,
    "Force": 15,
    "Energy and work": 16,
    "Power": 17,
    "Pressure": 18,
    "Electrical resistance": 19,
    "Capacitance": 20,
    "Inductance": 21,
    "Magnetic flux": 22,
    "Magnetic flux density": 23,
    "Electric potential": 24,
}

def dimension_signature(dimension: dict[str, Any]) -> tuple[tuple[str, int], ...]:
    return tuple(sorted((key, int(power)) for key, power in dimension.items() if int(power) != 0))

def format_dimension_latex(dimension: dict[str, Any]) -> str:
    terms = []
    for key, power in sorted(dimension.items()):
        if int(power) == 0:
            continue
        symbol = _DIMENSION_LATEX.get(key, rf"\mathrm{{{key}}}")
        terms.append(symbol if int(power) == 1 else rf"{symbol}^{{{int(power)}}}")
    return r"\(1\)" if not terms else r"\(" + r"\,".join(terms) + r"\)"

def generate_units_reference(units: dict[str, Any]) -> str:
    """Generate a compact, dimension-grouped unit reference from UETD's index."""
    dimensions = units.get("base_dimensions", [])
    lines = [
        "# UEF-PL unit reference",
        "",
        "This page is generated from [`UETD/registry/units/registry.json`](../../UETD/registry/units/registry.json). "
        "The registry is authoritative; edit it and regenerate this page rather than editing the tables.",
        "",
        "UEF-PL treats a unit as a compile-time dimension plus an exact scale and, for affine units, an offset. "
        "All dimensions below are independent dimensions in the UEF-PL type system.",
        "",
        "## Dimension symbols",
        "",
        "| Registry dimension | Symbol |",
        "|---|---|",
    ]
    for name in dimensions:
        symbol = _DIMENSION_LATEX.get(name, rf"\mathrm{{{name}}}")
        lines.append(f"| `{name}` | \\({symbol}\\) |")
    lines.extend(
        [
            "",
            "## Registered units",
            "",
            "The scale column gives the exact mapping from a value in that unit to its SI or canonical value. "
            "For affine units it shows the full transformation. Compound units use the registered symbols and "
            "combine their dimensions and scales; their dimensions need not be added to this table as new units.",
            "",
        ]
    )
    grouped: dict[tuple[tuple[str, int], ...], list[tuple[str, dict[str, Any]]]] = {}
    for symbol, unit in units.get("units", {}).items():
        grouped.setdefault(dimension_signature(unit.get("dimension", {})), []).append((symbol, unit))
    for signature, entries in sorted(
        grouped.items(),
        key=lambda item: (_DIMENSION_ORDER.get(_DIMENSION_LABELS.get(item[0], ""), 100), _DIMENSION_LABELS.get(item[0], "Derived dimension"), item[0]),
    ):
        dimension_name = _DIMENSION_LABELS.get(signature, "Derived dimension")
        dimension = entries[0][1].get("dimension", {})
        lines.extend([f"### {dimension_name} — {format_dimension_latex(dimension)}", "", "| Unit | Scale to SI or canonical value |", "|---|---|"])
        for symbol, unit in sorted(entries):
            lines.append(f"| `{symbol}` | {format_unit_scale_latex(unit)} |")
        lines.append("")
    lines.extend(
        [
            "## Expressions and conversion",
            "",
            "```uetd",
            "signal distance : f32<m> = 120 mm;",
            "signal speed : f32<m/s> = distance / 2 s;",
            "signal angle : f32<rad> = convert_unit<rad>(90 deg);",
            "```",
            "",
            "Multiplication adds dimension exponents and multiplies scales; division subtracts exponents and divides scales. "
            "`^` is used for integer unit exponents, as in `m/s^2`; numeric powers use `**`. "
            "Implicit conversion is not allowed. Use `convert_unit<TARGET_UNIT>(value)` when units are compatible. "
            "Absolute temperature values such as `degC` cannot be multiplied or added to another absolute value; "
            "temperature differences use `delta_degC`.",
            "",
            "The catalogue currently registers a finite set of named units. It does not imply that every physically "
            "dimension has a named unit; derived compound dimensions are formed in source from the registered base units.",
            "",
        ]
    )
    return "\n".join(lines)

def generate_catalogue_reference(module_index: dict[str, Any], types: list[dict[str, Any]], functions: list[dict[str, Any]]) -> str:
    """Generate the machine-derived inventories, apart from the human guide."""
    tick = chr(96)
    modules = module_index.get("modules", {})
    lines = [
        "# UEF-PL generated catalogue reference",
        "",
        "This reference is generated from UETD's module, type, and UMATH registries and public headers. "
        "The source JSON records and C headers are authoritative; edit those sources and regenerate this page.",
        "",
        "A registered name is discoverable, but generation also requires a reviewed authoring contract, "
        "an implemented UETD route, and a supported target. See the [language handbook](../guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md), "
        "[technical language guide](../guides/UEF_PROJECT_LANGUAGE_GUIDE.md), and [registry/discovery guide](../guides/UEF_PL_REGISTRIES.md).",
        "",
        "The [unit reference](UEF_PL_UNITS_REFERENCE.md) is generated separately and groups exact scales by dimension.",
        "",
        "## Registered module names",
        "",
        "This list is for discovery. A descriptor does not by itself mean its module is ready for project generation.",
        "",
        "| Source name | UETD module ID | Authoring status | Generation status | Manifest |",
        "|---|---|---|---|---|",
    ]
    authored_modules: list[tuple[str, dict[str, Any], str, dict[str, Any]]] = []
    for module_id, reference in sorted(modules.items(), key=lambda item: (item[1].get("DSL_descriptor", ""), item[0])):
        manifest_rel = reference.get("manifest", "")
        manifest_path = UETD / manifest_rel if manifest_rel else None
        manifest = load_json(manifest_path) if manifest_path and manifest_path.exists() else {}
        authoring = manifest.get("authoring")
        authoring = authoring if isinstance(authoring, dict) else {}
        generation = authoring.get("generation") or {}
        lines.append(
            f"| {tick}{escape_cell(str(reference.get('DSL_descriptor', '')))}{tick} | "
            f"{tick}{module_id}{tick} | {authoring.get('status', 'no authoring contract')} | "
            f"{generation.get('status', 'contract not yet authored')} | "
            f"[{manifest_rel}](../../UETD/{manifest_rel}) |"
        )
        if authoring:
            authored_modules.append((module_id, reference, manifest_rel, authoring))
    lines.extend(
        [
            "",
            f"## Module authoring contracts ({len(authored_modules)})",
            "",
            "Detailed settings, ports, operations, constraints, and generation routes are shown only when the linked UETD manifest has an `authoring` contract. The complete JSON is included for review.",
            "",
        ]
    )
    for module_id, reference, manifest_rel, authoring in authored_modules:
        descriptor = reference.get("DSL_descriptor", "")
        generation = authoring.get("generation") or {}
        summary = (
            f"{descriptor} — {module_id} · {authoring.get('kind', 'unspecified')} · "
            f"authoring {authoring.get('status', 'unspecified')} · "
            f"generation {generation.get('status', 'unspecified')}"
        )
        lines.extend(
            [
                "<details>",
                f"<summary>{escape_cell(summary)}</summary>",
                "",
                "```json",
                json.dumps(authoring, ensure_ascii=False, indent=2, sort_keys=True),
                "```",
                "",
                f"Source: [{manifest_rel}](../../UETD/{manifest_rel}).",
                "",
                "</details>",
                "",
            ]
        )
    lines.extend(
        [
            "## Public UETD C types and enums",
            "",
            f"Currently indexed declarations: {len(types)} ({sum(item.get('source_status') == 'public_header' for item in types)} public-header, {sum(item.get('source_status') == 'future_header' for item in types)} future-header). Explicit DSL spellings: {sum(bool(item.get('dsl_name')) for item in types)}; UMATH value candidates: {sum('UMATH value candidate' in str(item.get('language_visibility', '')) for item in types)}.",
            "",
            "These declarations are indexed from UETD public headers. `DSL type` is populated only when UETD explicitly exposes a project-language spelling. Header presence or autocomplete visibility does not grant permission to use a type as project data; its visibility and owning module contract determine that. `future_header` entries are discoverable drafts, not generation-ready API.",
            "",
            "| DSL type | C type | Kind | DSL visibility | Source status | Header |",
            "|---|---|---|---|---|---|",
        ]
    )
    for item in types:
        lines.append(
            f"| {tick}{escape_cell(str(item.get('dsl_name') or '—'))}{tick} | "
            f"{tick}{escape_cell(str(item['c_name']))}{tick} | {escape_cell(str(item['kind']))} | "
            f"{escape_cell(str(item['language_visibility']))} | {escape_cell(str(item['source_status']))} | "
            f"[{item['header']}](../../UETD/{item['header']}) |"
        )
    lines.extend(
        [
            "",
            f"## UMATH function declarations ({len(functions)})",
            "",
            "This index combines public header declarations with UETD-owned operation names, compatibility aliases, and binding status. Same-name functions are typed overloads selected from reviewed logical parameter types. The C symbol is shown below for source traceability. Callable scope is limited to custom algorithms and nonlinear model bodies, and generation requires an implemented function plus a reviewed DSL binding.",
            "",
            "| DSL operation | Compatibility aliases | C symbol | Module | Implementation | DSL binding | Header |",
            "|---|---|---|---|---|---|",
        ]
    )
    for item in functions:
        signature = item.get("dsl_signature") or f"{item['dsl_name']}(signature pending)"
        aliases = ", ".join(f"{tick}{escape_cell(str(alias))}{tick}" for alias in item.get("dsl_aliases", [])) or "—"
        lines.append(
            f"| {tick}{escape_cell(str(signature))}{tick} | {aliases} | {tick}{item['c_name']}{tick} | "
            f"{tick}{item.get('module_id') or 'unmapped'}{tick} | {item['implementation_status']} | "
            f"{item['dsl_binding_status']} | [{item['header']}](../../UETD/{item['header']}) |"
        )
    lines.extend(
        [
            "",
            "## Source catalogues",
            "",
            "- [Module index](../../UETD/registry/modules/registry.json)",
            "- [Type index](../../UETD/registry/types/registry.json)",
            "- [UMATH function index](../../UETD/registry/umath/registry.json)",
            "- [UETD public headers](../../UETD/include/)",
            "",
        ]
    )
    return "\n".join(lines)

def main() -> None:
    api = load_json(UETD / "api.json")
    module_index = load_json(UETD / api.get("module_registry", "registry/modules/registry.json"))
    unit_catalogue = load_json(UETD / api.get("unit_registry", "registry/units/registry.json"))
    headers = sorted(INCLUDE.rglob("*.h"))
    old_type_path = REGISTRY / "types" / "registry.json"
    old_type_entries: dict[tuple[str, str], dict[str, Any]] = {}
    if old_type_path.exists():
        old_type_catalogue = load_json(old_type_path)
        old_type_entries = {
            (entry.get("c_name"), entry.get("header")): entry
            for entry in old_type_catalogue.get("types", [])
            if isinstance(entry.get("c_name"), str) and isinstance(entry.get("header"), str)
        }
    types = collect_types(headers, old_type_entries)
    old_umath_path = REGISTRY / "umath" / "registry.json"
    old_entries = {}
    if old_umath_path.exists():
        old_catalogue = load_json(old_umath_path)
        old_entries = {(entry.get("c_name"), entry.get("header")): entry for entry in old_catalogue.get("functions", [])}
    functions = collect_functions(sorted((INCLUDE / "umath").rglob("*.h")), module_index, api, old_entries, types)
    type_catalogue = {
        "schema_version": "1.0",
        "source_of_truth": "UETD public C headers under include",
        "generated_by": "tools/generate_project_language_catalogues.py",
        "types": types,
    }
    umath_catalogue = {
        "schema_version": "1.0",
        "source_of_truth": "C signatures and family prefixes from public UETD UMATH headers/types; compatibility aliases and binding contracts authored in this UETD registry",
        "generated_by": "tools/generate_project_language_catalogues.py",
        "callable_contexts": ["custom_algorithm", "model_equation"],
        "source_call_policy": "Users call registry-owned operation names or compatibility aliases; overloads are selected by typed binding contracts. c_name is hidden and selected by the backend route.",
        "binding_contract_required": [
            "DSL-visible function name and signature",
            "typed arguments, modes, and fixed buffer shapes",
            "logical result type/shape/unit; C output pointer lowering",
            "fixed buffer shapes and dimension bounds",
            "aliasing and in-place behavior",
            "numeric and unit effects",
            "failure/status behavior and propagation",
            "target/runtime/numeric availability",
            "bounded work and scratch requirements",
        ],
        "functions": functions,
    }
    type_path = REGISTRY / "types" / "registry.json"
    umath_path = REGISTRY / "umath" / "registry.json"
    type_path.parent.mkdir(parents=True, exist_ok=True)
    umath_path.parent.mkdir(parents=True, exist_ok=True)
    type_path.write_text(json.dumps(type_catalogue, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    umath_path.write_text(json.dumps(umath_catalogue, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    catalogue_path = ROOT / "docs" / "reference" / "UEF_PROJECT_LANGUAGE_CATALOGUE.md"
    units_path = ROOT / "docs" / "reference" / "UEF_PL_UNITS_REFERENCE.md"
    catalogue_path.parent.mkdir(parents=True, exist_ok=True)
    catalogue_path.write_text(generate_catalogue_reference(module_index, types, functions), encoding="utf-8")
    units_path.write_text(generate_units_reference(unit_catalogue), encoding="utf-8")
    print(f"Types: {len(types)} declarations -> {type_path.relative_to(ROOT)}")
    print(f"UMATH functions: {len(functions)} -> {umath_path.relative_to(ROOT)}")
    print(f"Modules: {len(module_index.get('modules', {}))} -> {catalogue_path.relative_to(ROOT)}")
    print(f"Units: {len(unit_catalogue.get('units', {}))} -> {units_path.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
