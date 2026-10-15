"""Load the packaged .uef syntax registry without executing registry content."""

from __future__ import annotations
import json
import jsonschema
from dataclasses import dataclass
from importlib.resources import files
from typing import Any

class LanguageRegistryError(ValueError):
    """Invalid packaged language syntax or dispatch metadata."""

@dataclass(frozen=True)
class LanguageConstruct:
    """One declared syntax form and its fixed compiler dispatch identifiers."""

    construct_id: str
    category: str
    syntax: str
    parse_rule: str
    lowering_rule: str
    description: str
    contract_source: str | None = None

@dataclass(frozen=True)
class LanguageBuiltin:
    """One DSL-owned value form, separate from the UETD component catalogue."""

    builtin_id: str
    parse_rule: str
    lowering_rule: str
    syntax: str
    description: str
    field_policy: str
    fields: tuple[dict[str, Any], ...]

@dataclass(frozen=True)
class LanguageRegistry:
    """Validated DSL metadata bundled with uef-gen."""

    registry_version: str
    language: dict[str, Any]
    lexical: dict[str, Any]
    grammar: dict[str, Any]
    core_types: dict[str, Any]
    operators: tuple[dict[str, Any], ...]
    constructs: dict[str, LanguageConstruct]
    builtins: dict[str, LanguageBuiltin]
    value_forms: tuple[str, ...]
    forbidden_forms: tuple[str, ...]

def load_language_registry() -> LanguageRegistry:
    """Read and validate uef-gen's syntax registry from package resources.

    The JSON names fixed parser/lowering dispatch IDs. It is data, never Python
    code: implementation maps those IDs to an explicit allowlisted dispatch
    table. UETD-owned targets and algorithm/protocol contracts are loaded by
    the separate project-language registry adapter.
    """
    resource_dir = files("uef_gen.language.registry")
    try:
        document = json.loads(resource_dir.joinpath("registry.json").read_text(encoding="utf-8"))
        schema = json.loads(resource_dir.joinpath("schema.json").read_text(encoding="utf-8"))
        jsonschema.validate(document, schema)
    except (OSError, json.JSONDecodeError, jsonschema.ValidationError) as exc:
        raise LanguageRegistryError(f"Unable to load the uef-gen language registry: {exc}") from exc
    productions = document["grammar"]["productions"]
    if document["grammar"]["entrypoint"] not in productions:
        raise LanguageRegistryError("The language grammar entrypoint is not defined in its productions")
    constructs = {
        construct_id: LanguageConstruct(
            construct_id=construct_id,
            category=item["category"],
            syntax=item["syntax"],
            parse_rule=item["parse_rule"],
            lowering_rule=item["lowering_rule"],
            description=item["description"],
            contract_source=item.get("contract_source"),
        )
        for construct_id, item in document["constructs"].items()
    }
    builtins = {
        builtin_id: LanguageBuiltin(
            builtin_id=builtin_id,
            parse_rule=item["parse_rule"],
            lowering_rule=item["lowering_rule"],
            syntax=item["syntax"],
            description=item["description"],
            field_policy=item["field_policy"],
            fields=tuple(item["fields"]),
        )
        for builtin_id, item in document["builtins"].items()
    }
    registry = LanguageRegistry(
        registry_version=document["registry_version"],
        language=document["language"],
        lexical=document["lexical"],
        grammar=document["grammar"],
        core_types=document["core_types"],
        operators=tuple(document["operators"]),
        constructs=constructs,
        builtins=builtins,
        value_forms=tuple(document["value_forms"]),
        forbidden_forms=tuple(document["forbidden_forms"]),
    )
    from uef_gen.language.registry.dispatch import validate_dispatch_ids
    validate_dispatch_ids(registry)
    return registry

def resolve_construct(registry: LanguageRegistry, construct_id: str) -> LanguageConstruct:
    """Resolve a DSL construct by exact registry key or raise a typed error."""
    try:
        return registry.constructs[construct_id]
    except KeyError as exc:
        raise LanguageRegistryError(f"Unknown .uef language construct: {construct_id}") from exc

def resolve_builtin(registry: LanguageRegistry, builtin_id: str) -> LanguageBuiltin:
    """Resolve a DSL-native expression form without consulting UETD modules."""
    try:
        return registry.builtins[builtin_id]
    except KeyError as exc:
        raise LanguageRegistryError(f"Unknown .uef language builtin: {builtin_id}") from exc
