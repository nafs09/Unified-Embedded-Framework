"""Safe loading of the one explicit `.uefs` contract import in a `.uef` file.

UETD targets, modules, types, units, and UMATH functions are loaded from the
live API-linked UETD registry; they are not source imports. This module handles
only `contract import "relative/path.uefs";` and must never compile or execute
the imported system's sibling projects.
"""

from __future__ import annotations

import hashlib
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from uef_gen.language.frontends.project.ast import ContractImport, EnumDefinition, FlagsDefinition, FixedPointDefinition, SourceSpan, VariantDefinition
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic
from uef_gen.language.frontends.project.lexer import Lexer
from uef_gen.language.frontends.system.parser import SystemParser
from uef_gen.language.frontends.system.ast import SystemDocument, SystemLinkSpec, SystemProtocolSpec, SystemRecordSpec
from uef_gen.language.registry.loader import load_language_registry


@dataclass(frozen=True)
class ImportedSystemContract:
    """Only the shared declarations and this node's endpoint facts from a system."""

    system_name: str
    source_path: Path
    content_digest: str
    local_node_name: str
    records: tuple[SystemRecordSpec, ...]
    enums: tuple[EnumDefinition, ...]
    protocols: tuple[SystemProtocolSpec, ...]
    local_endpoint_facts: dict[str, Any]
    local_links: tuple[SystemLinkSpec, ...]
    variants: tuple[VariantDefinition, ...] = ()
    flags_types: tuple[FlagsDefinition, ...] = ()
    fixed_point_types: tuple[FixedPointDefinition, ...] = ()
    source_span: SourceSpan | None = None


def resolve_contract_import_path(import_decl: ContractImport, source_path: Path) -> tuple[Path | None, list[LanguageDiagnostic]]:
    """Resolve an import under the directory containing its importing file."""
    diagnostics: list[LanguageDiagnostic] = []
    if source_path is None:
        return None, [LanguageDiagnostic("error", "contract_import_requires_file", "A contract import requires a saved .uef file so its relative path has an unambiguous base.", import_decl.span)]
    source = source_path.expanduser().resolve(strict=False)
    relative = import_decl.path
    candidate_relative = Path(relative)
    if (
        not relative
        or candidate_relative.is_absolute()
        or candidate_relative.drive
        or ":" in relative
        or "\\" in relative
        or any(part in ("", ".", "..") for part in relative.split("/"))
        or not relative.endswith(".uefs")
    ):
        diagnostics.append(LanguageDiagnostic("error", "invalid_contract_import_path", "Contract imports must be normalized relative forward-slash paths ending in .uefs.", import_decl.span))
        return None, diagnostics
    root = source.parent.resolve(strict=False)
    candidate = (root / Path(*relative.split("/"))).resolve(strict=False)
    if not candidate.is_relative_to(root):
        diagnostics.append(LanguageDiagnostic("error", "contract_import_path_escape", "The resolved contract import must remain beneath the importing project's directory.", import_decl.span))
        return None, diagnostics
    if candidate.suffix != ".uefs":
        diagnostics.append(LanguageDiagnostic("error", "invalid_contract_import_suffix", "Contract import paths must end in .uefs.", import_decl.span))
        return None, diagnostics
    if not candidate.is_file():
        diagnostics.append(LanguageDiagnostic("error", "contract_import_not_found", f"Contract file does not exist: {relative}", import_decl.span))
        return None, diagnostics
    return candidate, diagnostics


def load_contract_imports(
    imports: tuple[ContractImport, ...],
    *,
    source_path: Path,
) -> tuple[tuple[ImportedSystemContract, ...], list[LanguageDiagnostic]]:
    """Parse a system file and return only the importing node's contract view."""
    diagnostics: list[LanguageDiagnostic] = []
    if len(imports) > 1:
        for extra in imports[1:]:
            diagnostics.append(LanguageDiagnostic("error", "too_many_contract_imports", "UEF-PL currently permits at most one contract import per project.", extra.span))
        return (), diagnostics
    if not imports:
        return (), diagnostics
    path, path_diagnostics = resolve_contract_import_path(imports[0], source_path)
    diagnostics.extend(path_diagnostics)
    if path is None:
        return (), diagnostics
    try:
        text = path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError) as exc:
        diagnostics.append(LanguageDiagnostic("error", "contract_import_read_error", f"Cannot read imported .uefs file: {exc}", imports[0].span))
        return (), diagnostics
    tokens, lexer_diagnostics = Lexer(text, path).tokenize()
    parser = SystemParser(tokens, path)
    document, parser_diagnostics = parser.parse()
    diagnostics.extend(lexer_diagnostics)
    diagnostics.extend(parser_diagnostics)
    if document is None or any(item.severity == "error" for item in diagnostics):
        return (), diagnostics
    importer = source_path.expanduser().resolve(strict=False)
    system_root = path.parent.resolve(strict=False)
    matches: list[str] = []
    for node in document.nodes:
        node_path = node.project_path
        if node_path.is_absolute() or ".." in node_path.parts:
            continue
        resolved_node = (system_root / node_path).resolve(strict=False)
        if resolved_node == importer:
            matches.append(node.name)
    if len(matches) != 1:
        diagnostics.append(LanguageDiagnostic("error", "contract_import_node_match", "The imported .uefs file must contain exactly one node whose project path names this .uef file.", imports[0].span))
        return (), diagnostics
    node_name = matches[0]
    local_endpoint_facts: dict[str, Any] = {}
    for endpoint in document.external_endpoints:
        if endpoint.attached_to == node_name:
            local_endpoint_facts[endpoint.name] = {
                "implementation": endpoint.implementation,
                "role": endpoint.role,
                "properties": dict(endpoint.properties),
                "source_span": endpoint.source_span,
            }
    local_links = tuple(
        link for link in document.links
        if _endpoint_owner(link.source_endpoint) == node_name or _endpoint_owner(link.destination_endpoint) == node_name
        or link.source_endpoint in local_endpoint_facts or link.destination_endpoint in local_endpoint_facts
    )
    imported = ImportedSystemContract(
        system_name=document.name,
        source_path=path,
        content_digest=system_document_digest(document, text),
        local_node_name=node_name,
        records=document.records,
        enums=document.enums,
        protocols=document.protocols,
        local_endpoint_facts=local_endpoint_facts,
        local_links=local_links,
        variants=document.variants,
        flags_types=document.flags_types,
        fixed_point_types=document.fixed_point_types,
        source_span=imports[0].span,
    )
    _, scope_diagnostics = build_contract_type_scope((imported,))
    diagnostics.extend(scope_diagnostics)
    return (imported,), diagnostics


def build_contract_type_scope(
    contracts: tuple[ImportedSystemContract, ...],
) -> tuple[dict[str, Any], list[LanguageDiagnostic]]:
    """Index shared nominal types and diagnose collisions across imports."""
    scope: dict[str, Any] = {}
    diagnostics: list[LanguageDiagnostic] = []
    for contract in contracts:
        items = [*contract.records, *contract.enums, *contract.variants, *contract.flags_types, *contract.fixed_point_types]
        for item in items:
            name = item.name
            if name in scope:
                prior = scope[name]
                related = ((f"First declaration of {name!r} is in {prior[0].source_path}.", prior[1]),) if prior[1] is not None else ()
                diagnostics.append(LanguageDiagnostic("error", "duplicate_imported_type", f"Imported type name {name!r} is declared more than once.", getattr(item, "span", None), related))
                continue
            scope[name] = (contract, getattr(item, "span", None), item)
        for protocol in contract.protocols:
            if protocol.name in scope:
                diagnostics.append(LanguageDiagnostic("error", "duplicate_imported_type", f"Imported protocol type name {protocol.name!r} collides with another imported type.", protocol.source_span))
            else:
                scope[protocol.name] = (contract, protocol.source_span, protocol)
    return scope, diagnostics


def system_document_digest(document: SystemDocument, source_text: str) -> str:
    """Hash canonical source text and the packaged `.uefs` language version."""
    del document
    version = load_language_registry().registry_version
    normalized = source_text.replace("\r\n", "\n").replace("\r", "\n")
    payload = f"uefs-registry:{version}\n{normalized}".encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def _endpoint_owner(endpoint: str) -> str | None:
    """Extract a qualified node owner from a dotted endpoint reference."""
    return endpoint.split(".", 1)[0] if "." in endpoint else None
