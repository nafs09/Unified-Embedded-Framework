"""Render NEXUS ControlIR through template contracts owned by the UEF checkout.

This adapter intentionally has no algorithm catalogue, algorithm dataclasses,
or local UCON templates. ControlIR nodes remain input data; UEF's manifest
chooses which UEF templates consume each node and which files they produce.
"""

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterator

from uef_gen.templates.models import GeneratedFile
from uef_gen.templates.uef_library import UefTemplateError, render_uef_template
from uef_gen.registry import UEFRegistryError, load_control_ir_manifest, load_module_manifest
from uef_gen.uef_adapter.api_database import UEFApiDatabase, UEFApiError


_C_IDENTIFIER = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")
_C_KEYWORDS = frozenset(
    "auto break case char const continue default do double else enum extern float for goto if "
    "inline int long register restrict return short signed sizeof static struct switch typedef "
    "union unsigned void volatile while _Alignas _Alignof _Atomic _Bool _Complex _Generic "
    "_Imaginary _Noreturn _Static_assert _Thread_local".split()
)


class UEFTemplateContractError(ValueError):
    """Raised when the located UEF checkout lacks a usable template contract."""

    def __init__(self, message: str, diagnostic_code: str = "uef_template_contract"):
        super().__init__(message)
        self.diagnostic_code = diagnostic_code


@dataclass(frozen=True)
class UEFTemplateRender:
    """Rendered UEF files and implementation actions declared by UEF."""

    files: tuple[GeneratedFile, ...]
    manual_actions: tuple[dict[str, Any], ...]


class UEFTemplateAdapter:
    """Read UEF's manifest/API data and render only paths declared by UEF."""

    def __init__(self, uef_root: Path):
        self.uef_root = uef_root.resolve()
        self.template_root = (self.uef_root / "templates").resolve()
        try:
            manifest = load_control_ir_manifest(self.uef_root)
            api = json.loads((self.uef_root / "uef_api.json").read_text(encoding="utf-8"))
            module_document = load_module_manifest(self.uef_root)
        except (OSError, json.JSONDecodeError, UEFRegistryError) as exc:
            raise UEFTemplateContractError(f"Cannot read UEF template/API registry metadata: {exc}") from exc
        if not isinstance(manifest, dict):
            raise UEFTemplateContractError("UEF ControlIR template manifest must be a JSON object")
        control_nodes = manifest.get("control_nodes", {})
        algorithms = manifest.get("algorithms", {})
        aliases = manifest.get("aliases", {})
        modules = module_document.get("modules") if isinstance(module_document, dict) else None
        shared_templates = manifest.get("shared_templates", [])
        shared_modules = manifest.get("modules", [])
        if manifest.get("schema_version") not in {"1.0", "1.1"}:
            raise UEFTemplateContractError(
                "UEF template manifest schema_version must be '1.0' or '1.1'"
            )
        if not isinstance(control_nodes, dict):
            raise UEFTemplateContractError("UEF manifest 'control_nodes' must be an object")
        if not isinstance(algorithms, dict):
            raise UEFTemplateContractError("UEF manifest 'algorithms' must be an object")
        if not isinstance(aliases, dict):
            raise UEFTemplateContractError("UEF manifest 'aliases' must be an object")
        if not isinstance(modules, dict):
            raise UEFTemplateContractError("UEF module metadata must contain a 'modules' object")
        if not isinstance(shared_templates, list):
            raise UEFTemplateContractError("UEF manifest 'shared_templates' must be an array")
        if not isinstance(shared_modules, list):
            raise UEFTemplateContractError("UEF manifest 'modules' must be an array")
        if not isinstance(manifest.get("manual_actions", []), list):
            raise UEFTemplateContractError("UEF manifest 'manual_actions' must be an array")

        for key, entry in control_nodes.items():
            if not isinstance(key, str) or not key.strip() or not isinstance(entry, dict):
                raise UEFTemplateContractError(
                    "UEF 'control_nodes' entries require non-empty keys and object values"
                )
            if not isinstance(entry.get("outputs"), list) or not entry["outputs"]:
                raise UEFTemplateContractError(
                    f"UEF template entry {key!r} must declare a non-empty outputs array"
                )
            if not isinstance(entry.get("modules", []), list):
                raise UEFTemplateContractError(
                    f"UEF template entry {key!r} 'modules' must be an array"
                )
            if not isinstance(entry.get("manual_actions", []), list):
                raise UEFTemplateContractError(
                    f"UEF template entry {key!r} 'manual_actions' must be an array"
                )
            for action in entry.get("manual_actions", []):
                self._validate_manual_action(action, key)

        for key, entry in algorithms.items():
            if not isinstance(key, str) or not key.strip() or not isinstance(entry, dict):
                raise UEFTemplateContractError(
                    "UEF 'algorithms' entries require non-empty keys and object values"
                )
            registered = entry.get("registered")
            renderable = entry.get("renderable", bool(entry.get("outputs")))
            if not isinstance(registered, bool) or not isinstance(renderable, bool):
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} must declare boolean registered/renderable fields"
                )
            outputs = entry.get("outputs")
            if not isinstance(outputs, list):
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} 'outputs' must be an array"
                )
            if renderable and (not registered or not outputs):
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} is renderable but is not registered with outputs"
                )
            if not registered and outputs:
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} is unregistered and cannot declare selectable outputs"
                )
            if not isinstance(entry.get("modules", []), list):
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} 'modules' must be an array"
                )
            if not isinstance(entry.get("manual_actions", []), list):
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} 'manual_actions' must be an array"
                )
            if registered and not renderable and entry.get("direct_use") is not True:
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} has no outputs and must explicitly declare direct_use"
                )
            for action in entry.get("manual_actions", []):
                self._validate_manual_action(action, key)

        for alias, canonical in aliases.items():
            if not isinstance(alias, str) or not alias.strip():
                raise UEFTemplateContractError("UEF algorithm aliases require non-empty names")
            if isinstance(canonical, str):
                if canonical not in algorithms:
                    raise UEFTemplateContractError(
                        f"UEF alias {alias!r} refers to unknown algorithm {canonical!r}"
                    )
            elif isinstance(canonical, list):
                if not canonical or any(
                    not isinstance(item, str) or item not in algorithms
                    for item in canonical
                ):
                    raise UEFTemplateContractError(
                        f"UEF ambiguous alias {alias!r} must list known algorithm keys"
                    )
            else:
                raise UEFTemplateContractError(
                    f"UEF alias {alias!r} must name one algorithm or an array of candidates"
                )
        if not isinstance(api, dict):
            raise UEFTemplateContractError("UEF API metadata must be a JSON object")
        try:
            UEFApiDatabase(api)
        except UEFApiError as exc:
            raise UEFTemplateContractError(f"UEF API metadata is invalid: {exc}") from exc
        if manifest.get("schema_version") == "1.1":
            ucon_api = api.get("ucon_api", {})
            scaffold_tree = ucon_api.get("scaffold_tree", {}) if isinstance(ucon_api, dict) else {}
            initial_planned = (
                scaffold_tree.get("initial_planned_algorithms", [])
                if isinstance(scaffold_tree, dict) else None
            )
            if not isinstance(initial_planned, list) or any(
                not isinstance(key, str) or not key for key in initial_planned
            ):
                raise UEFTemplateContractError(
                    "UEF API scaffold_tree.initial_planned_algorithms must be an array of algorithm keys"
                )
            self._validate_scaffold_entries(algorithms, modules, set(initial_planned))
        self.manifest = manifest
        self.api_document = api
        self.module_records = modules

    def _validate_scaffold_entries(
        self, algorithms: dict[str, Any], modules: dict[str, Any],
        initial_planned: set[str],
    ) -> None:
        """Keep every unregistered manifest row tied to its own generated file/module."""
        expected_stage = {"FIRST_PASS": "initial", "PLANNED": "future", "DEFERRED": "future"}
        seen_headers: set[str] = set()
        seen_sources: set[str] = set()
        common_header = self.uef_root / "include" / "uef" / "ucon" / "detail" / "algorithm_call.h"

        for key, entry in algorithms.items():
            if entry.get("registered") is True:
                continue
            priority = entry.get("priority")
            stage = expected_stage.get(priority)
            if priority == "PLANNED" and key in initial_planned:
                stage = "initial"
            if stage is None or entry.get("scaffold_stage") != stage:
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} has inconsistent scaffold stage for priority {priority!r}"
                )
            header = entry.get("scaffold_header")
            source = entry.get("scaffold_source")
            module_name = entry.get("scaffold_module")
            if not all(isinstance(value, str) and value for value in (header, source, module_name)):
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} must declare its individual scaffold header, source, and module"
                )

            normalized_paths: list[str] = []
            for label, relative in (("header", header), ("source", source)):
                normalized = Path(relative.replace("\\", "/"))
                if normalized.is_absolute() or ".." in normalized.parts:
                    raise UEFTemplateContractError(
                        f"UEF algorithm {key!r} has an unsafe scaffold {label} path {relative!r}"
                    )
                absolute = (self.uef_root / normalized).resolve()
                if not absolute.is_relative_to(self.uef_root):
                    raise UEFTemplateContractError(
                        f"UEF algorithm {key!r} scaffold {label} path escapes the UEF checkout"
                    )
                if not absolute.is_file():
                    raise UEFTemplateContractError(
                        f"UEF algorithm {key!r} scaffold {label} is missing: {relative}"
                    )
                normalized_paths.append(normalized.as_posix())
            header_key, source_key = normalized_paths
            if stage == "future":
                paths_match_stage = (
                    header_key.startswith("include/uef/ucon/future/")
                    and source_key.startswith("src/ucon/future/")
                )
            else:
                paths_match_stage = (
                    header_key.startswith("include/uef/ucon/")
                    and source_key.startswith("src/ucon/")
                    and "/future/" not in header_key
                    and "/future/" not in source_key
                    and "/scaffold/" not in header_key
                    and "/scaffold/" not in source_key
                )
            if not paths_match_stage:
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} scaffold files do not match its {stage} placement"
                )
            expected_module = "ucon/" + header_key.removeprefix("include/uef/ucon/").removesuffix(".h")
            expected_source = "src/ucon/" + header_key.removeprefix("include/uef/ucon/").removesuffix(".h") + ".c"
            if module_name != expected_module or source_key != expected_source:
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} scaffold module/path registration is inconsistent"
                )
            if header_key in seen_headers or source_key in seen_sources:
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} shares a scaffold file with another catalog entry"
                )
            seen_headers.add(header_key)
            seen_sources.add(source_key)

            record = modules.get(module_name)
            if not isinstance(record, dict):
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} scaffold module {module_name!r} is not registered"
                )
            if (
                record.get("algorithm") != key
                or record.get("scaffold_only") is not True
                or record.get("available_for_generation") is not False
                or record.get("priority") != priority
                or record.get("scaffold_stage") != stage
                or record.get("scaffold_reason") != entry.get("scaffold_reason")
                or record.get("project_relevance") != entry.get("project_relevance")
                or source_key not in record.get("sources", [])
                or header_key not in record.get("headers", [])
            ):
                raise UEFTemplateContractError(
                    f"UEF algorithm {key!r} scaffold module metadata does not match its manifest paths"
                )

        if not common_header.is_file():
            raise UEFTemplateContractError(
                "UEF shared development scaffold contract is missing: "
                "include/uef/ucon/detail/algorithm_call.h"
            )
        unknown = initial_planned - algorithms.keys()
        if unknown:
            raise UEFTemplateContractError(
                "UEF API marks unknown PLANNED algorithms as initial: "
                + ", ".join(sorted(unknown))
            )
        incorrectly_prioritized = sorted(
            key for key in initial_planned
            if algorithms[key].get("priority") != "PLANNED"
        )
        if incorrectly_prioritized:
            raise UEFTemplateContractError(
                "UEF API initial planned list contains non-PLANNED algorithms: "
                + ", ".join(incorrectly_prioritized)
            )

    def required_modules(self, control_ir: dict[str, Any]) -> tuple[str, ...]:
        """Return UEF modules declared by the matching UEF template entries."""
        self._validate_symbol_prefixes(control_ir)
        modules: set[str] = set()
        for module in self.manifest.get("modules", []):
            if not isinstance(module, str) or not module.strip():
                raise UEFTemplateContractError("UEF manifest 'modules' entries must be non-empty strings")
            modules.add(module)
        for _, node in iter_control_nodes(control_ir):
            entry = self._entry_for(node)
            node_modules = entry.get("modules", [])
            if not isinstance(node_modules, list):
                raise UEFTemplateContractError(
                    f"UEF template entry for {control_node_key(node)!r} 'modules' must be an array"
                )
            for module in node_modules:
                if not isinstance(module, str) or not module.strip():
                    raise UEFTemplateContractError(
                        f"UEF template entry for {control_node_key(node)!r} has an invalid module name"
                    )
                modules.add(module)
        return tuple(sorted(modules))

    def render_control_ir(self, context: dict[str, Any]) -> UEFTemplateRender:
        """Render declared templates and collect UEF-declared manual actions."""
        control_ir = context.get("control_ir")
        if not isinstance(control_ir, dict):
            raise UEFTemplateContractError("ControlIR template context must contain an object")
        self._validate_symbol_prefixes(control_ir)

        generated: list[GeneratedFile] = []
        manual_actions: list[dict[str, Any]] = []
        for action in self.manifest.get("manual_actions", []):
            self._validate_manual_action(action, "manifest")
            manual_actions.append(self._manual_action_record(action, None, "", "shared"))
        for entry in self.manifest.get("shared_templates", []):
            generated.append(self._render_file_entry(entry, context, "shared template"))

        for node_path, node in iter_control_nodes(control_ir):
            entry = self._entry_for(node)
            node_context = {
                **context,
                "node": node,
                "control_node": node,
                # `algo` is a convenience alias for existing UEF templates; it
                # refers to the raw ControlIR node, not a generator-owned IR.
                "algo": node,
                "template_entry": entry,
                "algorithm_entry": entry if entry.get("domain") == "ucon" else None,
                "algorithm_key": control_node_key(node),
                "canonical_algorithm_key": (
                    self._canonical_algorithm_key(control_node_key(node))
                    if control_node_key(node) in self.manifest.get("aliases", {})
                    or control_node_key(node) in self.manifest.get("algorithms", {})
                    else control_node_key(node)
                ),
                "node_path": node_path,
            }
            domain = entry.get("domain", "unspecified")
            for action in entry.get("manual_actions", []):
                self._validate_manual_action(action, control_node_key(node))
                manual_actions.append(
                    self._manual_action_record(action, node, node_path, str(domain))
                )
            outputs = entry.get("outputs")
            if not isinstance(outputs, list) or not outputs:
                raise UEFTemplateContractError(
                    f"UEF template entry for {control_node_key(node)!r} must declare a non-empty outputs array"
                )
            for output_index, output_entry in enumerate(outputs):
                generated.append(self._render_file_entry(
                    output_entry, node_context,
                    f"{control_node_key(node)!r} output {output_index}",
                ))
        return UEFTemplateRender(tuple(generated), tuple(manual_actions))

    def _validate_symbol_prefixes(self, control_ir: dict[str, Any]) -> None:
        """Require stable, unique C identifiers for nodes routed to UEF templates."""
        used: dict[str, str] = {}
        for node_path, node in iter_control_nodes(control_ir):
            key = control_node_key(node)
            self._entry_for(node)
            model = node.get("model")
            prefix = model.get("symbol_prefix") if isinstance(model, dict) else None
            if (
                not isinstance(prefix, str)
                or not _C_IDENTIFIER.fullmatch(prefix)
                or prefix in _C_KEYWORDS
            ):
                raise UEFTemplateContractError(
                    f"ControlIR node {key!r} at {node_path} requires a C-safe "
                    "model.symbol_prefix supplied by the ControlIR producer"
                )
            previous = used.get(prefix)
            if previous is not None:
                raise UEFTemplateContractError(
                    f"ControlIR model.symbol_prefix {prefix!r} is duplicated at "
                    f"{previous} and {node_path}"
                )
            used[prefix] = node_path

    def _canonical_algorithm_key(self, key: str) -> str:
        alias = self.manifest.get("aliases", {}).get(key, key)
        if isinstance(alias, list):
            raise UEFTemplateContractError(
                f"ControlIR algorithm key {key!r} is an ambiguous compatibility alias; "
                f"select one of: {', '.join(alias)}",
                diagnostic_code="algorithm_alias_ambiguous",
            )
        return str(alias)

    def _entry_for(self, node: dict[str, Any]) -> dict[str, Any]:
        key = control_node_key(node)
        protocol_entry = self.manifest["control_nodes"].get(key)
        if isinstance(protocol_entry, dict):
            return protocol_entry

        canonical = self._canonical_algorithm_key(key)
        entry = self.manifest.get("algorithms", {}).get(canonical)
        if not isinstance(entry, dict):
            raise UEFTemplateContractError(
                f"UEF has no catalogue entry for ControlIR node type/subtype {key!r}",
                diagnostic_code="algorithm_unknown",
            )
        if entry.get("registered") is not True:
            family = entry.get("family", "unassigned UCON family")
            scaffold = entry.get("scaffold_header", "no individual scaffold header registered")
            priority = entry.get("priority", "unclassified")
            raise UEFTemplateContractError(
                f"UEF algorithm {canonical!r} is not registered for use yet "
                f"(family: {family}, roadmap priority: {priority}). Its development scaffold "
                f"is at {scaffold} and returns UCON_NOT_IMPLEMENTED.",
                diagnostic_code="algorithm_unregistered",
            )
        if entry.get("renderable") is not True or not entry.get("outputs"):
            raise UEFTemplateContractError(
                f"UEF algorithm {canonical!r} is registered for direct C use but has no "
                "ControlIR output template.",
                diagnostic_code="algorithm_direct_only",
            )
        return entry

    @staticmethod
    def _validate_manual_action(action: Any, entry_key: str) -> None:
        if not isinstance(action, dict):
            raise UEFTemplateContractError(
                f"UEF manual action for {entry_key!r} must be an object"
            )
        if not isinstance(action.get("code"), str) or not action["code"].strip():
            raise UEFTemplateContractError(
                f"UEF manual action for {entry_key!r} requires a non-empty code"
            )
        if not isinstance(action.get("message"), str) or not action["message"].strip():
            raise UEFTemplateContractError(
                f"UEF manual action {action.get('code')!r} requires a non-empty message"
            )
        if "required_before_hardware_use" in action and not isinstance(
            action["required_before_hardware_use"], bool
        ):
            raise UEFTemplateContractError(
                f"UEF manual action {action['code']!r} required_before_hardware_use must be boolean"
            )

    @staticmethod
    def _manual_action_record(
        action: dict[str, Any], node: dict[str, Any] | None,
        node_path: str, domain: str,
    ) -> dict[str, Any]:
        return {
            "code": action["code"],
            "message": action["message"],
            "required_before_hardware_use": action.get(
                "required_before_hardware_use", True
            ),
            "domain": domain,
            "node_id": node.get("id") if node is not None else None,
            "node_path": node_path,
        }

    def _render_file_entry(
        self, entry: Any, context: dict[str, Any], label: str
    ) -> GeneratedFile:
        if not isinstance(entry, dict):
            raise UEFTemplateContractError(f"UEF {label} entry must be an object")
        return self._render_declared_file(
            entry.get("template"), entry.get("output"), context, label
        )

    def _render_declared_file(
        self, template: Any, output_template: Any,
        context: dict[str, Any], label: str,
    ) -> GeneratedFile:
        if not isinstance(template, str) or not template:
            raise UEFTemplateContractError(f"UEF {label} must name a template path")
        if not isinstance(output_template, str) or not output_template:
            raise UEFTemplateContractError(f"UEF {label} must declare an output path")
        try:
            content = render_uef_template(self.template_root, template, context)
            output = render_output_path(output_template, context)
        except UefTemplateError as exc:
            raise UEFTemplateContractError(str(exc)) from exc
        normalized = Path(output.replace("\\", "/"))
        if not output or normalized.is_absolute() or ".." in normalized.parts:
            raise UEFTemplateContractError(f"UEF {label} has an unsafe output path {output!r}")
        return GeneratedFile(normalized.as_posix(), content)


def control_node_key(node: dict[str, Any]) -> str:
    """Use the ControlIR subtype when present, otherwise its node type."""
    subtype = node.get("subtype")
    node_type = node.get("type")
    selected = subtype if isinstance(subtype, str) and subtype not in {"", "*"} else node_type
    if not isinstance(selected, str) or not selected.strip():
        raise UEFTemplateContractError("ControlIR node must contain a type or subtype")
    return selected.strip().upper()


def iter_control_nodes(control_ir: dict[str, Any]) -> Iterator[tuple[str, dict[str, Any]]]:
    """Walk top-level nodes and nested composite graphs in stable input order."""
    def walk(graph: dict[str, Any], prefix: str) -> Iterator[tuple[str, dict[str, Any]]]:
        nodes = graph.get("nodes", [])
        if not isinstance(nodes, list):
            raise UEFTemplateContractError(f"ControlIR {prefix or 'root'} nodes must be an array")
        for index, node in enumerate(nodes):
            path = f"{prefix}.nodes[{index}]" if prefix else f"nodes[{index}]"
            if not isinstance(node, dict):
                raise UEFTemplateContractError(f"ControlIR {path} must be an object")
            yield path, node
        composites = graph.get("composite_nodes", [])
        if not isinstance(composites, list):
            raise UEFTemplateContractError(f"ControlIR {prefix or 'root'} composite_nodes must be an array")
        for index, composite in enumerate(composites):
            path = f"{prefix}.composite_nodes[{index}]" if prefix else f"composite_nodes[{index}]"
            if not isinstance(composite, dict):
                raise UEFTemplateContractError(f"ControlIR {path} must be an object")
            internal = composite.get("internal_graph")
            if not isinstance(internal, dict):
                raise UEFTemplateContractError(f"ControlIR {path}.internal_graph must be an object")
            yield from walk(internal, f"{path}.internal_graph")

    yield from walk(control_ir, "")


def render_output_path(template: str, context: dict[str, Any]) -> str:
    """Render an output path pattern with the same strict variable policy."""
    try:
        from jinja2 import Environment, StrictUndefined
    except ImportError as exc:
        raise UefTemplateError(
            "Jinja2 is required to render UEF template paths; install uef-gen[templates]"
        ) from exc
    try:
        environment = Environment(undefined=StrictUndefined, autoescape=False)
        return environment.from_string(template).render(context)
    except Exception as exc:
        raise UefTemplateError(f"could not render UEF output path {template!r}: {exc}") from exc
