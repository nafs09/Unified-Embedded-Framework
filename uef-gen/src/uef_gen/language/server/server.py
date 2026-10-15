"""Shared LSP endpoint for VS Code and Visual Studio.

The language server delegates parsing and diagnostics to the public project
and system compiler APIs. Internal source spans use Python code-point columns;
the LSP adapter converts those columns to the UTF-16 positions expected by
editors. Completion and navigation use the same AST and live UETD registry.
"""

from __future__ import annotations
import argparse
import asyncio
import json
import logging
import os
import re
import sys
from dataclasses import fields, is_dataclass
from functools import lru_cache
from logging.handlers import RotatingFileHandler
from lsprotocol import types
from pathlib import Path
from pygls.protocol import _dict_to_object, default_converter
from pygls.protocol.json_rpc import JsonRPCNotification, JsonRPCRequestMessage
from pygls.lsp.server import LanguageServer
from uef_gen.uetd_adapter.locator import UETDLocator
from uef_gen.language.frontends.project import compile_project_text
from uef_gen.language.frontends.project.ast import MathCallExpression
from uef_gen.language.frontends.project.hints import HintContext, complete as complete_language, complete_contract_import_paths, hover as hover_language
from uef_gen.language.frontends.project.lexer import Lexer
from uef_gen.language.frontends.project.parser import Parser
from uef_gen.language.frontends.project.validation import build_project_symbol_table
from uef_gen.language.registry.loader import load_language_registry
from uef_gen.language.frontends.project.registry_contracts import AuthoringCatalogue, load_authoring_catalogue
from uef_gen.language.frontends.system.compiler import compile_system_text
from uef_gen.language.frontends.system.parser import SystemParser
from urllib.parse import unquote, urlparse

_LOGGER = logging.getLogger("uef_gen.language.server")

def _default_log_path() -> Path:
    """Return a stable per-user log path without writing into the project."""
    if os.name == "nt":
        base = Path(os.environ.get("LOCALAPPDATA") or (Path.home() / "AppData" / "Local"))
    else:
        base = Path(os.environ.get("XDG_STATE_HOME") or (Path.home() / ".local" / "state")) / "uef-gen"
    return base / "UEF-PL-LanguageServer.log"


def _configure_logging(log_file: Path | None, log_level: str | None) -> Path | None:
    """Log LSP startup, request failures, and compiler diagnostics to stderr and a file.

    LSP stdout is reserved for JSON-RPC framing; never attach a logging handler
    to it. ``UEF_PL_LOG_FILE`` and ``UEF_PL_LOG_LEVEL`` let either editor enable
    the same server-side diagnostics without editor-specific configuration.
    """
    selected_path = log_file or (Path(os.environ["UEF_PL_LOG_FILE"]) if os.environ.get("UEF_PL_LOG_FILE") else _default_log_path())
    selected_level = (log_level or os.environ.get("UEF_PL_LOG_LEVEL") or "INFO").upper()
    numeric_level = getattr(logging, selected_level, None)
    if not isinstance(numeric_level, int):
        numeric_level = logging.INFO
        selected_level = "INFO"

    formatter = logging.Formatter("%(asctime)s %(levelname)s %(name)s %(message)s", "%Y-%m-%d %H:%M:%S")
    handlers: list[logging.Handler] = []
    stderr_handler = logging.StreamHandler(sys.stderr)
    stderr_handler.setFormatter(formatter)
    handlers.append(stderr_handler)
    active_path: Path | None = None
    try:
        selected_path = selected_path.expanduser().resolve()
        selected_path.parent.mkdir(parents=True, exist_ok=True)
        file_handler = RotatingFileHandler(
            selected_path, maxBytes=4 * 1024 * 1024, backupCount=3, encoding="utf-8"
        )
        file_handler.setFormatter(formatter)
        handlers.append(file_handler)
        active_path = selected_path
    except OSError as exc:
        stderr_handler.emit(logging.LogRecord(
            name=__name__, level=logging.WARNING, pathname=__file__, lineno=0,
            msg="Could not open UEF-PL log file %s: %s", args=(selected_path, exc),
            exc_info=None, func=None, sinfo=None,
        ))

    logging.basicConfig(level=numeric_level, handlers=handlers, force=True)
    # pygls logs full JSON-RPC bodies at INFO, including very large semantic
    # token arrays. The editor already captures stderr, so those routine
    # protocol dumps can grow its user log without helping diagnose failures.
    logging.getLogger("pygls").setLevel(logging.WARNING)
    logging.captureWarnings(True)
    _LOGGER.info("UEF-PL language server starting; log_level=%s log_file=%s python=%s", selected_level,
                 active_path or "stderr only", sys.version.split()[0])
    return active_path


def _structure_diagnostic_tag(value: object, _target_type: type[types.DiagnosticTag]) -> types.DiagnosticTag | int:
    """Accept future/vendor DiagnosticTag values in client capabilities.

    Visual Studio advertises two additional integer tags in
    ``publishDiagnostics.tagSupport.valueSet``. They are valid capability
    values for negotiation, but the pinned ``lsprotocol`` enum only knows the
    two standard LSP tags. Preserve unknown integers so cattrs can finish
    decoding ``initialize``; the server does not interpret client tag values.
    """
    try:
        return types.DiagnosticTag(value)
    except (TypeError, ValueError):
        if isinstance(value, int) and not isinstance(value, bool):
            return value
        raise


def _language_server_converter():
    """Build pygls' converter with forward-compatible client-message parsing.

    Visual Studio adds W3C tracing metadata (currently ``traceparent``) to a
    few JSON-RPC envelopes. It is transport metadata, not an LSP parameter;
    drop that one known extension field before pygls structures generic custom
    requests/notifications. Standard LSP payloads and all other fields remain
    subject to pygls' normal validation.
    """
    converter = default_converter()
    converter.register_structure_hook(types.DiagnosticTag, _structure_diagnostic_tag)

    def structure_rpc_message(value: object, message_type: type):
        if not isinstance(value, dict):
            return message_type(**value)  # Preserve pygls' normal failure for malformed envelopes.
        envelope = dict(value)
        envelope.pop("traceparent", None)
        if "params" in envelope:
            envelope["params"] = _dict_to_object(envelope["params"])
        return message_type(**envelope)

    converter.register_structure_hook(JsonRPCNotification, structure_rpc_message)
    converter.register_structure_hook(JsonRPCRequestMessage, structure_rpc_message)
    return converter


server = LanguageServer("uef-gen-language-server", "0.1.0", converter_factory=_language_server_converter)
_explicit_uetd_root: Path | None = None
_WORD = re.compile(r"[A-Za-z_][A-Za-z_0-9]*")
_IDENTIFIER_TRIGGERS = list("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") + ["."]
_SEMANTIC_TOKEN_TYPES = [
    "namespace", "type", "class", "enumMember", "function", "method", "parameter",
    "variable", "property", "keyword", "number", "string", "comment", "operator", "unit",
    "projectKeyword", "typeKeyword", "hardwareKeyword", "algorithmKeyword", "runtimeKeyword",
    "bindingKeyword", "flowKeyword", "projectName", "hardwareName", "algorithmName",
    "runtimeName", "typeName", "recordField",
]
_SEMANTIC_TOKEN_LEGEND = types.SemanticTokensLegend(token_types=_SEMANTIC_TOKEN_TYPES, token_modifiers=[])
_IGNORED_WORKSPACE_DIRS = {".git", ".venv", "venv", "node_modules", "build", "dist", "out", "third_party", "vendor"}

def _path_from_uri(uri: str) -> Path | None:
    """Convert a local file URI into a path; ignore virtual editor documents."""
    parsed = urlparse(uri)
    if parsed.scheme.lower() != "file":
        return None
    path = unquote(parsed.path)
    if os.name == "nt" and path.startswith("/") and re.match(r"^/[A-Za-z]:", path):
        path = path[1:]
    if parsed.netloc:
        path = f"//{parsed.netloc}{path}"
    return Path(path).resolve()

def _locate_uetd(source_path: Path) -> Path:
    """Resolve UETD from the explicit client setting, environment, or locator."""
    if _explicit_uetd_root is not None:
        return _explicit_uetd_root
    return UETDLocator(source_path.parent).locate()

@lru_cache(maxsize=4)
def _catalogue_for_root(root: str) -> AuthoringCatalogue:
    """Load a UETD catalogue once per root while the editor session is active."""
    return load_authoring_catalogue(Path(root))

def _catalogue_for_document(uri: str) -> AuthoringCatalogue | None:
    source_path = _path_from_uri(uri)
    if source_path is None:
        return None
    try:
        return _catalogue_for_root(str(_locate_uetd(source_path)))
    except (OSError, ValueError) as exc:
        logging.info("UETD registry unavailable for %s: %s", source_path, exc)
        return None

@lru_cache(maxsize=1)
def _language_registry():
    """Validate and retain the packaged syntax registry for this server session."""
    return load_language_registry()

def _column_to_utf16(line: str, one_based_column: int) -> int:
    """Convert a 1-based code-point column to a 0-based UTF-16 character offset."""
    col_index = max(0, min(one_based_column - 1, len(line)))
    substring = line[:col_index]
    return len(substring.encode("utf-16le")) // 2

def _utf16_to_codepoint(line: str, character: int) -> int:
    """Convert an LSP UTF-16 character offset to a Python string index."""
    remaining = max(0, character)
    for index, char in enumerate(line):
        units = 2 if ord(char) > 0xFFFF else 1
        if remaining < units:
            return index
        remaining -= units
    return len(line)

def _offset_from_lsp(source: str, position: types.Position) -> int:
    """Convert an LSP UTF-16 position to a Python code-point source offset."""
    lines = source.splitlines(keepends=True)
    if not lines:
        return 0
    line_index = max(0, min(position.line, len(lines) - 1))
    line = lines[line_index].rstrip("\r\n")
    return sum(len(item) for item in lines[:line_index]) + _utf16_to_codepoint(line, position.character)

@lru_cache(maxsize=24)
def _parse_partial_source(source: str, path: Path | None):
    """Return the parser's recoverable AST for editor features."""
    tokens, _ = Lexer(source, path).tokenize()
    document, _ = Parser(tokens, path).parse()
    return document

def _walk_nodes(value: object):
    """Walk source AST dataclasses once, skipping spans and path metadata."""
    seen: set[int] = set()
    def walk(item: object):
        if item is None or isinstance(item, (str, int, float, bool, Path)):
            return
        identity = id(item)
        if identity in seen:
            return
        seen.add(identity)
        yield item
        if is_dataclass(item):
            for field in fields(item):
                if field.name in {"span", "source_span", "path"}:
                    continue
                yield from walk(getattr(item, field.name))
        elif isinstance(item, dict):
            for child in item.values():
                yield from walk(child)
        elif isinstance(item, (tuple, list, set)):
            for child in item:
                yield from walk(child)
    yield from walk(value)

def _hint_context(uri: str, source: str, line_number: int, utf16_character: int, document_ast: object) -> tuple[HintContext, str]:
    """Infer a small editor context from the recoverable AST and current line."""
    lines = source.splitlines()
    line = lines[line_number] if 0 <= line_number < len(lines) else ""
    character = _utf16_to_codepoint(line, utf16_character)
    before_line = line[:character]
    word = re.search(r"[A-Za-z_][A-Za-z_0-9]*$", before_line)
    prefix = word.group(0) if word else ""
    line_prefix_lengths = [len(item) for item in source.splitlines(keepends=True)]
    before = source[:sum(line_prefix_lengths[:line_number]) + character]
    source_path = _path_from_uri(uri)
    if source_path is not None and source_path.suffix.casefold() == ".uefs":
        return HintContext("declaration", prefix=prefix, partial_ast=document_ast, source_path=str(source_path)), prefix
    import_match = re.search(r"\bcontract\s+import\s+['\"]([^'\"]*)$", before_line)
    if import_match and source_path is not None:
        return HintContext("import_path", prefix=import_match.group(1), partial_ast=document_ast, source_path=str(source_path)), import_match.group(1)
    descriptor_match = re.search(r"\b(sensor|actuator|algorithm|protocol|bus|peripheral)\s+[A-Za-z_][A-Za-z_0-9]*\s*:\s*([A-Za-z_][A-Za-z_0-9]*)?$", before_line)
    if descriptor_match:
        kind = descriptor_match.group(1)
        return HintContext("descriptor", prefix=prefix, expected_kind=kind, partial_ast=document_ast, source_path=str(source_path) if source_path else None), prefix
    if document_ast is not None:
        implementation = next((item for item in document_ast.declarations if item.kind == "implementation_block"), None)
        if implementation is not None:
            signatures = {item.name: item for item in document_ast.declarations if item.prototype and item.name}
            for body in implementation.children:
                signature = signatures.get(body.name)
                end_offset = getattr(getattr(body.span, "end", None), "offset", None)
                descriptor_prefix = re.fullmatch(r"[ \t]+([A-Za-z_][A-Za-z_0-9]*)?", source[end_offset:len(before)]) if isinstance(end_offset, int) and end_offset <= len(before) else None
                if signature is not None and signature.registry_key and descriptor_prefix is not None:
                    expected = signature.registry_key
                    return HintContext("implementation_descriptor", prefix=descriptor_prefix.group(1) or "",
                                       descriptor=expected, partial_ast=document_ast, source_path=str(source_path) if source_path else None), descriptor_prefix.group(1) or ""
    if re.search(r"\bruntime\s*=\s*[A-Za-z_][A-Za-z_0-9]*?$", before_line):
        return HintContext("runtime", prefix=prefix, partial_ast=document_ast), prefix
    if re.search(r"\btarget\s*=\s*[A-Za-z_][A-Za-z_0-9]*?$", before_line):
        return HintContext("target", prefix=prefix, partial_ast=document_ast), prefix
    if re.search(r"\b[A-Za-z_][A-Za-z_0-9]*\s*:\s*[A-Za-z_][A-Za-z_0-9]*?$", before_line):
        return HintContext("type", prefix=prefix, partial_ast=document_ast, source_path=str(source_path) if source_path else None), prefix
    symbols = build_project_symbol_table(document_ast) if document_ast is not None else None
    if document_ast is not None:
        implementation = next((item for item in document_ast.declarations if item.kind == "implementation_block"), None)
        if implementation and implementation.span and implementation.span.start.offset <= len(before) <= implementation.span.end.offset:
            signatures = {item.name: item for item in document_ast.declarations if item.prototype and item.name}
            body = next((item for item in implementation.children if item.span and item.span.start.offset <= len(before) <= item.span.end.offset), None)
            if body is not None and body.name in signatures:
                descriptor = signatures[body.name].registry_key
                return HintContext("component_field", prefix=prefix, descriptor=descriptor, partial_ast=document_ast, symbols=symbols), prefix
            return HintContext("implementation", prefix=prefix, partial_ast=document_ast, symbols=symbols), prefix
    mode = "declaration" if re.match(r"\s*(project|target|numeric|parameter|contract|operatingsystem|implementation)?\s*$", before_line) else "expression"
    return HintContext(mode, prefix=prefix, partial_ast=document_ast, symbols=symbols, source_path=str(source_path) if source_path else None), prefix

def _lsp_range(span: object | None, source_lines: list[str] | None = None) -> types.Range:
    if span is None:
        return types.Range(start=types.Position(line=0, character=0), end=types.Position(line=0, character=1))    
    start = span.start
    end = span.end    
    start_line_idx = max(0, start.line - 1)
    end_line_idx = max(0, end.line - 1)
    if source_lines and start_line_idx < len(source_lines):
        start_char = _column_to_utf16(source_lines[start_line_idx], start.column)
    else:
        start_char = max(0, start.column - 1)        
    if source_lines and end_line_idx < len(source_lines):
        end_char = _column_to_utf16(source_lines[end_line_idx], end.column)
    else:
        end_char = max(0, end.column - 1)
    return types.Range(
        start=types.Position(line=start_line_idx, character=start_char),
        end=types.Position(line=end_line_idx, character=end_char),
    )

def _diagnostics(uri: str, source: str) -> list[types.Diagnostic]:
    """Ask the compiler API for current diagnostics, then adapt them to LSP."""
    source_path = _path_from_uri(uri)
    if source_path is None:
        return []
    _LOGGER.debug("Analyzing document uri=%s source_path=%s characters=%d", uri, source_path, len(source))
    try:
        uetd_root = _locate_uetd(source_path)
        _LOGGER.debug("Resolved UETD root=%s for %s", uetd_root, source_path)
        if source_path.suffix.lower() == ".uefs":
            result = compile_system_text(source, source_path=source_path, uetd_root=uetd_root)
        else:
            result = compile_project_text(source, source_path=source_path, uetd_root=uetd_root)
    except (OSError, ValueError) as exc:
        _LOGGER.exception("Could not load UETD registry while analyzing %s", source_path)
        return [
            types.Diagnostic(
                range=_lsp_range(None),
                message=f"UEF-PL could not load its UETD registry: {exc}",
                severity=types.DiagnosticSeverity.Warning,
                code="uetd_registry_unavailable",
                source="uef-gen",
            )
        ]
    except Exception as exc:
        _LOGGER.exception("UEF-PL analysis failed while updating editor diagnostics for %s", source_path)
        return [types.Diagnostic(
            range=_lsp_range(None),
            message=f"UEF-PL analysis could not complete: {type(exc).__name__}: {exc}",
            severity=types.DiagnosticSeverity.Error,
            code="uef-pl-analysis-failed",
            source="uef-gen",
        )]
    adapted: list[types.Diagnostic] = []
    for diagnostic in result.diagnostics:
        severity = {
            "error": types.DiagnosticSeverity.Error,
            "warning": types.DiagnosticSeverity.Warning,
            "information": types.DiagnosticSeverity.Information,
            "hint": types.DiagnosticSeverity.Hint,
        }.get(diagnostic.severity, types.DiagnosticSeverity.Warning)
        # This status is a tooling limitation, not an error in the user's file.
        if diagnostic.code == "project_language_frontend_incomplete":
            severity = types.DiagnosticSeverity.Warning
        adapted.append(types.Diagnostic(range=_lsp_range(diagnostic.span, source.splitlines()), message=diagnostic.message, severity=severity, code=diagnostic.code, source="uef-gen"))
    if source_path.suffix.lower() == ".uef":
        adapted.extend(_registry_readiness_diagnostics(source, uetd_root))
    error_count = sum(item.severity == types.DiagnosticSeverity.Error for item in adapted)
    warning_count = sum(item.severity == types.DiagnosticSeverity.Warning for item in adapted)
    _LOGGER.info("Analysis complete path=%s diagnostics=%d errors=%d warnings=%d", source_path,
                 len(adapted), error_count, warning_count)
    for item in adapted:
        start = item.range.start
        code = getattr(item.code, "value", item.code)
        diagnostic_record = (
            "Diagnostic path=%s line=%d column=%d severity=%s code=%s source=%s message=%s",
            source_path, start.line + 1, start.character + 1, item.severity, code, item.source, item.message,
        )
        # Always retain errors and warnings in the default user log so users
        # can report what the editor showed without first enabling DEBUG.
        # Informational diagnostics and hints are available at DEBUG level.
        if item.severity == types.DiagnosticSeverity.Error:
            _LOGGER.error(*diagnostic_record)
        elif item.severity == types.DiagnosticSeverity.Warning:
            _LOGGER.warning(*diagnostic_record)
        else:
            _LOGGER.debug(*diagnostic_record)
    return adapted

def _registry_readiness_diagnostics(source: str, uetd_root: Path) -> list[types.Diagnostic]:
    """Add non-blocking readiness hints from declarations and UMATH AST calls."""
    try:
        catalogue = _catalogue_for_root(str(uetd_root))
    except (OSError, ValueError) as exc:
        logging.info("UETD authoring catalogue could not be loaded: %s", exc)
        return []
    tokens, _ = Lexer(source).tokenize()
    document, _ = Parser(tokens).parse()
    if document is None:
        return []
    results: list[types.Diagnostic] = []
    for declaration in document.declarations:
        if not declaration.prototype or not declaration.registry_key:
            continue
        binding = catalogue.dsl_descriptors.get(declaration.registry_key)
        if binding is None:
            continue
        contract = binding.contract
        if contract is None:
            message = f"UETD module '{binding.module_id}' is registered, but has no authoring contract. It is discoverable and not yet ready for generation."
        elif contract.generation.get("status") != "ready":
            status = contract.generation.get("status", "unspecified")
            message = f"UETD module '{binding.module_id}' has generation status '{status}'. Generation readiness is incomplete."
        else:
            continue
        results.append(types.Diagnostic(range=_lsp_range(declaration.span, source.splitlines()), message=message,
                                       severity=types.DiagnosticSeverity.Warning, code="uetd-module-not-ready", source="UETD registry"))
    for node in _walk_nodes(document):
        if not isinstance(node, MathCallExpression):
            continue
        functions = catalogue.math_functions.get(node.callee, ())
        if not functions:
            hidden = catalogue.math_functions_by_c_name.get(node.callee)
            if hidden is None:
                continue
            message = f"Raw C UMATH name is hidden; use UEF-PL name '{hidden.dsl_name}'."
            code = "uetd-umath-c-name-hidden"
        elif any(item.available_for_generation and item.dsl_binding_status == "ready" for item in functions):
            continue
        else:
            states = sorted({f"{item.implementation_status}/{item.dsl_binding_status}" for item in functions})
            message = f"UMATH overload set '{node.callee}' is registered, but no overload is ready for generation ({', '.join(states)})."
            code = "uetd-umath-not-ready"
        results.append(types.Diagnostic(range=_lsp_range(node.span, source.splitlines()), message=message,
                                       severity=types.DiagnosticSeverity.Warning, code=code, source="UETD UMATH registry"))
    return results

_DIAGNOSTICS_BY_URI: dict[str, tuple[int | None, list[types.Diagnostic]]] = {}
_PENDING_DIAGNOSTICS: dict[str, tuple[int, int | None]] = {}
_DIAGNOSTIC_TASKS: dict[str, asyncio.Task[None]] = {}
_DIAGNOSTIC_SEMAPHORE = asyncio.Semaphore(2)


def _start_diagnostic_worker(uri: str) -> None:
    task = _DIAGNOSTIC_TASKS.get(uri)
    if task is None or task.done():
        _DIAGNOSTIC_TASKS[uri] = asyncio.create_task(_diagnostic_worker(uri))


def _request_diagnostics(uri: str) -> None:
    """Queue the latest buffer version for debounced, background analysis."""
    if not uri.lower().endswith((".uef", ".uefs")):
        return
    try:
        document = server.workspace.get_text_document(uri)
    except KeyError:
        return
    revision = _PENDING_DIAGNOSTICS.get(uri, (0, None))[0] + 1
    _PENDING_DIAGNOSTICS[uri] = (revision, document.version)
    _start_diagnostic_worker(uri)


async def _diagnostic_worker(uri: str) -> None:
    """Analyze one URI at a time without blocking other language-server work."""
    task = asyncio.current_task()
    try:
        while uri in _PENDING_DIAGNOSTICS:
            request = _PENDING_DIAGNOSTICS[uri]
            await asyncio.sleep(0.25)
            if _PENDING_DIAGNOSTICS.get(uri) != request:
                continue
            try:
                document = server.workspace.get_text_document(uri)
            except KeyError:
                _PENDING_DIAGNOSTICS.pop(uri, None)
                return
            if document.version != request[1]:
                _PENDING_DIAGNOSTICS[uri] = (request[0] + 1, document.version)
                continue

            source = document.source
            async with _DIAGNOSTIC_SEMAPHORE:
                if _PENDING_DIAGNOSTICS.get(uri) != request:
                    continue
                diagnostics = await asyncio.to_thread(_diagnostics, uri, source)
            latest_request = _PENDING_DIAGNOSTICS.get(uri)
            try:
                current_document = server.workspace.get_text_document(uri)
            except KeyError:
                _PENDING_DIAGNOSTICS.pop(uri, None)
                return
            if latest_request != request or current_document.version != request[1]:
                if latest_request == request:
                    _PENDING_DIAGNOSTICS[uri] = (request[0] + 1, current_document.version)
                continue

            _DIAGNOSTICS_BY_URI[uri] = (document.version, diagnostics)
            _LOGGER.debug("Publishing diagnostics uri=%s version=%s count=%d", uri, document.version, len(diagnostics))
            server.text_document_publish_diagnostics(
                types.PublishDiagnosticsParams(uri=uri, version=document.version, diagnostics=diagnostics)
            )
            _PENDING_DIAGNOSTICS.pop(uri, None)
            return
    finally:
        if _DIAGNOSTIC_TASKS.get(uri) is task:
            _DIAGNOSTIC_TASKS.pop(uri, None)
            if uri in _PENDING_DIAGNOSTICS:
                _start_diagnostic_worker(uri)

@server.feature(types.TEXT_DOCUMENT_DID_OPEN)
def did_open(_ls: LanguageServer, params: types.DidOpenTextDocumentParams) -> None:
    """Validate the initial contents of a `.uef` or `.uefs` document."""
    _request_diagnostics(params.text_document.uri)

@server.feature(types.TEXT_DOCUMENT_DID_CHANGE)
def did_change(_ls: LanguageServer, params: types.DidChangeTextDocumentParams) -> None:
    """Debounce and coalesce compiler diagnostics during edits."""
    _request_diagnostics(params.text_document.uri)

@server.feature(types.TEXT_DOCUMENT_DID_SAVE)
def did_save(_ls: LanguageServer, params: types.DidSaveTextDocumentParams) -> None:
    """Refresh diagnostics after save and reload data when a registry changed."""
    saved_path = _path_from_uri(params.text_document.uri)
    if saved_path is not None and (saved_path.name == "api.json" or "registry" in saved_path.parts):
        _catalogue_for_root.cache_clear()
    _request_diagnostics(params.text_document.uri)


@server.feature(types.TEXT_DOCUMENT_DID_CLOSE)
def did_close(_ls: LanguageServer, params: types.DidCloseTextDocumentParams) -> None:
    """Release per-document diagnostics after the editor closes a buffer."""
    uri = params.text_document.uri
    _DIAGNOSTICS_BY_URI.pop(uri, None)
    _PENDING_DIAGNOSTICS.pop(uri, None)
    task = _DIAGNOSTIC_TASKS.get(uri)
    if task is not None:
        task.cancel()

def _registry_completions(uri: str, context: HintContext, source: str) -> list[types.CompletionItem]:
    """Adapt compiler hint providers to LSP completion items."""
    catalogue = _catalogue_for_document(uri)
    source_path = _path_from_uri(uri)
    if source_path is not None and source_path.suffix.casefold() == ".uefs":
        hints = [("keyword", word, "UEF-PL system language", "Keyword from the packaged .uefs grammar.", word, None)
                 for word in _system_language_words()
                 if not context.prefix or word.casefold().startswith(context.prefix.casefold())]
        system = context.partial_ast
        for name, kind, detail in _system_visible_names(system):
            if not context.prefix or name.casefold().startswith(context.prefix.casefold()):
                hints.append((kind, name, detail, f"Declared in this system contract as {kind}.", name, None))
    else:
        if context.mode == "import_path":
            hints = [(item.kind, item.label, item.detail, item.documentation, item.insert_text, item.data)
                     for item in complete_contract_import_paths(context)]
        else:
            if catalogue is None:
                return []
            try:
                registry = _language_registry()
            except (OSError, ValueError):
                return []
            hints = [
                (item.kind, item.label, item.detail, item.documentation, item.insert_text, item.data)
                for item in complete_language(context, registry, catalogue)
            ]
    kind_map = {
        "keyword": types.CompletionItemKind.Keyword,
        "descriptor": types.CompletionItemKind.Module,
        "builtin": types.CompletionItemKind.Function,
        "field": types.CompletionItemKind.Field,
        "value": types.CompletionItemKind.Value,
        "function": types.CompletionItemKind.Function,
        "type": types.CompletionItemKind.Struct,
        "target": types.CompletionItemKind.Class,
        "runtime": types.CompletionItemKind.Module,
    }
    items = [types.CompletionItem(label=label, kind=kind_map.get(kind, types.CompletionItemKind.Text), detail=detail,
                                  documentation=(types.MarkupContent(kind=types.MarkupKind.Markdown, value=documentation)
                                                 if documentation else None),
                                  insert_text=insert_text, data=data,
                                  insert_text_format=types.InsertTextFormat.Snippet if insert_text and "$0" in insert_text else types.InsertTextFormat.PlainText)
             for kind, label, detail, documentation, insert_text, data in hints]
    if context.mode == "type" and source_path is not None and source_path.suffix.casefold() == ".uef":
        from uef_gen.language.frontends.project.contract_imports import resolve_contract_import_path
        labels = {item.label for item in items}
        for import_decl in getattr(context.partial_ast, "contract_imports", ()):
            contract_path, diagnostics = resolve_contract_import_path(import_decl, source_path)
            if diagnostics or contract_path is None:
                continue
            contract_source = _source_for_path(contract_path)
            if contract_source is None:
                continue
            contract_system = _parse_editor_ast(contract_path, contract_source)[1]
            for collection in ("records", "enums", "variants", "flags_types", "fixed_point_types"):
                for declaration in getattr(contract_system, collection, ()):
                    name = getattr(declaration, "name", None)
                    if name and name not in labels and name.casefold().startswith(context.prefix.casefold()):
                        items.append(types.CompletionItem(label=name, kind=types.CompletionItemKind.Struct,
                                                          detail=f"Imported {collection.removesuffix('s')} from {contract_path.name}",
                                                          documentation=f"Declared by the explicitly imported contract `{contract_path.name}`.",
                                                          insert_text=name))
                        labels.add(name)
    if context.mode == "target":
        source_path = _path_from_uri(uri)
        if source_path is not None:
            try:
                from uef_gen.chips import load_chip_database
                from uef_gen.chips.development_boards import load_development_boards
                root = _locate_uetd(source_path)
                target_names = set(load_chip_database(root).names()) | {record["id"] for record in load_development_boards(root).values()}
                items.extend(types.CompletionItem(label=name, kind=types.CompletionItemKind.Class, detail="UETD target", insert_text=name)
                             for name in sorted(target_names, key=str.casefold)
                             if name.casefold().startswith(context.prefix.casefold()))
            except (OSError, ValueError):
                pass
    return items

def _local_component_outputs(project: object, catalogue: AuthoringCatalogue | None, name: str, seen: set[str] | None = None) -> list[tuple[str, str | None, str]]:
    """Describe public outputs for a registered or project-defined component."""
    visited = set() if seen is None else seen
    if name in visited:
        return []
    visited.add(name)
    signatures = [item for item in getattr(project, "declarations", ()) if item.name == name and item.prototype]
    signature = signatures[0] if len(signatures) == 1 else None
    values: list[tuple[str, str | None, str]] = []
    if signature and catalogue and signature.registry_key:
        binding = catalogue.dsl_descriptors.get(signature.registry_key)
        if binding and binding.contract:
            values.extend((str(port.get("name")), str(port.get("type")) if port.get("type") else None,
                           str(port.get("description", "Registered component output")))
                          for port in binding.contract.outputs if isinstance(port.get("name"), str))
    implementation = next((item for item in getattr(project, "declarations", ()) if item.kind == "implementation_block"), None)
    bodies = [item for item in getattr(implementation, "children", ()) if item.name == name] if implementation else []
    for body in bodies:
        values.extend((item.name, item.type_spec.name if item.type_spec else None, f"Declared {item.role} port")
                      for item in body.bindings if item.role == "output")
        values.extend((item.name, None, "Explicitly assigned composition output") for item in body.composition_outputs)
        alias = next((item.registry_key for item in body.children if item.kind == "use" and item.name == name), None)
        if alias:
            values.extend(_local_component_outputs(project, catalogue, alias, visited))
    if not values:
        for declaration in _walk_nodes(project):
            if getattr(declaration, "kind", None) == "use" and getattr(declaration, "name", None) == name:
                values.extend(_local_component_outputs(project, catalogue, getattr(declaration, "registry_key", ""), visited))
                break
    unique: dict[str, tuple[str, str | None, str]] = {}
    for item in values:
        unique.setdefault(item[0], item)
    return list(unique.values())

def _type_members(project: object, catalogue: AuthoringCatalogue | None, type_name: str) -> list[tuple[str, str | None, str]]:
    """Return fields/cases visible on a project, imported, or registered type."""
    received = re.fullmatch(r"Received\s*<\s*(.+)\s*>", type_name.strip())
    if received:
        # Mirror the Received<T> members used by project type-interchange:
        # value carries T; the other fields are runtime receiver metadata.
        payload_type = received.group(1).strip()
        return [
            ("value", payload_type, "Received<T> payload"),
            ("fresh", "bool", "Received<T> freshness metadata"),
            ("sequence", "u32", "Received<T> sequence metadata"),
            ("received_at_us", "u64<us>", "Received<T> timestamp metadata"),
        ]
    # A unit annotation belongs to the value, not to the identity of its
    # underlying record/vector type (for example ``vec3<rad/s>``).
    registered_type_name = re.sub(r"\s*<.*>\s*$", "", type_name).strip()
    for item in getattr(project, "records", ()):
        if item.name == type_name:
            return [(field.name, _hover_type_name(field.type_spec) if field.type_spec else None, "Record field") for field in item.fields]
    for collection in ("enums", "variants", "flags"):
        for item in getattr(project, collection, ()):
            if item.name != type_name:
                continue
            members = (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ()))
            return [(member.name, None, "Enum or flags member") for member in members]
    source_path = getattr(project, "source_path", None)
    if source_path:
        for import_decl in getattr(project, "contract_imports", ()):
            from uef_gen.language.frontends.project.contract_imports import resolve_contract_import_path
            imported_path, diagnostics = resolve_contract_import_path(import_decl, source_path)
            if diagnostics or imported_path is None:
                continue
            imported_source = _source_for_path(imported_path)
            if imported_source is None:
                continue
            imported_system = _parse_editor_ast(imported_path, imported_source)[1]
            for collection in ("records", "enums", "variants", "flags_types"):
                for item in getattr(imported_system, collection, ()):
                    if item.name != type_name:
                        continue
                    fields_ = getattr(item, "fields", ())
                    if fields_:
                        return [(field[0], _hover_type_name(field[1]) if field[1] else None, "Imported record field") for field in fields_]
                    members = (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ()))
                    return [(member.name, None, "Imported type member") for member in members]
    if catalogue:
        definitions = [item for values in catalogue.types.values() for item in values
                       if registered_type_name in {item.dsl_name, item.c_name}]
        if definitions:
            definition = definitions[0]
            if definition.language_contract == "fixed_numeric_vector":
                scalar = _project_scalar_type_name(project)
                return [(member, scalar, f"Component of `{definition.dsl_name or definition.c_name}`")
                        for member in _registered_vector_component_names(definition.declaration)]
            return [(member, None, f"Registered member of `{type_name}`") for member in definition.enum_members]
    return []


def _project_scalar_type_name(project: object) -> str:
    """Return the project's selected numeric scalar spelling for vector fields."""
    numeric = next((item for item in getattr(project, "declarations", ())
                    if getattr(item, "kind", None) == "numeric"), None)
    scalar = next((item for item in getattr(numeric, "properties", ())
                   if getattr(item, "name", None) == "scalar"), None)
    value = getattr(getattr(scalar, "value", None), "value", None)
    if isinstance(value, str) and value:
        return value
    return "f32"


def _registered_struct_body(declaration: str, c_name: str) -> str | None:
    """Extract the field body of one exact typedef struct declaration."""
    pattern = re.compile(
        rf"\btypedef\s+struct(?:\s+[A-Za-z_]\w*)?\s*\{{(?P<body>.*?)\}}\s*"
        rf"(?:[A-Za-z_]\w*\s+)?{re.escape(c_name)}\s*;",
        re.DOTALL,
    )
    match = pattern.search(declaration)
    return match.group("body") if match else None


def _registered_vector_component_names(declaration: str) -> tuple[str, ...]:
    """Read simple scalar field names from the registry's fixed-vector contract."""
    body = _registered_struct_body(declaration, _registered_struct_typedef_name(declaration))
    if body is None:
        return ()
    return _scalar_struct_field_names(body)


def _registered_struct_typedef_name(declaration: str) -> str:
    """Get the typedef name from the final declarator of a registry struct."""
    match = re.search(r"\}\s*(?:[A-Za-z_]\w*\s+)?([A-Za-z_]\w*)\s*;\s*$", declaration.strip(), re.DOTALL)
    return match.group(1) if match else ""


def _scalar_struct_field_names(body: str) -> tuple[str, ...]:
    """Return field names for ordinary scalar declarations, ignoring C comments."""
    clean = re.sub(r"/\*.*?\*/|//[^\r\n]*", "", body, flags=re.DOTALL)
    names: list[str] = []
    for statement in clean.split(";"):
        match = re.fullmatch(r"\s*[A-Za-z_]\w*\s+([A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*)\s*", statement)
        if match:
            names.extend(re.findall(r"[A-Za-z_]\w*", match.group(1)))
    return tuple(names)


def _uetd_vector_member_declaration(name: str, type_name: str, catalogue: AuthoringCatalogue | None,
                                    source_path: Path) -> tuple[Path, str, object] | None:
    """Find a fixed-vector component in its registered UETD header."""
    if catalogue is None:
        return None
    base_type = re.sub(r"\s*<.*>\s*$", "", type_name).strip()
    definition = next((item for values in catalogue.types.values() for item in values
                       if base_type in {item.dsl_name, item.c_name}
                       and item.language_contract == "fixed_numeric_vector"), None)
    if definition is None:
        return None
    declared_names = _registered_vector_component_names(definition.declaration)
    if name not in declared_names:
        return None
    try:
        root = _locate_uetd(source_path)
        target = (root / definition.header).resolve(strict=False)
        if not target.is_relative_to(root) or not target.is_file():
            return None
        text = target.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError, ValueError):
        return None
    body = _registered_struct_body(text, definition.c_name)
    if body is None or _scalar_struct_field_names(body) != declared_names:
        return None
    # Re-extract the matching simple scalar declarator to avoid matching a
    # comment, a different declaration, or another type's same-named field.
    type_match = re.search(
        rf"\btypedef\s+struct(?:\s+[A-Za-z_]\w*)?\s*\{{(?P<body>.*?)\}}\s*"
        rf"(?:[A-Za-z_]\w*\s+)?{re.escape(definition.c_name)}\s*;",
        text,
        re.DOTALL,
    )
    if type_match is None:
        return None
    body_text = type_match.group("body")
    clean_body = re.sub(r"/\*.*?\*/|//[^\r\n]*", lambda match: " " * len(match.group(0)), body_text, flags=re.DOTALL)
    for statement in re.finditer(r"[^;]*;", clean_body):
        field_decl = statement.group(0)[:-1]
        parsed = re.fullmatch(r"\s*[A-Za-z_]\w*\s+([A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*)\s*", field_decl)
        if parsed is None or name not in re.findall(r"[A-Za-z_]\w*", parsed.group(1)):
            continue
        field_match = re.search(rf"(?<![A-Za-z0-9_]){re.escape(name)}(?![A-Za-z0-9_])", field_decl)
        if field_match is None:
            continue
        start = type_match.start("body") + statement.start() + field_match.start()
        span = _span_from_offsets(target, text, start, start + len(name))
        return target, text, span
    return None

def _channel_payload_type(project: object, catalogue: AuthoringCatalogue | None, channel_name: str) -> str | None:
    """Resolve a channel's declared source to its payload type for editor help."""
    channel = next((item for item in _walk_nodes(project)
                    if type(item).__name__ == "Declaration"
                    and getattr(item, "kind", None) == "channel"
                    and getattr(item, "name", None) == channel_name), None)
    if channel is None:
        return None
    source = next((item.value for item in getattr(channel, "properties", ()) if item.name == "source"), None)
    source_path = tuple(getattr(source, "path", ()))
    if not source_path:
        value = getattr(source, "value", None)
        source_path = (value,) if isinstance(value, str) else ()
    if not source_path:
        return None
    outputs = _local_component_outputs(project, catalogue, source_path[0])
    selected = next((item for item in outputs if item[0] == source_path[1]), None) if len(source_path) > 1 else (outputs[0] if len(outputs) == 1 else None)
    if selected is None or not selected[1]:
        return None
    type_name = selected[1]
    generic = re.fullmatch(r"Received\s*<\s*([^<>]+)\s*>", type_name)
    if generic:
        type_name = generic.group(1).strip()
    for member in source_path[2:]:
        field = next((item for item in _type_members(project, catalogue, type_name) if item[0] == member), None)
        if field is None or not field[1]:
            return None
        type_name = field[1]
    return type_name

def _qualified_receiver_before(source: str, offset: int) -> str | None:
    """Return the dotted identifier path immediately before a member token.

    Search the source prefix rather than just the current line so a formatted
    expression such as ``state.value.\n    uncertainty`` resolves like a
    single-line path. The expression must end in a member-access dot.
    """
    prefix = source[:max(0, min(offset, len(source)))]
    match = re.search(r"([A-Za-z_][A-Za-z_0-9]*(?:\s*\.\s*[A-Za-z_][A-Za-z_0-9]*)*)\.\s*$", prefix)
    if match is None:
        return None
    return re.sub(r"\s*\.\s*", ".", match.group(1))

def _members_for_receiver(project: object, catalogue: AuthoringCatalogue | None, receiver: str,
                          offset: int | None = None) -> list[tuple[str, str | None, str]]:
    """Resolve a dotted receiver through component outputs and declared records."""
    parts = receiver.split(".")
    if not parts or not parts[0]:
        return []
    channel_type = _channel_payload_type(project, catalogue, parts[0])
    routine_members = [
        (getattr(item, "name", ""), None, "Project-owned zero-argument callback")
        for item in getattr(project, "declarations", ())
        if getattr(item, "kind", None) == "project_routine"
        and getattr(item, "registry_key", None) == parts[0]
        and getattr(item, "name", None)
    ]
    if channel_type:
        if len(parts) == 1:
            return [("value", channel_type, "Received channel payload")]
        if parts[1] != "value":
            return []
        type_name = channel_type
        remaining = parts[2:]
    else:
        members = _local_component_outputs(project, catalogue, parts[0])
        if members:
            if len(parts) == 1:
                return [*members, *routine_members]
            selected = next((item for item in members if item[0] == parts[1]), None)
            if selected is None or not selected[1]:
                return []
            type_name = selected[1]
            remaining = parts[2:]
        else:
            selected_binding = (_nearest_project_binding(project, parts[0], offset)
                                if offset is not None else None)
            typed_bindings = [item for item in _walk_nodes(project)
                              if getattr(item, "name", None) == parts[0] and getattr(item, "type_spec", None) is not None]
            if selected_binding is not None and getattr(selected_binding, "type_spec", None) is not None:
                type_name = _hover_type_name(selected_binding.type_spec)
                remaining = parts[1:]
            elif typed_bindings:
                type_name = _hover_type_name(typed_bindings[0].type_spec)
                remaining = parts[1:]
            else:
                if len(parts) > 1:
                    return []
                direct = _type_members(project, catalogue, parts[0])
                return [*direct, *routine_members]
    for member_name in remaining:
        current = _type_members(project, catalogue, type_name)
        selected = next((item for item in current if item[0] == member_name), None)
        if selected is None or not selected[1]:
            return []
        type_name = selected[1]
    members = _type_members(project, catalogue, type_name)
    unit_match = re.fullmatch(r"\s*[^<>]+<(.+)>\s*", type_name)
    if unit_match and any(detail.startswith("Component of ") for _name, _kind, detail in members):
        unit = unit_match.group(1)
        members = [(name, f"{member_type}<{unit}>" if member_type else None, detail)
                   for name, member_type, detail in members]
    return members

def _member_completion_items(uri: str, source: str, line_number: int, line: str, prefix_match: re.Match[str]) -> list[types.CompletionItem]:
    """Offer members of UETD component outputs and project record types."""
    path = _path_from_uri(uri)
    if path is None or path.suffix.casefold() != ".uef":
        return []
    project = _parse_partial_source(source, path)
    members = _members_for_receiver(project, _catalogue_for_document(uri), prefix_match.group(1))
    member_prefix = prefix_match.group(2) or ""
    start = prefix_match.start(2) if prefix_match.group(2) is not None else prefix_match.end()
    edit_range = types.Range(start=types.Position(line_number, _column_to_utf16(line, start + 1)),
                              end=types.Position(line_number, _column_to_utf16(line, start + len(member_prefix) + 1)))
    result = []
    prefix = member_prefix.casefold()
    for name, type_name, detail in members:
        if prefix and not name.casefold().startswith(prefix):
            continue
        result.append(types.CompletionItem(label=name, kind=types.CompletionItemKind.Field,
                                           detail=f"{type_name}: {detail}" if type_name else detail,
                                           documentation=types.MarkupContent(
                                               kind=types.MarkupKind.Markdown,
                                               value=f"### `{receiver}.{name}`\n\n{detail}" + (f"\n\n**Type**  `{type_name}`" if type_name else "")),
                                           text_edit=types.TextEdit(range=edit_range, new_text=name)))
    return result

def _member_hover_content(project: object, catalogue: AuthoringCatalogue | None, receiver: str, member_name: str,
                          offset: int | None = None) -> str | None:
    member = next((item for item in _members_for_receiver(project, catalogue, receiver, offset) if item[0] == member_name), None)
    if member is None:
        return None
    detail = member[2].strip()
    title = "Vector component" if detail.startswith("Component of ") else detail
    owner = detail.removeprefix("Component of ") if title == "Vector component" else None
    result = f"**`{receiver}.{member_name}`** : `{member[1] or 'component'}`"
    if owner:
        result += f"  \nVector component of `{owner.strip('`')}`"
    else:
        result += f"  \n{title}"
    return result

@lru_cache(maxsize=1)
def _system_language_words() -> tuple[str, ...]:
    """Derive .uefs keyword suggestions from its declarative grammar registry."""
    registry_path = Path(__file__).parents[1] / "system" / "registry.json"
    try:
        data = json.loads(registry_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return ("system", "node", "external", "record", "enum", "link", "custom", "protocol", "hybrid", "delivery_policy")
    productions = data.get("grammar", {}).get("productions", {})
    words = set(re.findall(r"'([a-z][a-z0-9_]*)'", " ".join(str(value) for value in productions.values())))
    return tuple(sorted(words, key=str.casefold))

def _system_visible_names(system: object | None) -> list[tuple[str, str, str]]:
    """Collect local names that can complete references in a parsed .uefs file."""
    if system is None:
        return []
    values: list[tuple[str, str, str]] = [(system.name, "value", "system declaration")]
    for collection, kind, detail in (
        ("nodes", "module", "system node"), ("external_endpoints", "module", "external endpoint"),
        ("records", "type", "shared record"), ("enums", "type", "enum"), ("variants", "type", "variant"),
        ("flags_types", "type", "flags type"), ("fixed_point_types", "type", "fixed-point type"),
        ("protocols", "module", "custom protocol"), ("links", "value", "system link"), ("hybrid_systems", "module", "hybrid system"),
    ):
        values.extend((item.name, kind, detail) for item in getattr(system, collection, ()) if getattr(item, "name", None))
    for policy in getattr(system, "delivery_policies", ()):
        if policy.get("name"):
            values.append((policy["name"], "value", "delivery policy"))
    return values

@server.feature(types.TEXT_DOCUMENT_COMPLETION, types.CompletionOptions(trigger_characters=_IDENTIFIER_TRIGGERS))
def completion(_ls: LanguageServer, params: types.CompletionParams) -> list[types.CompletionItem]:
    """Offer syntax-context-filtered language and UETD registry entries."""
    document = server.workspace.get_text_document(params.text_document.uri)
    source_path = _path_from_uri(params.text_document.uri)
    if params.position.line < len(document.lines):
        line = document.lines[params.position.line]
        cursor = _utf16_to_codepoint(line, params.position.character)
        member_match = re.search(r"(?<![A-Za-z0-9_])([A-Za-z_][A-Za-z_0-9]*(?:\.[A-Za-z_][A-Za-z_0-9]*)*)\.([A-Za-z_][A-Za-z_0-9]*)?$", line[:cursor])
        if member_match:
            return _member_completion_items(params.text_document.uri, document.source, params.position.line, line, member_match)
    if source_path and source_path.suffix.casefold() == ".uefs":
        tokens, _ = Lexer(document.source, source_path).tokenize()
        ast, _ = SystemParser(tokens, source_path).parse()
    else:
        ast = _parse_partial_source(document.source, source_path)
    context, _ = _hint_context(params.text_document.uri, document.source, params.position.line, params.position.character, ast)
    items = _registry_completions(params.text_document.uri, context, document.source)
    if params.position.line < len(document.lines):
        line = document.lines[params.position.line]
        cursor = _utf16_to_codepoint(line, params.position.character)
        match = re.search(r"[A-Za-z_][A-Za-z_0-9/.-]*$", line[:cursor])
        if context.mode == "import_path":
            path_match = re.search(r"['\"]([^'\"]*)$", line[:cursor])
            start = path_match.start(1) if path_match else cursor
        else:
            start = match.start() if match else cursor
        edit_range = types.Range(
            start=types.Position(line=params.position.line, character=_column_to_utf16(line, start + 1)),
            end=types.Position(line=params.position.line, character=params.position.character),
        )
        for item in items:
            if item.insert_text is not None:
                item.text_edit = types.TextEdit(range=edit_range, new_text=item.insert_text)
                item.insert_text = None
    return items

def _word_at(line: str, character: int) -> tuple[str, int, int] | None:
    """Find the identifier under an LSP cursor position."""
    preceding: tuple[str, int, int] | None = None
    for match in _WORD.finditer(line):
        # LSP ranges are half-open. At a token boundary prefer the token to
        # the right; the preceding identifier no longer contains the cursor.
        if match.start() <= character < match.end() or (character == len(line) and match.end() == character):
            return match.group(0), match.start(), match.end()
        # Some editor gesture providers report the cursor at the token's end
        # (rather than on its final character), especially for Ctrl+Click and
        # Quick Info. Keep the normal half-open/right-token behavior above,
        # then use the immediately preceding identifier only when no token
        # starts at this boundary. This avoids making adjacent identifiers
        # resolve to the wrong side while accepting punctuation-edge positions.
        if match.end() == character:
            preceding = (match.group(0), match.start(), match.end())
    return preceding

def _name_offsets(name: str, span: object | None, source: str) -> tuple[int, int] | None:
    """Return the first exact identifier occurrence inside a declaration span."""
    if span is None or not name:
        return None
    start = max(0, getattr(getattr(span, "start", None), "offset", 0))
    end = min(len(source), getattr(getattr(span, "end", None), "offset", len(source)))
    if end < start:
        return None
    found = re.search(rf"(?<![A-Za-z0-9_]){re.escape(name)}(?![A-Za-z0-9_])", source[start:end])
    return (start + found.start(), start + found.end()) if found else None

def _location_for_span(span: object | None, fallback_path: Path | None = None, *, name: str | None = None, source: str | None = None) -> types.Location | None:
    """Convert a source span to a location, narrowing it to the declared name when possible."""
    if span is None:
        return None
    span_path = getattr(span, "path", None)
    path = span_path or fallback_path
    if path is None:
        return None
    path = Path(path).resolve(strict=False)
    # A span can point into a registry header even though the request originated
    # in a `.uef` document. Never interpret the caller's document text using an
    # unrelated file's offsets; that produces invalid Ctrl+Click line/columns.
    source_matches_path = (
        source is not None
        and (span_path is None or fallback_path is None
             or Path(span_path).resolve(strict=False) == Path(fallback_path).resolve(strict=False))
    )
    try:
        text = source if source_matches_path else path.read_text(encoding="utf-8-sig")
        lines = text.splitlines()
    except (OSError, UnicodeError):
        text = ""
        lines = []
    selected_span = span
    offsets = _name_offsets(name, span, text) if name and text else None
    if offsets:
        selected_span = _span_from_offsets(path, text, *offsets)
    return types.Location(uri=path.as_uri(), range=_lsp_range(selected_span, lines))

def _span_from_offsets(path: Path, source: str, start_offset: int, end_offset: int):
    """Create the lexer's one-based, code-point span for an exact source slice."""
    from uef_gen.language.frontends.project.ast import SourcePosition, SourceSpan
    def position(offset: int):
        prefix = source[:offset]
        line = prefix.count("\n") + prefix.count("\r") - prefix.count("\r\n") + 1
        line_start = max(prefix.rfind("\n"), prefix.rfind("\r")) + 1
        return SourcePosition(offset, line, offset - line_start + 1)
    return SourceSpan(path, position(start_offset), position(end_offset))

def _system_declaration_location(system: object, name: str, source: str | None = None) -> types.Location | None:
    """Find a node, endpoint, shared type, protocol, or link in a system AST."""
    for node in getattr(system, "nodes", ()):
        if node.name == name:
            project = (system.source_path.parent / node.project_path).resolve(strict=False) if system.source_path else node.project_path
            zero = types.Range(start=types.Position(line=0, character=0), end=types.Position(line=0, character=0))
            return types.Location(uri=Path(project).as_uri(), range=zero)
    collections = ("external_endpoints", "records", "enums", "protocols", "links", "hybrid_systems", "variants", "flags_types", "fixed_point_types")
    for collection_name in collections:
        for item in getattr(system, collection_name, ()):
            if getattr(item, "name", None) == name:
                return _location_for_span(getattr(item, "source_span", getattr(item, "span", None)), getattr(system, "source_path", None), name=name, source=source)
            for field in getattr(item, "fields", ()):
                if isinstance(field, tuple):
                    field_name, _, field_properties = field
                    field_span = field_properties.get("source_span", getattr(item, "source_span", None))
                else:
                    field_name = getattr(field, "name", None)
                    field_span = getattr(field, "span", None)
                if field_name == name:
                    return _location_for_span(field_span, getattr(system, "source_path", None), name=name, source=source)
            for nested_name in ("messages", "fragments", "transitions", "states", "variants", "cases", "members"):
                for child in getattr(item, nested_name, ()):
                    if isinstance(child, dict):
                        child_name = child.get("name")
                        child_span = child.get("source_span")
                    else:
                        child_name = getattr(child, "name", None)
                        child_span = getattr(child, "source_span", getattr(child, "span", None))
                    if child_name == name:
                        return _location_for_span(child_span, getattr(system, "source_path", None), name=name, source=source)
                    if isinstance(child, dict):
                        for field in child.get("fields", ()):
                            if isinstance(field, dict) and field.get("name") == name:
                                return _location_for_span(field.get("source_span"), getattr(system, "source_path", None), name=name, source=source)
    return None

def _hover_type_name(type_spec: object | None) -> str:
    """Format a UEF-PL type for editor hover without dropping units or shapes."""
    if type_spec is None:
        return "unspecified"
    name = getattr(type_spec, "name", "unspecified")
    arguments = getattr(type_spec, "type_arguments", ())
    if arguments:
        name += "<" + ", ".join(_hover_type_name(argument) for argument in arguments) + ">"
    shape = getattr(type_spec, "shape", ())
    if shape:
        name += "[" + ", ".join(map(str, shape)) + "]"
    unit = getattr(type_spec, "unit", None)
    if unit:
        name += f"<{unit}>"
    return name


def _project_implementation_for(document_ast: object, name: str) -> object | None:
    """Return the authored implementation paired with a public project signature."""
    signature = next((node for node in getattr(document_ast, "declarations", ())
                      if getattr(node, "name", None) == name and getattr(node, "prototype", False)), None)
    signature_kind = getattr(signature, "kind", None)
    return next((node for node in _walk_nodes(document_ast)
                 if type(node).__name__ == "Declaration"
                 and getattr(node, "name", None) == name
                 and not getattr(node, "prototype", False)
                 and (signature_kind is None or getattr(node, "kind", None) == signature_kind)
                 and getattr(node, "kind", None) != "implementation_block"), None)


def _hover_source_excerpt(source: str, span: object | None, limit: int = 180) -> str | None:
    """Return a compact single-line view of authored source for hover cards."""
    bounds = _span_offsets(span)
    if not source or bounds is None:
        return None
    start, end = max(0, bounds[0]), min(len(source), bounds[1])
    if end <= start:
        return None
    excerpt = re.sub(r"\s+", " ", source[start:end]).strip()
    if not excerpt:
        return None
    return excerpt if len(excerpt) <= limit else excerpt[:limit - 1].rstrip() + "…"


def _hover_value_text(value: object, source: str = "", span: object | None = None,
                      limit: int = 120) -> str:
    """Format a bounded AST value for hover without exposing Python reprs."""
    authored = _hover_source_excerpt(source, span or getattr(value, "span", None), limit)
    if authored:
        return authored
    value_type = type(value).__name__
    if value_type == "Literal":
        unit = getattr(value, "unit", None)
        return f"{getattr(value, 'value', '')}{' ' + unit if unit else ''}"
    if value_type == "Identifier":
        return str(getattr(value, "value", ""))
    if value_type == "Reference":
        return ".".join(getattr(value, "path", ()))
    if value_type == "UnaryExpression":
        return f"{getattr(value, 'operator', '')}{_hover_value_text(getattr(value, 'operand', None), limit=limit)}"
    if value_type == "BinaryExpression":
        left = _hover_value_text(getattr(value, "left", None), limit=limit)
        right = _hover_value_text(getattr(value, "right", None), limit=limit)
        return f"{left} {getattr(value, 'operator', '?')} {right}"
    if value_type == "MathCallExpression":
        args = ", ".join(_hover_value_text(item, limit=limit) for item in getattr(value, "arguments", ()))
        return f"{getattr(value, 'callee', '?')}({args})"
    if value_type == "CastExpression":
        return f"cast<{_hover_type_name(getattr(value, 'destination', None))}>"
    if value_type == "UnitConversionExpression":
        return f"convert_unit<{getattr(value, 'target_unit', '?')}>"
    if value_type == "ListValue":
        items = getattr(value, "items", ())
        body = ", ".join(_hover_value_text(item, limit=limit) for item in items[:6])
        return "[" + body + (", …" if len(items) > 6 else "") + "]"
    if value_type == "RecordValue":
        fields_ = getattr(value, "fields", ())
        body = ", ".join(f"{key} = {_hover_value_text(item, limit=limit)}" for key, item in fields_[:6])
        return "map(" + body + (", …" if len(fields_) > 6 else "") + ")"
    if value_type == "MatrixLiteral":
        rows = getattr(value, "rows", ())
        return f"matrix[{len(rows)} x {len(rows[0]) if rows else 0}]"
    if isinstance(value, dict):
        return ", ".join(f"{key} = {_hover_value_text(item, limit=limit)}" for key, item in list(value.items())[:6])
    if isinstance(value, (tuple, list)):
        return ", ".join(_hover_value_text(item, limit=limit) for item in value[:6])
    if isinstance(value, (str, int, float, bool)):
        return str(value)
    return value_type if value is not None else "unspecified"


def _type_hover_content(item: object, label: str, source: str = "", signature: str | None = None) -> str:
    """Describe a project- or system-owned data type and its public layout."""
    name = str(getattr(item, "name", "<unnamed>"))
    item_type = type(item).__name__
    lines = ([f"```c\n{signature}\n```", f"\n**{label}**"] if signature else
             [f"**`{name}`**  |  {label}"])
    fields_ = getattr(item, "fields", ())
    if fields_:
        lines.append("\n**Fields**")
        for field in fields_[:8]:
            if isinstance(field, tuple):
                field_name = str(field[0]) if field else "<unnamed>"
                type_spec = field[1] if len(field) > 1 else None
            else:
                field_name = str(getattr(field, "name", "<unnamed>"))
                type_spec = getattr(field, "type_spec", None)
            lines.append(f"- `{field_name}` : `{_hover_type_name(type_spec)}`")
        if len(fields_) > 8:
            lines.append(f"- {len(fields_) - 8} more fields")
    variants = getattr(item, "variants", ())
    cases = getattr(item, "cases", ())
    members = getattr(item, "members", ())
    if item_type == "EnumDefinition":
        lines.append(f"Storage  `{getattr(item, 'storage_type', 'unspecified')}`")
        if variants:
            lines.append("\n**Cases**")
            lines.extend(f"- `{entry.name}`" + (f" = `{entry.value}`" if entry.value is not None else "")
                         for entry in variants[:8])
            if len(variants) > 8:
                lines.append(f"- {len(variants) - 8} more cases")
    elif item_type == "VariantDefinition":
        lines.append(f"Tag type  `{getattr(item, 'tag_type', 'unspecified')}`")
        if cases:
            lines.append("\n**Cases**")
            for case in cases[:6]:
                payload = ", ".join(f"`{field.name}` : `{_hover_type_name(field.type_spec)}`"
                                     for field in getattr(case, "fields", ())[:5])
                suffix = f"({payload})" if payload else ""
                tag = getattr(case, "tag", None)
                lines.append(f"- `{case.name}`" + (f" = `{tag}`" if tag is not None else "") + suffix)
            if len(cases) > 6:
                lines.append(f"- {len(cases) - 6} more cases")
    elif item_type == "FlagsDefinition":
        lines.append(f"Storage  `{getattr(item, 'storage_type', 'unspecified')}`")
        if members:
            lines.append("\n**Bits**")
            lines.extend(f"- `{entry.name}` : bit `{entry.bit}`" for entry in members[:8])
            if len(members) > 8:
                lines.append(f"- {len(members) - 8} more members")
    elif item_type == "FixedPointDefinition":
        lines.extend((f"Storage  `{getattr(item, 'storage_type', 'unspecified')}`",
                      f"Fractional bits  `{getattr(item, 'fractional_bits', 'unspecified')}`",
                      f"Unit  `{getattr(item, 'unit', 'unspecified')}`",
                      f"Rounding  `{getattr(item, 'rounding', 'unspecified')}`",
                      f"Overflow  `{getattr(item, 'overflow', 'unspecified')}`"))
    boundary = getattr(item, "boundary", None)
    if boundary is not None:
        details = [f"{getattr(boundary, 'kind', 'boundary')} v{getattr(boundary, 'version', '?')}"]
        for key, title in (("schema_id", "schema"), ("alignment", "alignment"),
                           ("memory_order", "memory order"), ("cache_policy", "cache"),
                           ("integrity", "integrity"), ("migration_policy", "migration")):
            value = getattr(boundary, key, None)
            if value is not None:
                details.append(f"{title} `{value}`")
        lines.append("\nBoundary  " + "  |  ".join(details))
        offsets = getattr(boundary, "field_offsets", ())
        encodings = getattr(boundary, "field_encodings", ())
        if offsets:
            lines.append("Field offsets  " + ", ".join(f"`{field}` = `{offset}`" for field, offset in offsets[:10]))
        if encodings:
            lines.append("Field encodings  " + ", ".join(f"`{field}` = `{encoding}`" for field, encoding in encodings[:10]))
        atomic = getattr(boundary, "atomic_fields", ())
        if atomic:
            lines.append("Atomic fields  " + ", ".join(f"`{field}`" for field in atomic[:10]))
        participants = [f"{label} `{value}`" for label, value in (("producer", getattr(boundary, "producer", None)),
                       ("consumer", getattr(boundary, "consumer", None))) if value]
        if participants:
            lines.append("Boundary ownership  " + "  |  ".join(participants))
        barriers = getattr(boundary, "memory_barriers", ())
        if barriers:
            lines.append("Memory barriers  " + ", ".join(f"`{barrier}`" for barrier in barriers[:8]))
        publication = getattr(boundary, "publication_sequence_field", None)
        if publication:
            lines.append(f"Publication sequence field  `{publication}`")
    return "\n".join(lines)


def _fsm_hover_lines(declaration: object, source: str) -> list[str]:
    """Summarize explicit state, guarded transition, and entry/exit action declarations."""
    children = list(getattr(declaration, "children", ()))
    states = [item for item in children if getattr(item, "kind", None) == "machine_state"]
    transitions = [item for item in children if getattr(item, "kind", None) == "transition"]
    actions = [item for item in children if getattr(item, "kind", None) == "state_action"]
    lines = [f"State type  `{getattr(declaration, 'registry_key', 'unspecified')}`"]
    initial = next((item.value for item in getattr(declaration, "properties", ()) if item.name == "initial"), None)
    if initial is not None:
        lines.append(f"Initial state  `{_hover_value_text(initial, source)}`")
    if states:
        lines.append("\n**States**  " + ", ".join(f"`{item.name}`" for item in states[:8]))
        if len(states) > 8:
            lines.append(f"{len(states) - 8} more states")
    if transitions:
        lines.append("\n**Transitions**")
        for item in transitions[:5]:
            guard = next((prop for prop in getattr(item, "properties", ()) if prop.name == "when"), None)
            condition = _hover_value_text(getattr(guard, "value", None), source, limit=100) if guard else "unspecified"
            lines.append(f"- `{item.name} → {item.registry_key}` when `{condition}`")
        if len(transitions) > 5:
            lines.append(f"- {len(transitions) - 5} more transitions")
    if actions:
        lines.append("\n**State actions**")
        for item in actions[:5]:
            operation = next((prop.value for prop in getattr(item, "properties", ()) if prop.name == "operation"), None)
            lines.append(f"- `{item.registry_key} {item.name}` calls `{_hover_value_text(operation, source)}`")
        if len(actions) > 5:
            lines.append(f"- {len(actions) - 5} more actions")
    return lines


def _declaration_hover_content(document_ast: object, declaration: object, name: str,
                               catalogue: AuthoringCatalogue | None, source: str = "") -> str:
    """Build a compact, readable interface hover for one project declaration."""
    kind = str(getattr(declaration, "kind", "declaration")).replace("_", " ").lower()
    if kind == "project routine":
        owner = getattr(declaration, "registry_key", None)
        qualified = f"{owner}.{name}" if owner else name
        context = next((prop.value for prop in getattr(declaration, "properties", ()) if prop.name == "context"), None)
        owner_signature = next((item for item in getattr(document_ast, "declarations", ())
                                if owner and getattr(item, "name", None) == owner and getattr(item, "prototype", False)), None)
        owner_descriptor = getattr(owner_signature, "registry_key", None)
        owner_binding = catalogue.dsl_descriptors.get(owner_descriptor) if catalogue and owner_descriptor else None
        text = f"```c\nvoid {name}(void);\n```\n\nProject-owned routine `{qualified}`. Zero-argument, no-return callback declared by this project."
        text += f"\n\nExecution context  `{_hover_value_text(context) if context else 'task'}`"
        if owner_descriptor:
            text += f"\nOwner  `{owner}`  |  UEF-PL `{owner_descriptor}`"
            if owner_binding:
                text += f"  |  UETD `{owner_binding.module_id}`"
        return text
    if kind in {"record", "enum", "variant", "flags", "fixed point"}:
        collection = {"record": "records", "enum": "enums", "variant": "variants",
                      "flags": "flags", "fixed point": "fixed_point_types"}[kind]
        definition = next((item for item in getattr(document_ast, collection, ()) if item.name == name), None)
        if definition is not None:
            return _type_hover_content(definition, kind.title(), source)
    interface = _project_implementation_for(document_ast, name) or declaration
    # A C-like signature gives Visual Studio's native Markdown hover a
    # syntax-colourable symbol line, similar to its built-in C Quick Info.
    signature_kind = "".join(part.title() for part in kind.split()) or "Declaration"
    lines = [f"```c\n{signature_kind} {name}(void);\n```"]
    if getattr(declaration, "kind", None) == "channel":
        payload_type = _channel_payload_type(document_ast, catalogue, name)
        if payload_type:
            lines.append(f"Payload  `Received<{payload_type}>`")
    state_machine_declaration = getattr(declaration, "kind", None) == "state_machine"
    descriptor = (None if state_machine_declaration else
                  getattr(declaration, "registry_key", None)
                  or getattr(interface, "registry_key", None))
    if descriptor is None and not state_machine_declaration:
        prototype = next((item for item in getattr(document_ast, "declarations", ())
                          if getattr(item, "name", None) == name
                          and getattr(item, "prototype", False)), None)
        descriptor = getattr(prototype, "registry_key", None)
    if descriptor:
        summary = f"UEF-PL `{descriptor}`"
        binding = catalogue.dsl_descriptors.get(descriptor) if catalogue else None
        contract_description = None
        if binding:
            summary += f"  ·  UETD `{binding.module_id}`"
            if binding.contract:
                summary += f"  ·  generation {binding.contract.generation.get('status', 'unspecified')}"
                if binding.contract.description:
                    contract_description = re.sub(r"\s+", " ", binding.contract.description).strip()
                    if len(contract_description) > 180:
                        contract_description = contract_description[:177].rstrip() + "…"
            else:
                summary += "  ·  authoring pending"
        lines.append(summary)
        if contract_description:
            lines.append(f"\n{contract_description}")
    else:
        binding = None
    signature_type = getattr(declaration, "signature_type", None) or getattr(interface, "signature_type", None)
    if signature_type:
        lines.append(f"Signature `{signature_type}`")
    role_order = ("input", "output", "parameter", "state", "local")
    bindings = list(getattr(interface, "bindings", ()))
    for role in role_order:
        selected = [item for item in bindings if getattr(item, "role", None) == role]
        contract_ports = ()
        if not selected and binding is not None and binding.contract is not None:
            contract_ports = binding.contract.inputs if role == "input" else binding.contract.outputs if role == "output" else ()
        if selected or contract_ports:
            label = {"input": "Inputs", "output": "Outputs", "parameter": "Parameters", "state": "State", "local": "Locals"}[role]
            lines.append(f"\n**{label}**")
            if selected:
                lines.extend(f"- `{item.name}` : `{_hover_type_name(getattr(item, 'type_spec', None))}`" for item in selected[:6])
                if len(selected) > 6:
                    lines.append(f"- {len(selected) - 6} more")
            else:
                lines.extend(f"- `{item.get('name', '<unnamed>')}` : `{item.get('type', 'unspecified')}`"
                             for item in contract_ports[:6])
                if len(contract_ports) > 6:
                    lines.append(f"- {len(contract_ports) - 6} more")
    contract_properties = binding.contract.properties if binding and binding.contract else ()
    if contract_properties:
        lines.append("\n**Available settings**")
        lines.extend(f"- `{item.get('name', '<unnamed>')}` : `{item.get('type', 'value')}`"
                     for item in contract_properties[:5])
        if len(contract_properties) > 5:
            lines.append(f"- {len(contract_properties) - 5} more")
    contract_operations = binding.contract.operations if binding and binding.contract else ()
    if contract_operations:
        lines.append("\n**Component operations**")
        for operation in contract_operations[:4]:
            arguments = ", ".join(f"`{argument.get('id', '<arg>')}` : `{argument.get('value_type', 'value')}`"
                                   for argument in operation.get("arguments", ())[:5])
            description = str(operation.get("description", ""))
            if len(description) > 150:
                description = description[:147].rstrip() + "…"
            suffix = f" — {description}" if description else ""
            lines.append(f"- `{operation.get('id', '<operation>')}({arguments})`{suffix}")
        if len(contract_operations) > 4:
            lines.append(f"- {len(contract_operations) - 4} additional operations")
    if descriptor and binding is not None and binding.contract is None:
        lines.append("\nUETD-defined settings and generation are pending an authoring contract.")
    authored_properties = [prop for prop in getattr(interface, "properties", ())
                           if not (getattr(interface, "kind", None) == "state_machine" and prop.name == "initial")]
    if authored_properties:
        lines.append("\n**Configured values**")
        for prop in authored_properties[:5]:
            bounds = _span_offsets(getattr(prop, "span", None))
            authored = source[bounds[0]:bounds[1]].strip() if bounds and source else getattr(prop, "name", "")
            if authored:
                if len(authored) > 140:
                    authored = authored[:137].rstrip() + "…"
                lines.append(f"- `{authored}`")
        if len(authored_properties) > 5:
            lines.append(f"- {len(authored_properties) - 5} more values")
    stages = list(getattr(interface, "stages", ()))
    if stages:
        lines.append("\n**Implementation stages**")
        for item in stages[:5]:
            parameters = ", ".join(f"`{binding.name}` : `{_hover_type_name(binding.type_spec)}`"
                                    for binding in getattr(item, "parameters", ()))
            body = getattr(item, "body", ())
            lines.append(f"- `{item.name}({parameters})` — {len(body)} top-level statement(s)")
        if len(stages) > 5:
            lines.append(f"- {len(stages) - 5} more stages")
    if getattr(interface, "kind", None) == "state_machine":
        lines.extend(["", *_fsm_hover_lines(interface, source)])
    if getattr(interface, "kind", None) in {"operatingsystem", "taskset"}:
        children = list(getattr(interface, "children", ()))
        if children:
            lines.append("\n**Runtime members**")
            for child in children[:8]:
                child_name = getattr(child, "name", None) or "<unnamed>"
                child_kind = str(getattr(child, "kind", "member")).replace("_", " ").title()
                if getattr(child, "kind", None) == "state_machine":
                    machine_children = list(getattr(child, "children", ()))
                    state_count = sum(getattr(item, "kind", None) == "machine_state" for item in machine_children)
                    transition_count = sum(getattr(item, "kind", None) == "transition" for item in machine_children)
                    initial = next((prop.value for prop in getattr(child, "properties", ()) if prop.name == "initial"), None)
                    initial_text = f"; initial `{_hover_value_text(initial, source)}`" if initial is not None else ""
                    lines.append(f"- State machine `{child_name}` : `{child.registry_key}` — {state_count} state(s), {transition_count} transition(s){initial_text}")
                    continue
                details = []
                for prop in getattr(child, "properties", ()):
                    if prop.name in {"run", "rate", "priority", "deadline", "handler", "payload", "capacity", "stack"}:
                        details.append(f"{prop.name} = {_hover_value_text(prop.value, source, limit=72)}")
                suffix = f" — {'; '.join(details[:3])}" if details else ""
                lines.append(f"- {child_kind} `{child_name}`{suffix}")
                for nested in getattr(child, "children", ())[:4]:
                    nested_name = getattr(nested, "name", None) or "<unnamed>"
                    nested_kind = str(getattr(nested, "kind", "member")).replace("_", " ").title()
                    nested_details = []
                    for prop in getattr(nested, "properties", ()):
                        if prop.name in {"run", "rate", "priority", "deadline", "handler", "capacity", "stack"}:
                            nested_details.append(f"{prop.name} = {_hover_value_text(prop.value, source, limit=60)}")
                    nested_suffix = f" — {'; '.join(nested_details[:3])}" if nested_details else ""
                    lines.append(f"  - {nested_kind} `{nested_name}`{nested_suffix}")
                if len(getattr(child, "children", ())) > 4:
                    lines.append(f"  - {len(child.children) - 4} more members")
            if len(children) > 8:
                lines.append(f"- {len(children) - 8} more runtime members")
    if getattr(interface, "kind", None) in {"subsystem", "hybrid_algorithm"}:
        aliases = [child for child in getattr(interface, "children", ()) if getattr(child, "kind", None) == "use"]
        if aliases:
            lines.append("\n**Declared members**")
            lines.extend(f"- `{item.name}` uses `{item.registry_key}`" for item in aliases[:6])
        composition_stages = list(getattr(interface, "composition_stages", ()))
        if composition_stages:
            lines.append("\n**Execution order**")
            for index, stage in enumerate(composition_stages[:6], 1):
                args = ", ".join(f"{key} = {_hover_value_text(value, source, getattr(value, 'span', None), 64)}"
                                  for key, value in getattr(stage, "arguments", ()))
                suffix = f"({args})" if args else "()"
                lines.append(f"{index}. `{stage.name}` calls `{stage.member}{suffix}`")
            if len(composition_stages) > 6:
                lines.append(f"{len(composition_stages) - 6} more stages")
        outputs = list(getattr(interface, "composition_outputs", ()))
        if outputs:
            lines.append("\n**Output mappings**")
            lines.extend(f"- `{item.name}` = `{_hover_value_text(item.expression, source, limit=100)}`" for item in outputs[:5])
            if len(outputs) > 5:
                lines.append(f"- {len(outputs) - 5} more outputs")
    if getattr(interface, "kind", None) == "model":
        form = getattr(interface, "model_form", None) or getattr(interface, "signature_type", None)
        if form:
            lines.append(f"\nDynamics form  `{form}`")
        equation_nodes = list(getattr(interface, "equations", ()))
        if equation_nodes:
            lines.append(f"\n**Dynamics equations**  {len(equation_nodes)} statement(s)")
            for equation in equation_nodes[:3]:
                excerpt = _hover_source_excerpt(source, getattr(equation, "span", None), 130)
                if excerpt:
                    lines.append(f"- `{excerpt}`")
            if len(equation_nodes) > 3:
                lines.append(f"- {len(equation_nodes) - 3} more equations")
        matrix_blocks = [child for child in getattr(interface, "children", ()) if getattr(child, "kind", None) == "matrix_block"]
        if matrix_blocks:
            lines.append("\n**Matrix blocks**")
            for child in matrix_blocks[:4]:
                lines.append(f"- `{child.kind}`")
                for prop in getattr(child, "properties", ())[:4]:
                    lines.append(f"  - `{prop.name}` = `{_hover_value_text(prop.value, source, limit=100)}`")
    if getattr(declaration, "prototype", False) and _project_implementation_for(document_ast, name) is not None:
        lines.append("Implementation available in this project.")
    return "\n".join(lines)


def _project_hover_content(document_ast: object, name: str, source: str, catalogue: AuthoringCatalogue | None,
                          symbol_span: object | None = None) -> str | None:
    """Describe project-owned declarations before falling back to registry names."""
    target_bounds = _span_offsets(symbol_span)
    def selected(node: object) -> bool:
        return target_bounds is None or _span_offsets(_node_span(node)) == target_bounds
    if getattr(document_ast, "name", None) == name and target_bounds == _span_offsets(getattr(document_ast, "span", None)):
        target = next((item for item in getattr(document_ast, "declarations", ()) if getattr(item, "kind", None) == "target"), None)
        target_value = next((prop.value for prop in getattr(target, "properties", ()) if prop.name == "value"), None) if target else None
        runtime = next((item for item in getattr(document_ast, "declarations", ()) if getattr(item, "kind", None) == "operatingsystem"), None)
        components = [item for item in getattr(document_ast, "declarations", ())
                      if getattr(item, "name", None) and getattr(item, "kind", None) not in {"implementation_block", "operatingsystem"}]
        lines = [f"**UETD project**  `{name}`"]
        if target_value is not None:
            lines.append(f"Target  `{_hover_value_text(target_value, source)}`")
        if runtime is not None:
            lines.append(f"Runtime  `{runtime.name}`")
        if components:
            lines.append("\n**Declared components and types**")
            lines.extend(f"- {str(item.kind).replace('_', ' ').title()} `{item.name}`" for item in components[:6])
            if len(components) > 6:
                lines.append(f"- {len(components) - 6} more declarations")
        return "\n".join(lines)
    for declaration in getattr(document_ast, "declarations", ()):
        if declaration.name != name or not selected(declaration):
            continue
        return _declaration_hover_content(document_ast, declaration, name, catalogue, source)
    for binding in getattr(document_ast, "parameters", ()):
        if binding.name == name and selected(binding):
            result = f"**`{name}`** : `{_hover_type_name(binding.type_spec)}`\n\nProject parameter"
            if binding.value is not None:
                result += f"\nDefault  `{_hover_value_text(binding.value, source, binding.span)}`"
            return result
    for collection_name, label in (("records", "Record"), ("enums", "Enum"), ("variants", "Variant"), ("flags", "Flags"), ("fixed_point_types", "Fixed-point type")):
        for item in getattr(document_ast, collection_name, ()):
            if item.name == name and selected(item):
                return _type_hover_content(item, label, source)
            for field in getattr(item, "fields", ()):
                if getattr(field, "name", None) == name and selected(field):
                    return (f"**`{item.name}.{name}`** : `{_hover_type_name(field.type_spec)}`"
                            f"\n\nField of `{item.name}`")
            for member in (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ())):
                if getattr(member, "name", None) == name and selected(member):
                    owner_type = item.name
                    value = getattr(member, "value", getattr(member, "tag", getattr(member, "bit", None)))
                    member_kind = "enum case" if hasattr(member, "value") else "flags member" if hasattr(member, "bit") else "variant case"
                    detail = f"  |  {member_kind} of `{owner_type}`"
                    if value is not None:
                        detail += f"  |  value `{value}`" if hasattr(member, "value") else f"  |  bit `{value}`" if hasattr(member, "bit") else f"  |  tag `{value}`"
                    fields_ = getattr(member, "fields", ())
                    if fields_:
                        detail += "\n\nPayload  " + ", ".join(f"`{field.name}` : `{_hover_type_name(field.type_spec)}`" for field in fields_[:8])
                    return f"**`{name}`**{detail}"
    for item, ancestors in _ast_nodes_with_ancestors(document_ast):
        if getattr(item, "name", None) != name or not selected(item):
            continue
        type_spec = getattr(item, "type_spec", None)
        if type_spec is not None:
            detail = _hover_type_name(type_spec)
            role = str(getattr(item, "role", "value")).replace("_", " ")
            owner = next((ancestor for ancestor in reversed(ancestors)
                          if type(ancestor).__name__ == "Declaration"), None)
            context = f" in `{getattr(owner, 'name', '')}`" if owner is not None and getattr(owner, "name", None) else ""
            return f"**`{name}`** : `{detail}`  \n{role.title()}{context}"
        if type(item).__name__ == "EnumVariant":
            owner = next((node for node in reversed(ancestors) if type(node).__name__ == "EnumDefinition"), None)
            value = getattr(item, "value", None)
            return f"**`{name}`**  |  enum member of `{getattr(owner, 'name', 'unknown enum')}`" + (f"  |  value `{value}`" if value is not None else "")
        if type(item).__name__ == "VariantCaseDefinition":
            owner = next((node for node in reversed(ancestors) if type(node).__name__ == "VariantDefinition"), None)
            fields_ = getattr(item, "fields", ())
            members = "\n".join(f"- `{field.name}` : `{_hover_type_name(field.type_spec)}`" for field in fields_[:8])
            return f"**`{name}`**  |  variant case of `{getattr(owner, 'name', 'unknown variant')}`" + (f"\n\n{members}" if members else "")
        if type(item).__name__ == "FlagsMember":
            owner = next((node for node in reversed(ancestors) if type(node).__name__ == "FlagsDefinition"), None)
            return f"**`{name}`**  |  flags member of `{getattr(owner, 'name', 'unknown flags')}`  |  bit `{getattr(item, 'bit', 'unspecified')}`"
        if type(item).__name__ == "CompositionStage":
            arguments = ", ".join(f"`{key} = {_hover_value_text(value, source, getattr(value, 'span', None), 80)}`"
                                   for key, value in getattr(item, "arguments", ())) or "none"
            owner = next((ancestor for ancestor in reversed(ancestors)
                          if type(ancestor).__name__ == "Declaration"), None)
            text = f"**`{name}`**  |  composition stage"
            if owner is not None:
                text += f" in `{getattr(owner, 'name', '<composition>')}`"
            text += f"\n\nCalls `{getattr(item, 'member', 'unspecified')}`  |  bindings {arguments}"
            if getattr(item, "guard", None) is not None:
                text += f"\nGuard  `{_hover_value_text(item.guard, source)}`"
            if getattr(item, "on_failure", None) is not None:
                text += f"\nFailure action  `{_hover_value_text(item.on_failure, source)}`"
            return text
        if type(item).__name__ == "CompositionLocal":
            type_spec = getattr(item, "type_spec", None)
            return f"**`{name}`** : `{_hover_type_name(type_spec)}`\n\nComposition local"
        if type(item).__name__ == "CompositionOutput":
            start = max(0, item.span.start.offset)
            end = min(len(source), item.span.end.offset)
            authored = source[start:end].strip()
            return f"**`{name}`**  |  composition output\n\n`{authored}`"
        if type(item).__name__ == "AlgorithmStage":
            parameters = ", ".join(f"`{binding.name}` : `{_hover_type_name(binding.type_spec)}`" for binding in getattr(item, "parameters", ()))
            statement_count = len(getattr(item, "body", ()))
            return (f"**`{name}`**  |  implementation stage\n\nParameters  {parameters or 'none'}"
                    f"\nBody  {statement_count} bounded statement(s)")
        if type(item).__name__ == "Declaration":
            return _declaration_hover_content(document_ast, item, name, catalogue, source)
        if getattr(item, "kind", None) == "use":
            return f"**`{name}`**  |  subsystem member alias\n\nUses `{getattr(item, 'registry_key', 'unspecified')}`."
        value = getattr(item, "value", None)
        span = getattr(item, "span", None)
        if value is not None and span is not None:
            start = max(0, span.start.offset)
            end = min(len(source), span.end.offset)
            authored = source[start:end].strip()
            if type(item).__name__ == "Property":
                descriptor = _property_descriptor_at(document_ast, source, name, span)
                module = catalogue.dsl_descriptors.get(descriptor) if descriptor and catalogue else None
                contract_property = next((entry for entry in module.contract.properties if entry.get("name") == name), None) if module and module.contract else None
                if contract_property is not None:
                    lines = [f"**UETD property**  `{name}` : `{contract_property.get('type', 'value')}`", f"\nConfigured as `{authored}`"]
                    if contract_property.get("description"):
                        lines.append(str(contract_property["description"]))
                    return "\n\n".join(lines)
            return f"**`{name}`**  |  configured property\n\n`{authored}`"
    for item in _walk_nodes(document_ast):
        if getattr(item, "variable", None) == name and hasattr(item, "stop_exclusive") and selected(item):
            return f"**`{name}`** : `i32`\n\nLoop variable | statically bounded iteration"
        if type(item).__name__ == "MatchArm" and selected(item):
            if getattr(item, "binding_name", None) == name or any(binding_name == name for _field_name, binding_name in getattr(item, "field_bindings", ())):
                return f"**`{name}`**\n\nPattern binding | local to this match arm"
    return None

def _system_hover_content(system: object, name: str, symbol_span: object | None = None,
                         source: str = "") -> str | None:
    """Describe system declarations, wire layouts, data contracts, and link policy."""
    target_bounds = _span_offsets(symbol_span)
    def card(signature: str, details: str) -> str:
        return f"```c\n{signature}\n```\n\n{details}"

    def selected(span: object | None) -> bool:
        return target_bounds is None or _span_offsets(span) == target_bounds
    def properties(title: str, values: object, limit: int = 6) -> list[str]:
        if not isinstance(values, dict) or not values:
            return []
        result = [f"\n**{title}**"]
        for key, value in list(values.items())[:limit]:
            result.append(f"- `{key}` = `{_hover_value_text(value, source)}`")
        if len(values) > limit:
            result.append(f"- {len(values) - limit} additional properties")
        return result
    if getattr(system, "name", None) == name and selected(getattr(system, "source_span", None)):
        lines = ["**UEF-PL system**", "\nMulti-target system contract."]
        for collection, label in (("nodes", "Projects"), ("external_endpoints", "External endpoints"),
                                  ("links", "Links"), ("protocols", "Protocols")):
            items = getattr(system, collection, ())
            if items:
                lines.append(f"\n**{label}**  " + ", ".join(f"`{item.name}`" for item in items[:6]))
                if len(items) > 6:
                    lines.append(f"{len(items) - 6} more {label.lower()}")
        type_count = sum(len(getattr(system, collection, ())) for collection in
                         ("records", "enums", "variants", "flags_types", "fixed_point_types"))
        if type_count:
            lines.append(f"\nShared data types  `{type_count}`")
        return card(f"struct {name};", "\n".join(lines))
    for node in getattr(system, "nodes", ()):
        if node.name == name and selected(node.source_span):
            return card(f"struct {name};",
                        f"**System node**\n\nRole  `{node.role}`\nProject  `{node.project_path}`")
    for endpoint in getattr(system, "external_endpoints", ()):
        if endpoint.name == name and selected(endpoint.source_span):
            lines = [f"**External endpoint**  `{name}`", f"\nRole  `{endpoint.role}`",
                     f"Implementation  `{endpoint.implementation}`"]
            if endpoint.attached_to:
                lines.append(f"Attached to  `{endpoint.attached_to}`")
            lines.extend(properties("Endpoint contract", endpoint.properties))
            return card(f"struct {name};", "\n".join(lines))
    for link in getattr(system, "links", ()):
        if link.name == name and selected(link.source_span):
            lines = [f"**System link**  `{name}`",
                     f"\nDirection  `{link.source_endpoint}` → `{link.destination_endpoint}`",
                     f"Protocol  `{link.protocol_name or 'unspecified'}`",
                     f"Contract  `{link.contract_name or 'unspecified'}`"]
            lines.extend(properties("Delivery and recovery policy", link.properties))
            return card(f"struct {name};", "\n".join(lines))
    for protocol in getattr(system, "protocols", ()):
        if protocol.name == name and selected(protocol.source_span):
            lines = [f"**Custom wire protocol**  `{name}`", f"\nTransport  `{protocol.transport or 'unspecified'}`"]
            associated_links = [link for link in getattr(system, "links", ()) if link.protocol_name == protocol.name]
            if protocol.transport is None and associated_links:
                lines.append("\n**Bound by system links**")
                for link in associated_links[:6]:
                    transport = link.properties.get("transport", "unspecified")
                    lines.append(f"- `{link.name}` uses `{_hover_value_text(transport, source)}`")
            for label, values in (("Framing", protocol.framing), ("Header layout", protocol.header),
                                  ("Sequence", protocol.sequence), ("Checksum", protocol.checksum),
                                  ("Receiver behavior", protocol.receiver)):
                lines.extend(properties(label, values, 6))
            lines.extend(properties("Protocol settings", protocol.properties, 6))
            messages = list(protocol.messages)
            if messages:
                lines.append("\n**Messages**")
                for message in messages[:6]:
                    message_name = message.get("name", "<message>")
                    message_id = message.get("id")
                    logical_type = message.get("properties", {}).get("logical_type")
                    header = f"- `{message_name}`" + (f" (ID `{message_id}`)" if message_id is not None else "")
                    if logical_type is not None:
                        header += f" : `{_hover_value_text(logical_type, source)}`"
                    direct_fields = message.get("fields", ())
                    fragments = message.get("fragments", ())
                    field_names = [field.get("name", "<field>") for field in direct_fields]
                    field_names.extend(field.get("name", "<field>") for fragment in fragments
                                       for field in fragment.get("fields", ()))
                    if field_names:
                        header += f"  |  {len(field_names)} field(s): " + ", ".join(f"`{field}`" for field in field_names[:6])
                        if len(field_names) > 6:
                            header += f", {len(field_names) - 6} more"
                    if fragments:
                        header += f"  |  {len(fragments)} fragment(s)"
                    lines.append(header)
                if len(messages) > 6:
                    lines.append(f"- {len(messages) - 6} more messages")
            return card(f"struct {name};", "\n".join(lines))
        for message in protocol.messages:
            if message.get("name") == name and selected(message.get("source_span")):
                fields_ = message.get("fields", ())
                message_id = message.get("id")
                logical_type = message.get("properties", {}).get("logical_type")
                lines = ["**Protocol message**"]
                if message_id is not None:
                    lines.append(f"ID  `{message_id}`")
                if logical_type is not None:
                    lines.append(f"Logical payload  `{_hover_value_text(logical_type, source)}`")
                if fields_:
                    lines.append("\n**Fields**")
                    for field in fields_[:8]:
                        field_type = _hover_type_name(field.get("type"))
                        props = field.get("properties", {})
                        layout = ", ".join(f"{key}={_hover_value_text(value, source)}" for key, value in list(props.items())[:3])
                        lines.append(f"- `{field.get('name', '<field>')}` : `{field_type}`" + (f" — {layout}" if layout else ""))
                    if len(fields_) > 8:
                        lines.append(f"- {len(fields_) - 8} more fields")
                if message.get("fragments"):
                    lines.append(f"\nFragmented payload  `{len(message['fragments'])}` fragments")
                lines.extend(properties("Message properties", message.get("properties", {}), 6))
                return card(f"struct {name};", "\n".join(lines))
            for field in message.get("fields", ()):
                if field.get("name") == name and selected(field.get("source_span")):
                    type_name = _hover_type_name(field.get("type"))
                    lines = ["**Protocol field**"]
                    lines.extend(properties("Wire layout", field.get("properties", {}), 6))
                    return card(f"{type_name} {name};", "\n".join(lines))
            for fragment in message.get("fragments", ()):
                if selected(fragment.get("source_span")):
                    fields_ = fragment.get("fields", ())
                    fragment_index = fragment.get('index', '<unnamed>')
                    lines = ["**Protocol fragment**",
                             f"\nMessage  `{message.get('name', '<message>')}`"]
                    lines.extend(f"- `{field.get('name', '<field>')}` : `{_hover_type_name(field.get('type'))}`"
                                 for field in fields_[:8])
                    if len(fields_) > 8:
                        lines.append(f"- {len(fields_) - 8} more fields")
                    lines.extend(properties("Fragment properties", fragment.get("properties", {}), 6))
                    return card(f"struct fragment_{fragment_index};", "\n".join(lines))
                for field in fragment.get("fields", ()):
                    if field.get("name") == name and selected(field.get("source_span")):
                        field_type = _hover_type_name(field.get('type'))
                        lines = ["**Protocol field**",
                                 f"\nFragment  `{fragment.get('index', '<unnamed>')}` of `{message.get('name', '<message>')}`"]
                        lines.extend(properties("Wire layout", field.get("properties", {}), 6))
                        return card(f"{field_type} {name};", "\n".join(lines))
    for item in (*getattr(system, "records", ()), *getattr(system, "enums", ()), *getattr(system, "variants", ()), *getattr(system, "flags_types", ()), *getattr(system, "fixed_point_types", ()), *getattr(system, "hybrid_systems", ())):
        if item.name == name and selected(_node_span(item)):
            if type(item).__name__ == "HybridSystemSpec":
                lines = ["**Hybrid system**"]
                lines.extend(properties("Cross-target timing and coupling", item.properties))
                return card(f"struct {name};", "\n".join(lines))
            labels = {"SystemRecordSpec": "Shared record", "EnumDefinition": "Enum",
                      "VariantDefinition": "Variant", "FlagsDefinition": "Flags type",
                      "FixedPointDefinition": "Fixed-point type"}
            signature_kind = "enum" if type(item).__name__ in {"EnumDefinition", "FlagsDefinition"} else "struct"
            return _type_hover_content(item, labels.get(type(item).__name__, "System type"), source,
                                       signature=f"{signature_kind} {name};")
        for field in getattr(item, "fields", ()):
            field_name = field[0] if isinstance(field, tuple) else getattr(field, "name", None)
            field_span = field[2].get("source_span") if isinstance(field, tuple) and len(field) > 2 else _node_span(field)
            if field_name == name and selected(field_span):
                type_spec = field[1] if isinstance(field, tuple) and len(field) > 1 else getattr(field, "type_spec", None)
                return card(f"{_hover_type_name(type_spec)} {name};",
                            f"**Field of shared type `{item.name}`**")
        for case in (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ())):
            if getattr(case, "name", None) == name and selected(_node_span(case)):
                value = getattr(case, "value", getattr(case, "tag", getattr(case, "bit", None)))
                member_kind = "enum case" if hasattr(case, "value") else "flags member" if hasattr(case, "bit") else "variant case"
                detail = f"**{member_kind.title()} of `{item.name}`**"
                if value is not None:
                    detail += f"  |  value `{value}`"
                fields_ = getattr(case, "fields", ())
                if fields_:
                    detail += "\n\nPayload  " + ", ".join(f"`{field.name}` : `{_hover_type_name(field.type_spec)}`" for field in fields_[:8])
                signature = f"enum {item.name} {{ {name} }};" if member_kind in {"enum case", "flags member"} else f"struct {item.name}_{name};"
                return card(signature, detail)
    for policy in getattr(system, "delivery_policies", ()):
        if policy.get("name") == name and selected(policy.get("source_span")):
            return card(f"struct {name};",
                        "\n".join(["**Delivery policy**", *properties("Policy", policy.get("properties", {}))]))
    return None


@lru_cache(maxsize=1)
def _system_keyword_reference() -> dict[str, tuple[str, str]]:
    """Index grammar keywords to their production names and authored syntax."""
    path = Path(__file__).parents[1] / "system" / "registry.json"
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}
    result: dict[str, tuple[str, str]] = {}
    for production, syntax in data.get("grammar", {}).get("productions", {}).items():
        if not isinstance(syntax, str):
            continue
        for keyword in re.findall(r"'([a-z][a-z0-9_]*)'", syntax):
            result.setdefault(keyword, (production, syntax))
    return result


def _system_keyword_hover_content(name: str) -> str | None:
    """Offer concise grammar-backed hover help for `.uefs` keywords."""
    reference = _system_keyword_reference().get(name)
    if reference is None:
        return None
    production, syntax = reference
    return f"**UEF-PL keyword**  `{name}`\n\nGrammar rule `{production}`\n\nSyntax  `{syntax}`"


def _project_keyword_hover_content(name: str) -> str | None:
    """Describe project-language keywords from the authored language registry."""
    try:
        registry = _language_registry()
    except (OSError, ValueError):
        return None
    construct = registry.constructs.get(name)
    if construct is not None:
        return (
            f"**UEF-PL construct**  `{name}`  |  {construct.category.replace('_', ' ')}\n\n"
            f"{construct.description}\n\n"
            f"Syntax  `{construct.syntax}`\n"
            f"Parser `{construct.parse_rule}`  |  lowering `{construct.lowering_rule}`"
        )
    builtin = registry.builtins.get(name)
    if builtin is not None:
        return (
            f"**UEF-PL builtin**  `{name}`\n\n"
            f"{builtin.description}\n\n"
            f"Syntax  `{builtin.syntax}`"
        )
    # Operators and flow words do not all have a standalone construct entry.
    # Find the shortest grammar production that actually contains the word.
    productions = registry.grammar.get("productions", {})
    candidates = [
        (production, syntax)
        for production, syntax in productions.items()
        if isinstance(syntax, str) and re.search(rf"(?<![A-Za-z0-9_])'{re.escape(name)}'(?![A-Za-z0-9_])", syntax)
    ]
    if not candidates:
        return None
    production, syntax = min(candidates, key=lambda item: (len(item[1]), item[0]))
    return f"**UEF-PL keyword**  `{name}`\n\nGrammar rule `{production}`\n\nSyntax  `{syntax}`"

def _registered_type_hover_content(name: str, catalogue: AuthoringCatalogue | None) -> str | None:
    """Describe a registered type independently of descriptor-name collisions."""
    if catalogue is None:
        return None
    definitions = [item for values in catalogue.types.values() for item in values
                   if name in {item.dsl_name, item.c_name}]
    if len(definitions) != 1:
        return None
    item = definitions[0]
    display_name = item.dsl_name or item.c_name
    if item.language_contract == "fixed_numeric_vector":
        members = ", ".join(f"`{part}`" for part in _registered_vector_component_names(item.declaration))
        detail = f"**Numeric vector type**  `{display_name}`\n\n{item.vector_dimension}-component value"
        if members:
            detail += f"  |  components {members}"
        detail += f"\n\nUETD C type `{item.c_name}`  |  `{item.header}`"
    elif item.language_contract == "enum":
        members = ", ".join(f"`{part}`" for part in item.enum_members[:10])
        detail = f"**Enum type**  `{display_name}`\n\nUETD C type `{item.c_name}`"
        if members:
            detail += f"\n\nMembers  {members}"
    else:
        detail = f"**UETD type**  `{display_name}`\n\nC type `{item.c_name}`  |  header `{item.header}`"
        detail += f"\nKind  `{item.kind}`  |  source status `{item.source_status}`"
        detail += f"\nLanguage visibility  `{item.language_visibility}`"
    if item.declaration:
        detail += f"\n\nC declaration  `{item.declaration}`"
    return detail


def _registry_hover_content(name: str, catalogue: AuthoringCatalogue | None, source_path: Path | None) -> str | None:
    """Resolve language-owned and UETD-owned hover documentation without AST context."""
    if catalogue is not None:
        binding = catalogue.dsl_descriptors.get(name)
        if binding:
            contract = binding.contract
            status = contract.generation.get("status", "unspecified") if contract else "authoring contract missing"
            kind = contract.kind if contract else "uncontracted module"
            display_name = contract.display_name if contract else name
            detail = f"**{kind.replace('_', ' ').title()}**  ·  `{name}`"
            if display_name != name:
                detail += f"\n{display_name}"
            if contract and contract.description:
                description = re.sub(r"\s+", " ", contract.description).strip()
                if len(description) > 180:
                    description = description[:177].rstrip() + "…"
                detail += f"\n\n{description}"
            detail += f"\n\nUETD `{binding.module_id}`  ·  generation `{status}`"
            if contract:
                properties = [f"- `{item.get('name', '<unnamed>')}` : `{item.get('type', 'value')}`" for item in contract.properties[:5]]
                ports = [f"- `{item.get('direction', 'port')} {item.get('name', '<unnamed>')}` : `{item.get('type', 'unspecified')}`"
                         for item in (*contract.inputs, *contract.outputs)[:6]]
                if properties:
                    detail += "\n\n**Parameters**\n" + "\n".join(properties)
                    if len(contract.properties) > 5:
                        detail += f"\n- {len(contract.properties) - 5} more parameters"
                if ports:
                    detail += "\n\n**Ports**\n" + "\n".join(ports)
                    if len(contract.inputs) + len(contract.outputs) > 6:
                        detail += f"\n- {len(contract.inputs) + len(contract.outputs) - 6} more ports"
                if contract.operations:
                    operation_lines = []
                    for item in contract.operations[:4]:
                        description = " ".join(str(item.get("description", "")).split())
                        if len(description) > 120:
                            description = description[:117].rstrip() + "…"
                        suffix = f" — {description}" if description else ""
                        operation_lines.append(f"- `{item.get('id', '<operation>')}`{suffix}")
                    detail += "\n\n**Operations**\n" + "\n".join(operation_lines)
                    if len(contract.operations) > 4:
                        detail += f"\n- {len(contract.operations) - 4} more operations"
                scheduling = contract.scheduling
                if scheduling:
                    scheduling_items = [f"{key}: {', '.join(map(str, value)) if isinstance(value, (list, tuple)) else value}"
                                        for key, value in scheduling.items() if value not in (None, [], {}, "")]
                    if scheduling_items:
                        detail += "\n\n**Scheduling**  " + "  ·  ".join(scheduling_items[:3])
                numeric = contract.numerics
                if numeric:
                    numeric_items = [f"{key}: {', '.join(map(str, value)) if isinstance(value, (list, tuple)) else value}"
                                     for key, value in numeric.items() if value not in (None, [], {}, "")]
                    if numeric_items:
                        detail += "\n\n**Numerics**  " + "  ·  ".join(numeric_items[:3])
            else:
                detail += "\n\nUETD-defined settings and generation are pending an authoring contract."
            return detail
        functions = catalogue.math_functions.get(name, ())
        if functions:
            overload_details = []
            for function in functions[:5]:
                signature = function.dsl_signature or f"{name}({function.parameters})"
                state = f"{function.implementation_status} | {function.dsl_binding_status}"
                binding = f"`{function.c_name}` from `{function.header}`"
                if function.module_id:
                    binding += f"  |  UETD `{function.module_id}`"
                operators = f"  |  operators `{', '.join(function.operators)}`" if function.operators else ""
                contexts = f"  |  contexts `{', '.join(function.callable_contexts)}`" if function.callable_contexts else ""
                overload_details.append(f"- `{signature}`\n  `{state}`\n  {binding}{operators}{contexts}")
            if len(functions) > 5:
                overload_details.append(f"- {len(functions) - 5} more overloads")
            return "**UMATH function**  `" + name + "`\n\n" + "\n".join(overload_details)
        type_detail = _registered_type_hover_content(name, catalogue)
        if type_detail is not None:
            return type_detail
        unit = catalogue.units.get(name)
        if unit is not None:
            return f"**UETD unit**  `{name}`\n\n{unit}"
    try:
        registry = _language_registry()
    except (OSError, ValueError):
        return None
    construct = registry.constructs.get(name)
    if construct:
        return (f"**UEF-PL construct**  `{name}`\n\n{construct.description}\n\n"
                f"Syntax  `{construct.syntax}`\n"
                f"Parser `{construct.parse_rule}`  |  lowering `{construct.lowering_rule}`")
    builtin = registry.builtins.get(name)
    if builtin:
        return (f"**UEF-PL builtin**  `{name}`\n\n{builtin.description}\n\n"
                f"Syntax `{builtin.syntax}`  |  field policy `{builtin.field_policy}`")
    return None

def _registered_member_declaration(name: str, receiver: str, catalogue: AuthoringCatalogue | None,
                                   source_path: Path) -> tuple[Path, str, object] | None:
    """Find a UETD member's source file and authored identifier span."""
    if catalogue is None:
        return None
    relative_path: str | None = None
    binding = catalogue.dsl_descriptors.get(receiver)
    if binding and binding.contract:
        declared = {str(item.get("name")) for item in (*binding.contract.properties, *binding.contract.inputs, *binding.contract.outputs)}
        if name in declared:
            relative_path = binding.source_path
    if relative_path is None:
        definition = next((item for values in catalogue.types.values() for item in values
                           if receiver in {item.dsl_name, item.c_name} and name in item.enum_members), None)
        if definition is not None:
            relative_path = definition.header
    if not relative_path:
        return None
    try:
        root = _locate_uetd(source_path)
        target = (root / relative_path).resolve(strict=False)
        if not target.is_relative_to(root) or not target.is_file():
            return None
        text = target.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError, ValueError):
        return None
    match = re.search(rf"(?<![A-Za-z0-9_]){re.escape(name)}(?![A-Za-z0-9_])", text)
    if match is None:
        return None
    span = _span_from_offsets(target, text, match.start(), match.end())
    return target, text, span

def _registered_member_location(name: str, receiver: str, catalogue: AuthoringCatalogue | None,
                                source_path: Path) -> types.Location | None:
    """Navigate a UETD type member or registered component field to its source contract."""
    declaration = _registered_member_declaration(name, receiver, catalogue, source_path)
    if declaration is None:
        return None
    target, text, span = declaration
    return _location_for_span(span, target, source=text)


def _registered_header_symbol_location(c_name: str, relative_path: str, source_path: Path,
                                      *, call_name: bool = False, type_kind: str | None = None) -> types.Location | None:
    """Return an exact UETD header location for a registered C symbol."""
    try:
        root = _locate_uetd(source_path)
        target = (root / relative_path).resolve(strict=False)
        if not target.is_relative_to(root) or not target.is_file():
            return None
        text = target.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError, ValueError):
        return None
    escaped = re.escape(c_name)
    if call_name:
        match = re.search(rf"(?<![A-Za-z0-9_]){escaped}(?![A-Za-z0-9_])\s*(?=\()", text)
    else:
        match = None
        if type_kind == "struct":
            match = re.search(rf"\btypedef\s+struct(?:\s+[A-Za-z_]\w*)?\s*\{{.*?\}}\s*{escaped}\s*;", text, re.DOTALL)
        elif type_kind == "enum":
            match = re.search(rf"\btypedef\s+enum(?:\s+[A-Za-z_]\w*)?\s*\{{.*?\}}\s*{escaped}\s*;", text, re.DOTALL)
        if match is not None:
            occurrences = list(re.finditer(rf"(?<![A-Za-z0-9_]){escaped}(?![A-Za-z0-9_])", text[match.start():match.end()]))
            if occurrences:
                # In a typedef struct/enum the final occurrence is its type
                # name; earlier matches may be self-referential field types.
                symbol = occurrences[-1]
                start = match.start() + symbol.start()
                span = _span_from_offsets(target, text, start, start + len(c_name))
                return _location_for_span(span, target, name=c_name, source=text)
        # Typedef aliases, function-pointer typedefs, and older headers do not
        # share one C grammar shape. Restrict the search to a typedef
        # statement when possible, then fall back to the exact public name.
        if match is None:
            match = re.search(rf"\btypedef\b[^;]*\b{escaped}\s*;", text, re.DOTALL)
        if match is not None:
            symbol_match = re.search(rf"(?<![A-Za-z0-9_]){escaped}(?![A-Za-z0-9_])", text[match.start():match.end()])
            if symbol_match is not None:
                start = match.start() + symbol_match.start()
                span = _span_from_offsets(target, text, start, start + len(c_name))
                return _location_for_span(span, target, name=c_name, source=text)
        match = re.search(rf"(?<![A-Za-z0-9_]){escaped}(?![A-Za-z0-9_])", text)
        if match is None:
            return None
        span = _span_from_offsets(target, text, match.start(), match.end())
        return _location_for_span(span, target, name=c_name, source=text)
    if match is None:
        return None
    span = _span_from_offsets(target, text, match.start(), match.end())
    return _location_for_span(span, target, name=c_name, source=text)


def _registered_type_declaration(name: str, catalogue: AuthoringCatalogue | None,
                                 source_path: Path) -> tuple[Path, str, object] | None:
    """Find the exact public typedef span for a registered DSL type name."""
    if catalogue is None:
        return None
    definitions = [definition for values in catalogue.types.values() for definition in values
                   if name in {definition.dsl_name, definition.c_name}]
    if len(definitions) != 1:
        return None
    definition = definitions[0]
    try:
        root = _locate_uetd(source_path)
        target = (root / definition.header).resolve(strict=False)
        if not target.is_relative_to(root) or not target.is_file():
            return None
        text = target.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError, ValueError):
        return None
    escaped = re.escape(definition.c_name)
    match = None
    if definition.kind == "struct":
        match = re.search(rf"\btypedef\s+struct(?:\s+[A-Za-z_]\w*)?\s*\{{.*?\}}\s*{escaped}\s*;", text, re.DOTALL)
    elif definition.kind == "enum":
        match = re.search(rf"\btypedef\s+enum(?:\s+[A-Za-z_]\w*)?\s*\{{.*?\}}\s*{escaped}\s*;", text, re.DOTALL)
    if match is not None:
        occurrences = list(re.finditer(rf"(?<![A-Za-z0-9_]){escaped}(?![A-Za-z0-9_])", text[match.start():match.end()]))
        selected = occurrences[-1] if occurrences else None
        if selected is not None:
            start = match.start() + selected.start()
            return target, text, _span_from_offsets(target, text, start, start + len(definition.c_name))
    match = re.search(rf"\btypedef\b[^;]*\b{escaped}\s*;", text, re.DOTALL)
    if match is not None:
        selected = re.search(rf"(?<![A-Za-z0-9_]){escaped}(?![A-Za-z0-9_])", text[match.start():match.end()])
        if selected is not None:
            start = match.start() + selected.start()
            return target, text, _span_from_offsets(target, text, start, start + len(definition.c_name))
    selected = re.search(rf"(?<![A-Za-z0-9_]){escaped}(?![A-Za-z0-9_])", text)
    if selected is None:
        return None
    return target, text, _span_from_offsets(target, text, selected.start(), selected.end())

@server.feature(types.TEXT_DOCUMENT_HOVER)
def hover(_ls: LanguageServer, params: types.HoverParams) -> types.Hover | None:
    """Show syntax, UETD registry, project symbol, and system-contract information."""
    document = server.workspace.get_text_document(params.text_document.uri)
    _LOGGER.debug("Hover request uri=%s line=%d character=%d", params.text_document.uri,
                  params.position.line + 1, params.position.character + 1)
    if params.position.line >= len(document.lines):
        _LOGGER.debug("Hover ignored: requested line is outside the document")
        return None
    line = document.lines[params.position.line]
    character = _utf16_to_codepoint(line, params.position.character)
    word = _word_at(line, character)
    if word is None:
        _LOGGER.debug("Hover ignored: cursor is not on an identifier")
        return None
    name, start, end = word
    source_token = _source_token_at(document.source, params.position.line, params.position.character)
    symbol_offset = source_token[1] if source_token is not None else _offset_from_lsp(document.source, params.position)
    source_path = _path_from_uri(params.text_document.uri)
    is_system = bool(source_path and source_path.suffix.casefold() == ".uefs")
    catalogue = _catalogue_for_document(params.text_document.uri)
    if is_system:
        _, system = _parse_editor_ast(source_path, document.source)
        symbol = _local_symbol_at(source_path, document.source, params.position.line, params.position.character)
        content = _system_hover_content(system, name, symbol[1], document.source) if system is not None and symbol else None
    else:
        ast = _parse_partial_source(document.source, source_path)
        symbol = (_local_symbol_at(source_path, document.source, params.position.line, params.position.character)
                  if source_path is not None else None)
        symbol_path = Path(symbol[1].path).resolve(strict=False) if symbol and getattr(symbol[1], "path", None) else source_path
        if symbol is not None and symbol_path is not None and source_path is not None and symbol_path != source_path.resolve(strict=False):
            symbol_source = _source_for_path(symbol_path)
            target_project, target_system = _parse_editor_ast(symbol_path, symbol_source) if symbol_source is not None else (None, None)
            content = (_project_hover_content(target_project, name, symbol_source, catalogue, symbol[1]) if target_project is not None
                       else _system_hover_content(target_system, name, symbol[1], symbol_source) if target_system is not None else None)
            if content is None and ast is not None:
                receiver = _qualified_receiver_before(document.source, symbol_offset)
                if receiver:
                    content = _member_hover_content(ast, catalogue, receiver, name,
                                                    symbol_offset)
        else:
            content = _project_hover_content(ast, name, document.source, catalogue, symbol[1]) if ast is not None and symbol else None
        if content is None and ast is not None:
            receiver = _qualified_receiver_before(document.source, symbol_offset)
            if receiver:
                content = _member_hover_content(ast, catalogue, receiver, name,
                                                symbol_offset)
            if content is None and symbol is None and source_path is not None:
                imported = _imported_contract_declarations(ast, source_path, name)
                if len(imported) == 1:
                    target_path, target_source, target_span, _kind = imported[0]
                    target_system = _parse_editor_ast(target_path, target_source)[1]
                    content = _system_hover_content(target_system, name, target_span, target_source) if target_system is not None else None
        if content is None and is_system:
            content = _system_keyword_hover_content(name)
        if content is None and not is_system:
            content = _project_keyword_hover_content(name)
        if content is None:
            content = _registry_hover_content(name, catalogue, source_path)
        if not is_system and _project_type_spec_at(
            ast, name, _offset_from_lsp(document.source, params.position)
        ):
            # Type spellings can overlap module descriptors (for example
            # `quaternion`). In a TypeSpec, the type meaning wins.
            content = _registered_type_hover_content(name, catalogue) or content
        if content is None and catalogue is not None:
            try:
                symbols = build_project_symbol_table(ast) if ast is not None else None
                hint = hover_language(HintContext("expression", prefix=name, field_name=name, partial_ast=ast, symbols=symbols), _language_registry(), catalogue)
            except (OSError, ValueError):
                hint = None
            if hint is not None:
                content = f"**{hint.detail}**" + (f"\n\n{hint.documentation}" if hint.documentation else "")
    if content is None:
        _LOGGER.debug("Hover unresolved uri=%s symbol=%s line=%d", params.text_document.uri, name, params.position.line + 1)
        return None
    _LOGGER.debug("Hover resolved uri=%s symbol=%s markdown_characters=%d", params.text_document.uri, name, len(content))
    return types.Hover(
        contents=types.MarkupContent(kind=types.MarkupKind.Markdown, value=content),
        range=types.Range(start=types.Position(line=params.position.line, character=_column_to_utf16(line, start + 1)), end=types.Position(line=params.position.line, character=_column_to_utf16(line, end + 1))),
    )


@server.feature("uetd/quickInfo")
def quick_info(_ls: LanguageServer, params: object) -> dict[str, object] | None:
    """Return the semantic parts used by Visual Studio's interactive Quick Info UI.

    Visual Studio's LSP hover surface currently consumes plain text and cannot
    host links or copy actions. The VSIX asks this private request for the same
    language-server facts, then presents them through Visual Studio's editor
    Quick Info API. StreamJsonRpc sends one positional argument, so pygls can
    deliver ``params`` as a one-item list containing a named object rather than
    as a plain dictionary. Accept both forms; other LSP clients continue to use
    ``textDocument/hover``.
    """

    # StreamJsonRpc sends this request's single positional argument as a JSON
    # array. pygls passes the JSON-RPC params value straight to the handler, so
    # unwrap that one argument before reading the request object.
    if isinstance(params, (list, tuple)):
        if len(params) != 1:
            return None
        params = params[0]

    def value(source: object, *names: str) -> object | None:
        if isinstance(source, dict):
            for name in names:
                if name in source:
                    return source[name]
            return None
        for name in names:
            if hasattr(source, name):
                return getattr(source, name)
        return None

    text_document = value(params, "textDocument", "text_document")
    position = value(params, "position")
    # pygls converts JSON objects to namedtuples before dispatching custom
    # methods, while direct callers and tests may pass ordinary dictionaries.
    uri = value(text_document, "uri")
    line = value(position, "line")
    character = value(position, "character")
    if not isinstance(uri, str) or not isinstance(line, int) or not isinstance(character, int):
        return None
    try:
        document = server.workspace.get_text_document(uri)
    except KeyError:
        return None
    if line < 0 or line >= len(document.lines):
        return None
    request_position = types.Position(line=line, character=character)
    name = _word_at(document.lines[line], _utf16_to_codepoint(document.lines[line], character))
    hover_result = (hover(server, types.HoverParams(
        text_document=types.TextDocumentIdentifier(uri=uri), position=request_position
    )) if name is not None else None)
    if hover_result is None:
        content = ""
    else:
        contents = hover_result.contents
        if isinstance(contents, types.MarkupContent):
            content = contents.value
        elif isinstance(contents, str):
            content = contents
        else:
            content = str(contents)

    if name is not None:
        start_character = _column_to_utf16(document.lines[line], name[1] + 1)
        end_character = _column_to_utf16(document.lines[line], name[2] + 1)
        locations = definition(server, types.DefinitionParams(
            text_document=types.TextDocumentIdentifier(uri=uri), position=request_position
        ))
    else:
        start_character = end_character = character
        locations = []
    target = None
    if locations:
        location = locations[0]
        target_range = getattr(location, "range", None) or getattr(location, "target_selection_range", None)
        target = {
            "uri": str(getattr(location, "uri", None) or getattr(location, "target_uri", "")),
            "line": target_range.start.line if target_range else 0,
            "character": target_range.start.character if target_range else 0,
            "endLine": target_range.end.line if target_range else 0,
            "endCharacter": target_range.end.character if target_range else 0,
        }

    diagnostic = None
    cached = _DIAGNOSTICS_BY_URI.get(uri)
    if cached is not None and cached[0] == document.version:
        candidates = []
        for item in cached[1]:
            start = item.range.start
            end = item.range.end
            if name is None:
                overlaps_word = (
                    (start.line < line or (start.line == line and start.character <= character))
                    and (end.line > line or (end.line == line and character < end.character
                                             or end.line == line and start.character == end.character == character))
                )
            else:
                overlaps_word = (start.line < line or (start.line == line and start.character < end_character)) and (
                    end.line > line or (end.line == line and end.character > start_character)
                )
            if overlaps_word:
                candidates.append(item)
        if candidates:
            selected = min(candidates, key=lambda item: (
                item.severity != types.DiagnosticSeverity.Error,
                item.severity != types.DiagnosticSeverity.Warning,
                item.range.end.line - item.range.start.line,
                item.range.end.character - item.range.start.character,
            ))
            diagnostic = {
                "message": selected.message,
                "code": str(getattr(selected.code, "value", selected.code) or ""),
                "severity": int(selected.severity) if selected.severity is not None else 3,
                "line": selected.range.start.line,
                "character": selected.range.start.character,
                "endLine": selected.range.end.line,
                "endCharacter": selected.range.end.character,
            }
    if not content and diagnostic is None:
        return None
    return {
        "name": name[0] if name is not None else "",
        "content": content,
        "definition": target,
        "diagnostic": diagnostic,
        "sourceUri": uri,
    }

@server.feature(types.TEXT_DOCUMENT_DEFINITION)
def definition(_ls: LanguageServer, params: types.DefinitionParams) -> list[types.Location]:
    """Navigate local declarations, system-node files, and UETD contracts."""
    document = server.workspace.get_text_document(params.text_document.uri)
    _LOGGER.debug("Definition request uri=%s line=%d character=%d", params.text_document.uri,
                  params.position.line + 1, params.position.character + 1)
    if params.position.line >= len(document.lines):
        _LOGGER.debug("Definition ignored: requested line is outside the document")
        return []
    line = document.lines[params.position.line]
    word = _word_at(line, _utf16_to_codepoint(line, params.position.character))
    source_path = _path_from_uri(params.text_document.uri)
    if word is None or source_path is None:
        _LOGGER.debug("Definition ignored: cursor is not on an identifier or URI has no source path")
        return []
    name = word[0]
    _LOGGER.debug("Definition resolving symbol=%s source=%s", name, source_path)
    local_symbol = None
    ast = None
    if source_path.suffix.casefold() == ".uef":
        ast = _parse_partial_source(document.source, source_path)
        if ast is not None:
            local_symbol = _local_symbol_at(source_path, document.source, params.position.line, params.position.character)
            if local_symbol and local_symbol[2] in {
                "algorithm", "custom_algorithm", "hybrid_algorithm", "custom_protocol",
                "model", "subsystem", "component",
            }:
                implementation = _project_implementation_for(ast, name)
                if implementation is not None:
                    location = _location_for_span(_node_span(implementation), source_path,
                                                   name=name, source=document.source)
                    if location:
                        _LOGGER.debug("Definition resolved %s to same-file implementation", name)
                        return [location]
            local = (_location_for_span(local_symbol[1], source_path, name=name, source=document.source)
                     if local_symbol and local_symbol[2] != "property" else None)
            if local:
                _LOGGER.debug("Definition resolved %s to same-file declaration", name)
                return [local]
            imported = _imported_contract_declarations(ast, source_path, name)
            if len(imported) == 1:
                target_path, target_source, target_span, _kind = imported[0]
                location = _location_for_span(target_span, target_path, name=name, source=target_source)
                if location:
                    _LOGGER.debug("Definition resolved %s to imported contract %s", name, target_path)
                    return [location]
    elif source_path.suffix.casefold() == ".uefs":
        _, system = _parse_editor_ast(source_path, document.source)
        if system is not None:
            local_symbol = _local_symbol_at(source_path, document.source, params.position.line, params.position.character)
            if local_symbol and local_symbol[2] != "node":
                local = _location_for_span(local_symbol[1], source_path, name=name, source=document.source)
                if local:
                    _LOGGER.debug("Definition resolved %s to same-file system declaration", name)
                    return [local]
            local = _system_declaration_location(system, name, document.source)
            if local:
                _LOGGER.debug("Definition resolved %s to same-file system declaration", name)
                return [local]
    catalogue = _catalogue_for_document(params.text_document.uri)
    if source_path.suffix.casefold() == ".uef" and local_symbol and local_symbol[2] == "property":
        descriptor = _property_descriptor_at(ast, document.source, name, local_symbol[1])
        member_location = _registered_member_location(name, descriptor, catalogue, source_path) if descriptor else None
        if member_location is not None:
            _LOGGER.debug("Definition resolved member %s through registered component contract", name)
            return [member_location]
    member_receiver = _qualified_receiver_before(
        document.source, _offset_from_lsp(document.source, params.position)
    )
    if member_receiver is not None:
        receiver_name = member_receiver.split(".", 1)[0]
        receiver_key = receiver_name
        if source_path.suffix.casefold() == ".uef":
            project_ast = _parse_partial_source(document.source, source_path)
            receiver_decl = next((item for item in _walk_nodes(project_ast) if type(item).__name__ == "Declaration"
                                  and getattr(item, "name", None) == receiver_name and getattr(item, "registry_key", None)), None)
            if receiver_decl is not None:
                receiver_key = receiver_decl.registry_key
        member_location = _registered_member_location(name, receiver_key, catalogue, source_path)
        if member_location is not None:
            _LOGGER.debug("Definition resolved member %s through receiver %s", name, receiver_key)
            return [member_location]
    if catalogue is None:
        _LOGGER.debug("Definition unresolved %s: UETD catalogue unavailable", name)
        return []
    relative_path: str | None = None
    binding = catalogue.dsl_descriptors.get(name)
    if binding is not None:
        relative_path = binding.source_path
    elif name in catalogue.math_functions:
        locations = [location for function in catalogue.math_functions[name]
                     if (location := _registered_header_symbol_location(
                         function.c_name, function.header, source_path, call_name=True
                     )) is not None]
        if locations:
            _LOGGER.debug("Definition resolved UMATH symbol=%s locations=%d", name, len(locations))
            return locations
        try:
            root = _locate_uetd(source_path)
        except (OSError, ValueError):
            return []
        origin = types.Range(start=types.Position(line=0, character=0), end=types.Position(line=0, character=0))
        locations = []
        for relative in sorted({item.header for item in catalogue.math_functions[name]}):
            target = (root / relative).resolve()
            if target.is_relative_to(root) and target.is_file():
                locations.append(types.Location(uri=target.as_uri(), range=origin))
        _LOGGER.debug("Definition resolved UMATH symbol=%s locations=%d", name, len(locations))
        return locations
    else:
        type_definitions = [definition for definitions in catalogue.types.values() for definition in definitions
                            if name in {definition.dsl_name, definition.c_name}]
        if type_definitions:
            locations = [location for definition in type_definitions
                         if (location := _registered_header_symbol_location(
                             definition.c_name, definition.header, source_path, type_kind=definition.kind
                         )) is not None]
            if locations:
                _LOGGER.debug("Definition resolved UETD type=%s locations=%d", name, len(locations))
                return locations
            relative_path = type_definitions[0].header
        elif name in catalogue.units:
            try:
                api = json.loads((_locate_uetd(source_path) / "api.json").read_text(encoding="utf-8"))
                relative_path = api.get("unit_registry")
            except (OSError, ValueError, TypeError):
                relative_path = None
    if not relative_path:
        _LOGGER.warning("Definition unresolved uri=%s symbol=%s line=%d character=%d",
                        params.text_document.uri, name, params.position.line + 1,
                        params.position.character + 1)
        return []
    try:
        root = _locate_uetd(source_path)
        target = (root / relative_path).resolve()
        if not target.is_relative_to(root) or not target.is_file():
            return []
    except (OSError, ValueError):
        _LOGGER.debug("Definition unresolved symbol=%s: could not resolve registered source path", name)
        return []
    origin = types.Range(start=types.Position(line=0, character=0), end=types.Position(line=0, character=0))
    _LOGGER.debug("Definition resolved symbol=%s target=%s", name, target)
    return [types.Location(uri=target.as_uri(), range=origin)]

@server.feature("textDocument/implementation")
def implementation(_ls: LanguageServer, params: types.DefinitionParams) -> list[types.Location]:
    """Navigate component signatures and system nodes to authored implementations."""
    document = server.workspace.get_text_document(params.text_document.uri)
    source_path = _path_from_uri(params.text_document.uri)
    if source_path is None or params.position.line >= len(document.lines):
        return []
    word = _word_at(document.lines[params.position.line],
                    _utf16_to_codepoint(document.lines[params.position.line], params.position.character))
    if word is None:
        return []
    if source_path.suffix.casefold() == ".uef":
        project = _parse_partial_source(document.source, source_path)
        symbol = _local_symbol_at(source_path, document.source, params.position.line, params.position.character)
        if project is None or symbol is None or symbol[2] not in {
            "algorithm", "custom_algorithm", "hybrid_algorithm", "custom_protocol",
            "model", "subsystem", "component", "sensor", "actuator", "protocol",
            "peripheral", "bus", "module",
        }:
            return []
        body = _project_implementation_for(project, symbol[0])
        location = _location_for_span(_node_span(body), source_path, name=symbol[0], source=document.source)
        if location is not None:
            _LOGGER.debug("Implementation resolved %s to %s", symbol[0], location.uri)
            return [location]
    elif source_path.suffix.casefold() == ".uefs":
        system = _parse_editor_ast(source_path, document.source)[1]
        symbol = _local_symbol_at(source_path, document.source, params.position.line, params.position.character)
        if system is not None and symbol is not None and symbol[2] == "node":
            node = next((item for item in getattr(system, "nodes", ()) if item.name == symbol[0]), None)
            if node is not None:
                target_path = (source_path.parent / node.project_path).resolve(strict=False)
                if target_path.is_file():
                    origin = types.Range(start=types.Position(line=0, character=0),
                                         end=types.Position(line=0, character=0))
                    return [types.Location(uri=target_path.as_uri(), range=origin)]
    return []

@server.feature(types.TEXT_DOCUMENT_SIGNATURE_HELP)
def signature_help(_ls: LanguageServer, params: types.SignatureHelpParams) -> types.SignatureHelp | None:
    """Show UEF-PL builtin or UMATH signature and the active argument."""
    document = server.workspace.get_text_document(params.text_document.uri)
    if params.position.line >= len(document.lines):
        return None
    source_path = _path_from_uri(params.text_document.uri)
    line = document.lines[params.position.line]
    cursor_offset = _offset_from_lsp(document.source, params.position)
    tokens, _ = Lexer(document.source, source_path).tokenize()
    frames: list[list[object]] = []
    previous = None
    for token in tokens:
        if token.span.start.offset >= cursor_offset:
            break
        if token.kind.value == "(":
            frames.append([previous.text if previous and previous.kind.value == "identifier" else None, 0])
        elif token.kind.value == "," and frames:
            frames[-1][1] = int(frames[-1][1]) + 1
        elif token.kind.value == ")" and frames:
            frames.pop()
        previous = token
    if not frames or not frames[-1][0]:
        return None
    catalogue = _catalogue_for_document(params.text_document.uri)
    try:
        language_registry = _language_registry()
    except (OSError, ValueError):
        language_registry = None
    name = str(frames[-1][0])
    active_parameter = int(frames[-1][1])
    functions = catalogue.math_functions.get(name, ()) if catalogue else ()
    builtin = language_registry.builtins.get(name) if language_registry else None
    if functions:
        signatures = []
        for function in functions:
            label = function.dsl_signature or f"{name}({function.parameters})"
            parameter_labels = _signature_parameters(label)
            docs = (f"UMATH `{function.c_name}` | {function.implementation_status} | DSL binding {function.dsl_binding_status}"
                    f"\n\n{function.c_signature}\n\nHeader: `{function.header}`")
            parameters = [types.ParameterInformation(label=item) for item in parameter_labels]
            signatures.append(types.SignatureInformation(
                label=label, documentation=docs, parameters=parameters,
                active_parameter=min(active_parameter, max(0, len(parameters) - 1)),
            ))
        return types.SignatureHelp(signatures=signatures, active_signature=0,
                                   active_parameter=active_parameter)
    elif builtin:
        label = builtin.syntax
        parameter_labels = _signature_parameters(label)
        docs = builtin.description
    else:
        project = _parse_partial_source(document.source, source_path) if source_path and source_path.suffix.casefold() == ".uef" else None
        component = _component_call_signature(project, catalogue, name) if project is not None else None
        if component is None:
            return None
        label, parameter_labels, parameter_docs = component
        docs = "UEF-PL component call\n\n" + "\n".join(parameter_docs)
    parameters = [types.ParameterInformation(label=item) for item in parameter_labels]
    signature = types.SignatureInformation(
        label=label,
        documentation=docs,
        parameters=parameters,
        active_parameter=min(active_parameter, max(0, len(parameters) - 1)),
    )
    return types.SignatureHelp(signatures=[signature], active_signature=0, active_parameter=min(active_parameter, max(0, len(parameters) - 1)))

def _signature_parameters(label: str) -> list[str]:
    """Split a compact language signature into top-level argument labels."""
    opening = label.find("(")
    closing = label.rfind(")")
    if opening < 0 or closing <= opening + 1:
        return []
    content = label[opening + 1:closing]
    result: list[str] = []
    depth = 0
    start = 0
    for index, char in enumerate(content):
        if char in "(<[":
            depth += 1
        elif char in ")>]":
            depth = max(0, depth - 1)
        elif char == "," and depth == 0:
            result.append(content[start:index].strip())
            start = index + 1
    final = content[start:].strip()
    if final:
        result.append(final)
    return result

def _component_call_signature(project: object, catalogue: AuthoringCatalogue | None, name: str, seen: set[str] | None = None):
    """Build signature help for registered, custom, hybrid, and subsystem calls."""
    visited = set() if seen is None else seen
    if name in visited:
        return None
    visited.add(name)
    signatures = [item for item in getattr(project, "declarations", ()) if item.name == name and item.prototype]
    signature = signatures[0] if len(signatures) == 1 else None
    parameters: list[tuple[str, str, str]] = []
    if signature and catalogue and signature.registry_key:
        binding = catalogue.dsl_descriptors.get(signature.registry_key)
        if binding and binding.contract:
            parameters.extend((str(item.get("name")), str(item.get("type", "value")),
                               str(item.get("description", "")))
                              for item in binding.contract.inputs if isinstance(item.get("name"), str))
    implementation = next((item for item in getattr(project, "declarations", ()) if item.kind == "implementation_block"), None)
    bodies = [item for item in getattr(implementation, "children", ()) if item.name == name] if implementation else []
    if not parameters:
        for body in bodies:
            parameters.extend((item.name, item.type_spec.name if item.type_spec else "value", "")
                              for item in body.bindings if item.role == "input")
    if not parameters and signature and signature.kind in {"custom_algorithm", "hybrid_algorithm", "subsystem", "model"}:
        for body in bodies:
            parameters.extend((item.name, item.type_spec.name if item.type_spec else "value", "")
                              for item in body.bindings if item.role == "input")
    if not parameters:
        for declaration in _walk_nodes(project):
            if getattr(declaration, "kind", None) == "use" and getattr(declaration, "name", None) == name:
                return _component_call_signature(project, catalogue, getattr(declaration, "registry_key", ""), visited)
    if not parameters:
        return None
    labels = [f"{parameter}: {type_name}" for parameter, type_name, _ in parameters]
    docs = [f"`{parameter}: {type_name}`" + (f" — {description}" if description else "")
            for parameter, type_name, description in parameters]
    return f"{name}({', '.join(labels)})", labels, docs

def _make_document_symbol(name: str, kind: types.SymbolKind, span: object | None, source: str, path: Path | None,
                          detail: str | None = None, children: list[types.DocumentSymbol] | None = None) -> types.DocumentSymbol | None:
    """Create a protocol symbol with a precise identifier selection range."""
    if span is None:
        return None
    lines = source.splitlines()
    selection = span
    start = max(0, getattr(span.start, "offset", 0))
    end = min(len(source), getattr(span.end, "offset", len(source)))
    name_match = re.search(rf"(?<![A-Za-z0-9_]){re.escape(name)}(?![A-Za-z0-9_])", source[start:end]) if name else None
    if name_match:
        selection = _span_from_offsets(path or getattr(span, "path", None) or Path.cwd(), source,
                                      start + name_match.start(), start + name_match.end())
    return types.DocumentSymbol(name=name, kind=kind, range=_lsp_range(span, lines),
                                selection_range=_lsp_range(selection, lines), detail=detail, children=children or None)

def _declaration_symbols(declaration: object, source: str, path: Path | None) -> types.DocumentSymbol | None:
    """Build a hierarchy for components and implementation blocks."""
    children: list[types.DocumentSymbol] = []
    for child in getattr(declaration, "children", ()):
        symbol = _declaration_symbols(child, source, path)
        if symbol:
            children.append(symbol)
    for binding in getattr(declaration, "bindings", ()):
        symbol = _make_document_symbol(binding.name, types.SymbolKind.Field if binding.role in {"input", "output", "state", "field"} else types.SymbolKind.Variable,
                                       binding.span, source, path, binding.type_spec.name if binding.type_spec else binding.role)
        if symbol:
            children.append(symbol)
    for stage in getattr(declaration, "stages", ()):
        stage_children = [_make_document_symbol(item.name, types.SymbolKind.Variable, item.span, source, path,
                                                item.type_spec.name if item.type_spec else "parameter") for item in stage.parameters]
        stage_children = [item for item in stage_children if item]
        symbol = _make_document_symbol(stage.name, types.SymbolKind.Method, stage.span, source, path, "custom algorithm stage", stage_children)
        if symbol:
            children.append(symbol)
    for stage in getattr(declaration, "composition_stages", ()):
        symbol = _make_document_symbol(stage.name, types.SymbolKind.Method, stage.span, source, path, f"call to {stage.member}")
        if symbol:
            children.append(symbol)
    for local in (*getattr(declaration, "composition_locals", ()), *getattr(declaration, "composition_outputs", ())):
        type_spec = getattr(local, "type_spec", None)
        symbol = _make_document_symbol(local.name, types.SymbolKind.Variable, local.span, source, path,
                                       type_spec.name if type_spec else "composition value")
        if symbol:
            children.append(symbol)
    name = getattr(declaration, "name", None) or ("implementation" if getattr(declaration, "kind", "") == "implementation_block" else None)
    if not name:
        return None
    kind_map = {
        "implementation_block": types.SymbolKind.Namespace, "operatingsystem": types.SymbolKind.Namespace,
        "taskset": types.SymbolKind.Namespace, "subsystem": types.SymbolKind.Class, "state_machine": types.SymbolKind.Enum,
        "custom_algorithm": types.SymbolKind.Function, "hybrid_algorithm": types.SymbolKind.Function,
        "custom_protocol": types.SymbolKind.Interface, "model": types.SymbolKind.Class, "algorithm": types.SymbolKind.Function,
        "sensor": types.SymbolKind.Class, "actuator": types.SymbolKind.Class, "peripheral": types.SymbolKind.Class,
        "bus": types.SymbolKind.Class, "protocol": types.SymbolKind.Interface, "task": types.SymbolKind.Function,
        "loop": types.SymbolKind.Function, "project_routine": types.SymbolKind.Method,
        "signal": types.SymbolKind.Variable, "run": types.SymbolKind.Function,
        "state": types.SymbolKind.EnumMember, "transition": types.SymbolKind.Event, "message": types.SymbolKind.Struct,
        "fragment": types.SymbolKind.Struct, "field": types.SymbolKind.Field,
    }
    kind = kind_map.get(getattr(declaration, "kind", ""), types.SymbolKind.Object)
    detail = getattr(declaration, "registry_key", None) or getattr(declaration, "signature_type", None)
    return _make_document_symbol(name, kind, getattr(declaration, "span", None), source, path, detail, children)

def _project_document_symbols(document_ast: object, source: str, path: Path | None) -> list[types.DocumentSymbol]:
    """Return the outline for a `.uef` project, retaining its source hierarchy."""
    children: list[types.DocumentSymbol] = []
    for binding in getattr(document_ast, "parameters", ()):
        symbol = _make_document_symbol(binding.name, types.SymbolKind.Constant, binding.span, source, path,
                                       binding.type_spec.name if binding.type_spec else "project parameter")
        if symbol:
            children.append(symbol)
    for collection, kind in (("records", types.SymbolKind.Struct), ("enums", types.SymbolKind.Enum),
                             ("variants", types.SymbolKind.Enum), ("flags", types.SymbolKind.Enum),
                             ("fixed_point_types", types.SymbolKind.TypeParameter)):
        for declaration in getattr(document_ast, collection, ()):
            members: list[types.DocumentSymbol] = []
            for member in (*getattr(declaration, "fields", ()), *getattr(declaration, "variants", ()),
                           *getattr(declaration, "cases", ()), *getattr(declaration, "members", ())):
                member_name = getattr(member, "name", None)
                member_span = getattr(member, "span", None)
                if member_name and member_span:
                    detail = getattr(getattr(member, "type_spec", None), "name", None)
                    symbol = _make_document_symbol(member_name, types.SymbolKind.Field if hasattr(member, "type_spec") else types.SymbolKind.EnumMember,
                                                   member_span, source, path, detail)
                    if symbol:
                        members.append(symbol)
            symbol = _make_document_symbol(declaration.name, kind, declaration.span, source, path,
                                           getattr(declaration, "storage_type", None), members)
            if symbol:
                children.append(symbol)
    for declaration in getattr(document_ast, "declarations", ()):
        if getattr(declaration, "kind", "") in {"record", "enum", "variant", "flags", "fixed_point"}:
            continue
        symbol = _declaration_symbols(declaration, source, path)
        if symbol:
            children.append(symbol)
    root = _make_document_symbol(document_ast.name, types.SymbolKind.Module, document_ast.span, source, path,
                                 "UEF-PL project", children)
    return [root] if root else children

def _system_document_symbols(system: object, source: str, path: Path | None) -> list[types.DocumentSymbol]:
    """Return a hierarchical outline for `.uefs` nodes, protocols, and links."""
    children: list[types.DocumentSymbol] = []
    for collection, kind, detail in (("nodes", types.SymbolKind.Module, "node project"),
                                     ("external_endpoints", types.SymbolKind.Interface, "external endpoint"),
                                     ("links", types.SymbolKind.Event, "communication link"),
                                     ("hybrid_systems", types.SymbolKind.Class, "hybrid system")):
        for item in getattr(system, collection, ()):
            span = getattr(item, "source_span", None)
            if span:
                symbol = _make_document_symbol(item.name, kind, span, source, path, detail)
                if symbol:
                    children.append(symbol)
    for collection, kind in (("records", types.SymbolKind.Struct), ("enums", types.SymbolKind.Enum),
                             ("variants", types.SymbolKind.Enum), ("flags_types", types.SymbolKind.Enum),
                             ("fixed_point_types", types.SymbolKind.TypeParameter)):
        for item in getattr(system, collection, ()):
            members = []
            for field in getattr(item, "fields", ()):
                field_name = field[0] if isinstance(field, tuple) else getattr(field, "name", None)
                field_span = (field[2].get("source_span", getattr(item, "source_span", None))
                              if isinstance(field, tuple) else getattr(field, "span", None))
                if field_name and field_span:
                    symbol = _make_document_symbol(field_name, types.SymbolKind.Field, field_span, source, path)
                    if symbol:
                        members.append(symbol)
            for member in (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ())):
                if getattr(member, "name", None) and getattr(member, "span", None):
                    symbol = _make_document_symbol(member.name, types.SymbolKind.EnumMember, member.span, source, path)
                    if symbol:
                        members.append(symbol)
            symbol = _make_document_symbol(item.name, kind, getattr(item, "source_span", getattr(item, "span", None)), source, path,
                                           getattr(item, "storage_type", None), members)
            if symbol:
                children.append(symbol)
    for protocol in getattr(system, "protocols", ()):
        messages: list[types.DocumentSymbol] = []
        for message in protocol.messages:
            message_span = message.get("source_span")
            fields_ = [_make_document_symbol(item.get("name", ""), types.SymbolKind.Field, item.get("source_span"), source, path,
                                             getattr(item.get("type"), "name", None)) for item in message.get("fields", ())]
            fields_ = [item for item in fields_ if item]
            symbol = _make_document_symbol(message.get("name", ""), types.SymbolKind.Struct, message_span, source, path,
                                           f"message ID {message.get('id', 'unspecified')}", fields_)
            if symbol:
                messages.append(symbol)
        symbol = _make_document_symbol(protocol.name, types.SymbolKind.Interface, protocol.source_span, source, path,
                                       "custom wire protocol", messages)
        if symbol:
            children.append(symbol)
    for policy in getattr(system, "delivery_policies", ()):
        span = policy.get("source_span")
        name = policy.get("name")
        if name and span:
            symbol = _make_document_symbol(name, types.SymbolKind.Property, span, source, path, "delivery policy")
            if symbol:
                children.append(symbol)
    root = _make_document_symbol(system.name, types.SymbolKind.Module, system.source_span, source, path, "UEF-PL system", children)
    return [root] if root else children

@server.feature(types.TEXT_DOCUMENT_DOCUMENT_SYMBOL, types.DocumentSymbolOptions())
def document_symbols(_ls: LanguageServer, params: types.DocumentSymbolParams) -> list[types.DocumentSymbol]:
    """Provide the editor outline, breadcrumbs, and symbol navigation tree."""
    document = server.workspace.get_text_document(params.text_document.uri)
    path = _path_from_uri(params.text_document.uri)
    if path is None:
        return []
    if path.suffix.casefold() == ".uefs":
        tokens, _ = Lexer(document.source, path).tokenize()
        system, _ = SystemParser(tokens, path).parse()
        return _system_document_symbols(system, document.source, path) if system else []
    ast = _parse_partial_source(document.source, path)
    return _project_document_symbols(ast, document.source, path) if ast else []

def _local_declarations(project: object | None, system: object | None) -> list[tuple[str, object, str]]:
    """Index declaration names for references and safe, file-local rename."""
    entries: list[tuple[str, object, str]] = []
    def add(name: object, span: object, kind: str):
        if isinstance(name, str) and name and span is not None:
            entries.append((name, span, kind))
    def add_binding(binding: object, kind: str | None = None):
        add(getattr(binding, "name", None), getattr(binding, "span", None), kind or getattr(binding, "role", "variable"))
    def add_decl(declaration: object, *, include_name: bool = True, skip_bindings: set[str] | None = None):
        decl_kind = getattr(declaration, "kind", "declaration")
        # ``run = module(...)`` is a call site, not a declaration of another
        # symbol named after the module. Call-site navigation is handled by
        # _composition_call_target; indexing the name here makes ordinary
        # references to the module ambiguous with this invocation.
        if include_name and decl_kind not in {"implementation_block", "run"}:
            add(getattr(declaration, "name", None), getattr(declaration, "span", None), decl_kind)
        for binding in getattr(declaration, "bindings", ()):
            if not skip_bindings or getattr(binding, "name", None) not in skip_bindings:
                add_binding(binding)
        for stage in getattr(declaration, "stages", ()):
            add(getattr(stage, "name", None), getattr(stage, "span", None), "stage")
            for binding in getattr(stage, "parameters", ()):
                add_binding(binding, "parameter")
        for stage in getattr(declaration, "composition_stages", ()):
            add(getattr(stage, "name", None), getattr(stage, "span", None), "stage")
        for local in (*getattr(declaration, "composition_locals", ()), *getattr(declaration, "composition_outputs", ())):
            add(getattr(local, "name", None), getattr(local, "span", None), "local")
        for child in getattr(declaration, "children", ()):
            add_decl(child)
    if project is not None:
        add(getattr(project, "name", None), getattr(project, "span", None), "project")
        for binding in getattr(project, "parameters", ()):
            add_binding(binding, "parameter")
        for collection, kind in (("records", "record"), ("enums", "enum"), ("variants", "variant"), ("flags", "flags"), ("fixed_point_types", "fixed_point")):
            for item in getattr(project, collection, ()):
                add(getattr(item, "name", None), getattr(item, "span", None), kind)
                for field in getattr(item, "fields", ()):
                    add_binding(field, "field")
                for member in (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ())):
                    add(getattr(member, "name", None), getattr(member, "span", None), "member")
                    for field in getattr(member, "fields", ()):
                        add_binding(field, "field")
        top_level_names = {item.name for item in getattr(project, "declarations", ()) if getattr(item, "name", None) and getattr(item, "kind", "") != "implementation_block"}
        for declaration in getattr(project, "declarations", ()):
            if getattr(declaration, "kind", "") == "implementation_block":
                for child in getattr(declaration, "children", ()):
                    # A component body implements the public signature with the
                    # same name; it is one semantic symbol, not a collision.
                    signature = next((item for item in project.declarations
                                      if item.name == child.name and item.prototype), None) if child.name in top_level_names else None
                    signature_bindings = {item.name for item in getattr(signature, "bindings", ())} if signature else set()
                    add_decl(child, include_name=child.name not in top_level_names, skip_bindings=signature_bindings)
            else:
                add_decl(declaration)
    if system is not None:
        add(getattr(system, "name", None), getattr(system, "source_span", None), "system")
        for collection in ("nodes", "external_endpoints", "records", "enums", "protocols", "links", "hybrid_systems", "variants", "flags_types", "fixed_point_types"):
            for item in getattr(system, collection, ()):
                span = getattr(item, "source_span", getattr(item, "span", None))
                add(getattr(item, "name", None), span, collection.removesuffix("s"))
                for field in getattr(item, "fields", ()):
                    if isinstance(field, tuple):
                        add(field[0], field[2].get("source_span", span), "field")
                    else:
                        add_binding(field, "field")
                for member in (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ())):
                    add(getattr(member, "name", None), getattr(member, "span", None), "member")
                    for field in getattr(member, "fields", ()):
                        add_binding(field, "field")
                for message in getattr(item, "messages", ()):
                    if isinstance(message, dict):
                        add(message.get("name"), message.get("source_span"), "message")
                        for field in message.get("fields", ()):
                            add(field.get("name"), field.get("source_span"), "field")
        for policy in getattr(system, "delivery_policies", ()):
            add(policy.get("name"), policy.get("source_span"), "delivery_policy")
    unique: dict[tuple[str, int, int], tuple[str, object, str]] = {}
    for name, span, kind in entries:
        start = getattr(getattr(span, "start", None), "offset", -1)
        end = getattr(getattr(span, "end", None), "offset", -1)
        unique.setdefault((name, start, end), (name, span, kind))
    return list(unique.values())

@lru_cache(maxsize=24)
def _parse_editor_ast(path: Path, source: str) -> tuple[object | None, object | None]:
    """Parse one editor buffer with the appropriate recoverable frontend."""
    if path.suffix.casefold() == ".uefs":
        tokens, _ = Lexer(source, path).tokenize()
        system, _ = SystemParser(tokens, path).parse()
        return None, system
    return _parse_partial_source(source, path), None

def _ast_nodes_with_ancestors(root: object):
    """Yield AST nodes with their parent chain, without revisiting shared nodes."""
    seen: set[int] = set()
    def walk(value: object, ancestors: tuple[object, ...]):
        if value is None or isinstance(value, (str, int, float, bool, Path)):
            return
        identity = id(value)
        if identity in seen:
            return
        seen.add(identity)
        yield value, ancestors
        if is_dataclass(value):
            next_ancestors = (*ancestors, value)
            for field in fields(value):
                if field.name not in {"span", "source_span", "path"}:
                    yield from walk(getattr(value, field.name), next_ancestors)
        elif isinstance(value, dict):
            for child in value.values():
                yield from walk(child, ancestors)
        elif isinstance(value, (tuple, list, set)):
            for child in value:
                yield from walk(child, ancestors)
    yield from walk(root, ())

def _span_offsets(span: object | None) -> tuple[int, int] | None:
    start = getattr(getattr(span, "start", None), "offset", None)
    end = getattr(getattr(span, "end", None), "offset", None)
    return (start, end) if isinstance(start, int) and isinstance(end, int) else None

def _span_contains(span: object | None, offset: int) -> bool:
    bounds = _span_offsets(span)
    return bounds is not None and bounds[0] <= offset < bounds[1]

def _source_token_at(source: str, line_number: int, character: int) -> tuple[str, int, int] | None:
    """Return an identifier and absolute Python-code-point offsets at an LSP position."""
    lines = source.splitlines(keepends=True)
    if line_number < 0 or line_number >= len(lines):
        return None
    line = lines[line_number].rstrip("\r\n")
    word = _word_at(line, _utf16_to_codepoint(line, character))
    if word is None:
        return None
    line_start = sum(len(item) for item in lines[:line_number])
    return word[0], line_start + word[1], line_start + word[2]

def _node_span(node: object) -> object | None:
    return getattr(node, "span", None) or getattr(node, "source_span", None)

def _exact_local_declaration(project: object | None, system: object | None, name: str,
                              token_start: int, token_end: int, source: str) -> tuple[str, object, str] | None:
    """Resolve a cursor on a declaration name, including duplicate member names."""
    matches: list[tuple[str, object, str]] = []
    for entry in _local_declarations(project, system):
        declared_name, span, kind = entry
        if declared_name != name or _name_offsets(name, span, source) != (token_start, token_end):
            continue
        matches.append(entry)
    if not matches:
        return None
    priority = {"member": 0, "field": 1, "parameter": 2, "input": 2, "output": 2,
                "state": 2, "local": 2, "stage": 3}
    return min(matches, key=lambda item: (priority.get(item[2], 4), (_span_offsets(item[1]) or (0, len(source)))[1] - (_span_offsets(item[1]) or (0, len(source)))[0]))

def _nearest_project_binding(project: object, name: str, offset: int) -> object | None:
    """Find a typed binding visible at this cursor, honoring nested declaration scope."""
    visible: list[tuple[int, int, object]] = []
    for node, ancestors in _ast_nodes_with_ancestors(project):
        node_type = type(node).__name__
        if node_type not in {"TypedBinding", "CompositionLocal", "CompositionOutput", "CompositionStage"} and not (
            node_type == "Declaration" and getattr(node, "kind", None) == "use"
        ):
            continue
        if getattr(node, "name", None) != name:
            continue
        owner = next((item for item in reversed(ancestors) if type(item).__name__ == "Declaration"), None)
        if owner is None or not _span_contains(getattr(owner, "span", None), offset):
            continue
        node_span = _node_span(node)
        bounds = _span_offsets(node_span)
        if bounds is not None and bounds[0] > offset:
            continue
        stage = next((item for item in reversed(ancestors) if type(item).__name__ == "AlgorithmStage"), None)
        if stage is not None and not _span_contains(getattr(stage, "span", None), offset):
            continue
        # Lower value wins: the innermost owner takes precedence; within that
        # owner C-like declarations shadow earlier same-named locals.
        owner_size = (_span_offsets(getattr(owner, "span", None)) or (0, len(getattr(project, "source", ""))))
        visible.append((owner_size[1] - owner_size[0], -bounds[0] if bounds else 0, node))
    if visible:
        return min(visible, key=lambda item: (item[0], item[1]))[2]
    return None

def _project_type_definition(project: object, type_name: str) -> object | None:
    for collection in ("records", "enums", "variants", "flags", "fixed_point_types"):
        for item in getattr(project, collection, ()):
            if getattr(item, "name", None) == type_name:
                return item
    source_path = getattr(project, "source_path", None)
    if source_path is not None:
        from uef_gen.language.frontends.project.contract_imports import resolve_contract_import_path
        for import_decl in getattr(project, "contract_imports", ()):
            contract_path, diagnostics = resolve_contract_import_path(import_decl, source_path)
            if diagnostics or contract_path is None:
                continue
            contract_source = _source_for_path(contract_path)
            if contract_source is None:
                continue
            system = _parse_editor_ast(contract_path, contract_source)[1]
            for collection in ("records", "enums", "variants", "flags_types", "fixed_point_types"):
                for item in getattr(system, collection, ()):
                    if getattr(item, "name", None) == type_name:
                        return item
    return None

def _project_descriptor_for_receiver(project: object, receiver: str) -> str | None:
    for node in _walk_nodes(project):
        if type(node).__name__ == "Declaration" and getattr(node, "name", None) == receiver:
            key = getattr(node, "registry_key", None)
            if key:
                return key
    return None

def _property_descriptor_at(project: object, source: str, name: str, span: object) -> str | None:
    """Find the UETD descriptor whose authored property is under the cursor."""
    target_bounds = _span_offsets(span)
    for node, ancestors in _ast_nodes_with_ancestors(project):
        if type(node).__name__ != "Property" or getattr(node, "name", None) != name or _span_offsets(_node_span(node)) != target_bounds:
            continue
        owner = next((item for item in reversed(ancestors) if type(item).__name__ == "Declaration"), None)
        if owner is None:
            continue
        descriptor = getattr(owner, "registry_key", None)
        if descriptor:
            return descriptor
        owner_name = getattr(owner, "name", None)
        if owner_name:
            signature = next((item for item in getattr(project, "declarations", ())
                              if getattr(item, "name", None) == owner_name and getattr(item, "prototype", False)), None)
            if signature is not None:
                return getattr(signature, "registry_key", None)
    return None

def _type_member_node(type_definition: object | None, member_name: str) -> object | None:
    if type_definition is None:
        return None
    for collection in ("fields", "variants", "cases", "members"):
        for item in getattr(type_definition, collection, ()):
            item_name = item[0] if isinstance(item, tuple) and item else getattr(item, "name", None)
            if item_name == member_name:
                return item
    return None

def _project_receiver_type(project: object, receiver: str, offset: int,
                           catalogue: AuthoringCatalogue | None) -> tuple[str | None, object | None, object | None, bool]:
    """Resolve a project's dotted receiver to its final type declaration.

    The boolean result marks synthetic ``Received<T>`` metadata, which has no
    authored field declaration and should navigate to the envelope annotation.
    This mirrors completion's component-output and wrapper unwrapping rules.
    """
    parts = receiver.split(".")
    if not parts or not parts[0]:
        return None, None, None, False
    root, remaining = parts[0], parts[1:]
    binding = _nearest_project_binding(project, root, offset)
    type_name = _hover_type_name(getattr(binding, "type_spec", None)) if binding is not None else None

    if type_name is None:
        channel_type = _channel_payload_type(project, catalogue, root)
        if channel_type:
            if remaining and remaining[0] == "value":
                type_name, remaining = channel_type, remaining[1:]
            elif remaining and remaining[0] in {"fresh", "sequence", "received_at_us"}:
                return "Received<" + channel_type + ">", None, None, True

    if type_name is None:
        outputs = _local_component_outputs(project, catalogue, root)
        if outputs and remaining:
            selected = next((item for item in outputs if item[0] == remaining[0]), None)
            if selected is not None:
                type_name, remaining = selected[1], remaining[1:]

    if type_name is None:
        machine = next((item for item in _walk_nodes(project)
                        if type(item).__name__ == "Declaration"
                        and getattr(item, "kind", None) == "state_machine"
                        and getattr(item, "name", None) == root), None)
        if machine is not None and remaining and remaining[0] == "state":
            type_name, remaining = getattr(machine, "registry_key", None), remaining[1:]

    if type_name is None:
        type_name = root
        type_definition = _project_type_definition(project, type_name)
    else:
        type_definition = None

    received = re.fullmatch(r"Received\s*<\s*(.+)\s*>", (type_name or "").strip())
    if received:
        payload = received.group(1).strip()
        if remaining and remaining[0] == "value":
            type_name, remaining = payload, remaining[1:]
        elif remaining and remaining[0] in {"fresh", "sequence", "received_at_us"}:
            return type_name, None, binding, True

    type_definition = type_definition or _project_type_definition(project, type_name or "")
    for member_name in remaining:
        if type_definition is None:
            return type_name, None, binding, False
        member = _type_member_node(type_definition, member_name)
        if member is None:
            return type_name, None, binding, False
        if isinstance(member, tuple):
            member_type = member[1] if len(member) > 1 else None
            type_name = _hover_type_name(member_type) if member_type is not None else None
        else:
            type_spec = getattr(member, "type_spec", None)
            type_name = _hover_type_name(type_spec) if type_spec is not None else None
        received = re.fullmatch(r"Received\s*<\s*(.+)\s*>", (type_name or "").strip())
        if received:
            type_name = received.group(1).strip()
        type_definition = _project_type_definition(project, type_name or "")
    return type_name, type_definition, binding, False

def _project_member_declaration(project: object, source: str, offset: int, name: str,
                                catalogue: AuthoringCatalogue | None = None) -> tuple[str, object, str] | None:
    """Resolve ``receiver.member`` to its project/imported type member declaration."""
    receiver = _qualified_receiver_before(source, offset)
    if receiver is None:
        return None
    parts = receiver.split(".")
    if len(parts) == 1:
        routine = next((item for item in getattr(project, "declarations", ())
                        if getattr(item, "kind", None) == "project_routine"
                        and getattr(item, "registry_key", None) == parts[0]
                        and getattr(item, "name", None) == name), None)
        if routine is not None:
            return name, _node_span(routine), "project_routine"
    source_path = getattr(project, "source_path", None)
    receiver_type, receiver_definition, receiver_binding, received_metadata = _project_receiver_type(
        project, receiver, offset, catalogue
    )
    if receiver_definition is not None:
        member = _type_member_node(receiver_definition, name)
        if member is not None:
            span = member[2].get("source_span") if isinstance(member, tuple) and len(member) > 2 else _node_span(member)
            if span is not None:
                kind = "member" if type(member).__name__ in {"EnumVariant", "VariantCaseDefinition", "FlagsMember"} else "field"
                return name, span, kind
    if received_metadata and receiver_binding is not None:
        type_span = getattr(getattr(receiver_binding, "type_spec", None), "span", None)
        if type_span is not None:
            return name, type_span, "member"
    if receiver_type and catalogue is not None and source_path is not None:
        vector_member = _uetd_vector_member_declaration(name, receiver_type, catalogue, source_path)
        if vector_member is not None:
            return name, vector_member[2], "registry_member"
    if len(parts) == 1:
        for declaration in _walk_nodes(project):
            if type(declaration).__name__ != "Declaration" or getattr(declaration, "name", None) != parts[0]:
                continue
            output = next((binding for binding in getattr(declaration, "bindings", ())
                           if getattr(binding, "name", None) == name and getattr(binding, "role", None) == "output"), None)
            if output is not None:
                return name, getattr(output, "span", None), "output"
    type_definition = _project_type_definition(project, parts[0])
    if type_definition is None:
        receiver_key = parts[0]
        # A named component/subsystem publishes its own typed output ports.
        if len(parts) > 1:
            for declaration in _walk_nodes(project):
                if type(declaration).__name__ != "Declaration" or getattr(declaration, "name", None) != parts[0]:
                    continue
                output = next((binding for binding in getattr(declaration, "bindings", ())
                               if getattr(binding, "name", None) == parts[1] and getattr(binding, "role", None) == "output"), None)
                if output is not None and getattr(output, "type_spec", None) is not None:
                    receiver_key = output.type_spec.name
                    type_definition = _project_type_definition(project, output.type_spec.name)
                    if type_definition is not None:
                        # `type_definition` is the type of parts[1], so keep
                        # the remaining receiver path for nested fields.
                        parts = parts[1:]
                        break
        if type_definition is None:
            binding = _nearest_project_binding(project, parts[0], offset)
            type_spec = getattr(binding, "type_spec", None)
            if type_spec is not None:
                receiver_key = getattr(type_spec, "name", receiver_key)
                type_definition = _project_type_definition(project, getattr(type_spec, "name", ""))
        # A registered component may publish fields documented by its UETD contract.
        if type_definition is None and catalogue is not None:
            # Resolve the complete receiver path first. This covers nested
            # output paths such as ``pipeline.navigation.velocity_ned.z``
            # where the vector is several typed ports/record fields away from
            # the first identifier.
            visible_members = _members_for_receiver(project, catalogue, receiver, offset)
            selected_vector_member = next((item for item in visible_members
                                           if item[0] == name and item[2].startswith("Component of ")), None)
            if selected_vector_member is not None and source_path is not None:
                vector_type = selected_vector_member[2].removeprefix("Component of ").strip("`")
                external = _uetd_vector_member_declaration(name, vector_type, catalogue, source_path)
                if external is not None:
                    return name, external[2], "registry_member"
            selected_member = next((item for item in visible_members if item[0] == name), None)
            if selected_member is not None and selected_member[2].startswith("Received<T>"):
                if name == "value" and selected_member[1]:
                    payload_definition = _project_type_definition(project, selected_member[1])
                    if payload_definition is not None:
                        return name, _node_span(payload_definition), "member"
                    if source_path is not None:
                        external = _registered_type_declaration(selected_member[1], catalogue, source_path)
                        if external is not None:
                            return name, external[2], "registry_type"
                # Runtime metadata has no user-authored field declaration.
                # Anchor navigation at the explicit Received<T> annotation
                # that introduced this value instead of returning no target.
                binding = _nearest_project_binding(project, parts[0], offset)
                type_span = getattr(getattr(binding, "type_spec", None), "span", None)
                if type_span is not None:
                    return name, type_span, "member"
            descriptor = _project_descriptor_for_receiver(project, parts[0]) or receiver_key
            if descriptor and source_path is not None:
                external = _registered_member_declaration(name, descriptor, catalogue, source_path)
                if external is not None:
                    return name, external[2], "registry_member"
            # A component invocation may publish a local record output.
            # Resolve the final receiver segment as a port/record field on its
            # parent. Looking at the fully qualified receiver here returns the
            # members of that port's type (for example x/y/z), which loses the
            # port's own type before Ctrl+Click on ``imu.angular_rate.x``.
            parent_receiver = ".".join(parts[:-1])
            output_members = _members_for_receiver(project, catalogue, parent_receiver) if parent_receiver else []
            selected = next((item for item in output_members if item[0] == parts[-1]), None)
            if selected and selected[1]:
                receiver_key = selected[1]
                type_definition = _project_type_definition(project, selected[1])
                parts = []
    member: object | None = None
    for part in (*parts[1:], name):
        if type_definition is None:
            external = (_uetd_vector_member_declaration(part, receiver_key, catalogue, source_path)
                        if source_path is not None else None)
            if external is not None and part == name:
                return name, external[2], "registry_member"
            return None
        member = _type_member_node(type_definition, part)
        if member is None:
            return None
        type_spec = member[1] if isinstance(member, tuple) and len(member) > 1 else getattr(member, "type_spec", None)
        receiver_key = getattr(type_spec, "name", "") if type_spec is not None else ""
        type_definition = _project_type_definition(project, receiver_key) if receiver_key else None
    span = member[2].get("source_span") if isinstance(member, tuple) and len(member) > 2 else _node_span(member)
    if span is None:
        return None
    kind = "member" if type(member).__name__ in {"EnumVariant", "VariantCaseDefinition", "FlagsMember"} else "field"
    return name, span, kind

def _system_member_declaration(system: object, source: str, offset: int, name: str) -> tuple[str, object, str] | None:
    """Resolve a type-qualified member in a `.uefs` document."""
    receiver = _qualified_receiver_before(source, offset)
    if receiver is None:
        return None
    parts = receiver.split(".")
    type_definition = next((item for collection in ("records", "enums", "variants", "flags_types", "fixed_point_types")
                            for item in getattr(system, collection, ()) if getattr(item, "name", None) == parts[0]), None)
    if type_definition is None:
        return None
    member: object | None = None
    for part in (*parts[1:], name):
        member = _type_member_node(type_definition, part)
        if member is None:
            return None
        type_spec = member[1] if isinstance(member, tuple) and len(member) > 1 else getattr(member, "type_spec", None)
        type_name = getattr(type_spec, "name", "") if type_spec is not None else ""
        type_definition = next((item for collection in ("records", "enums", "variants", "flags_types", "fixed_point_types")
                                for item in getattr(system, collection, ()) if getattr(item, "name", None) == type_name), None)
    span = member[2].get("source_span") if isinstance(member, tuple) and len(member) > 2 else _node_span(member)
    if span is None:
        return None
    kind = "member" if type(member).__name__ in {"EnumVariant", "VariantCaseDefinition", "FlagsMember"} else "field"
    return name, span, kind

def _state_machine_member_declaration(project: object, source: str, offset: int, name: str) -> tuple[str, object, str] | None:
    """Map FSM state references back to the state enum member they denote."""
    for node, ancestors in _ast_nodes_with_ancestors(project):
        if type(node).__name__ != "Declaration" or getattr(node, "kind", None) != "state_machine":
            continue
        state_type = getattr(node, "registry_key", None)
        if not state_type:
            continue
        enum = _project_type_definition(project, state_type)
        if enum is None:
            continue
        for child in getattr(node, "children", ()):
            span = getattr(child, "span", None)
            bounds = _span_offsets(span)
            if bounds is None or not bounds[0] <= offset < bounds[1]:
                continue
            if getattr(child, "kind", None) == "transition":
                start, end = bounds
                text = source[start:end]
                src_name = getattr(child, "name", None)
                dst_name = getattr(child, "registry_key", None)
                if src_name and dst_name:
                    match = re.search(rf"\btransition\s+({re.escape(src_name)})\s*->\s*({re.escape(dst_name)})\b", text)
                    if match:
                        for group, state_name in zip((1, 2), (src_name, dst_name)):
                            state_start = start + match.start(group)
                            state_end = start + match.end(group)
                            if state_start <= offset < state_end and state_name == name:
                                member = _type_member_node(enum, name)
                                member_span = _node_span(member) if member is not None else None
                                return (name, member_span, "member") if member_span is not None else None
            if getattr(child, "kind", None) == "state_action" and getattr(child, "name", None) == name:
                state_start = _name_offsets(name, span, source)
                if state_start and state_start[0] <= offset < state_start[1]:
                    member = _type_member_node(enum, name)
                    member_span = _node_span(member) if member is not None else None
                    return (name, member_span, "member") if member_span is not None else None
        for prop in getattr(node, "properties", ()):
            value = getattr(prop, "value", None)
            if getattr(prop, "name", None) == "initial" and getattr(value, "value", None) == name:
                value_span = getattr(value, "span", None)
                if _span_contains(value_span, offset):
                    member = _type_member_node(enum, name)
                    member_span = _node_span(member) if member is not None else None
                    return (name, member_span, "member") if member_span is not None else None
    return None

def _composition_call_target(project: object, source: str, offset: int, name: str) -> tuple[str, object, str] | None:
    """Resolve call-site names in `run` and composition stages to their declarations."""
    for node, ancestors in _ast_nodes_with_ancestors(project):
        node_type = type(node).__name__
        if node_type == "CompositionStage" and getattr(node, "member", None) == name and _span_contains(getattr(node, "span", None), offset):
            node_span = _node_span(node)
            bounds = _span_offsets(node_span)
            if bounds is None or not re.search(r"=\s*$", source[bounds[0]:offset]):
                continue
            owner = next((item for item in reversed(ancestors) if type(item).__name__ == "Declaration"), None)
            alias = next((child for child in getattr(owner, "children", ())
                          if getattr(child, "kind", None) == "use" and getattr(child, "name", None) == name), None)
            if alias is not None:
                return name, getattr(alias, "span", None), "use"
        if node_type == "Declaration" and getattr(node, "kind", None) == "run" and getattr(node, "name", None) == name:
            if _name_offsets(name, getattr(node, "span", None), source) != (offset, offset + len(name)):
                continue
            candidates = [entry for entry in _local_declarations(project, None)
                          if entry[0] == name and entry[2] in {"custom_algorithm", "hybrid_algorithm", "algorithm", "component", "model", "subsystem"}]
            if len(candidates) == 1:
                return candidates[0]
    return None


def _project_type_spec_at(project: object, name: str, offset: int) -> bool:
    """Tell whether the cursor is on a type name in a project type expression."""
    return any(type(node).__name__ == "TypeSpec"
               and getattr(node, "name", None) == name
               and _span_contains(_node_span(node), offset)
               for node in _walk_nodes(project))

def _resolve_project_symbol_at(project: object, source: str, line_number: int, character: int,
                               catalogue: AuthoringCatalogue | None = None) -> tuple[str, object, str] | None:
    token = _source_token_at(source, line_number, character)
    if token is None:
        return None
    name, token_start, token_end = token
    if catalogue is not None and _project_type_spec_at(project, name, token_start):
        source_path = getattr(project, "source_path", None)
        external = _registered_type_declaration(name, catalogue, source_path) if source_path is not None else None
        if external is not None:
            return name, external[2], "registry_type"
    state_member = _state_machine_member_declaration(project, source, token_start, name)
    if state_member is not None:
        return state_member
    call_target = _composition_call_target(project, source, token_start, name)
    if call_target is not None:
        return call_target
    exact = _exact_local_declaration(project, None, name, token_start, token_end, source)
    if exact is not None:
        return exact
    for node, _ancestors in _ast_nodes_with_ancestors(project):
        if type(node).__name__ == "Property" and getattr(node, "name", None) == name:
            if _name_offsets(name, getattr(node, "span", None), source) == (token_start, token_end):
                return name, getattr(node, "span", None), "property"
    member = _project_member_declaration(project, source, token_start, name, catalogue)
    if member is not None:
        return member
    if _qualified_receiver_before(source, token_start) is not None:
        return None
    offset = token_start
    binding = _nearest_project_binding(project, name, offset)
    if binding is not None:
        return name, _node_span(binding), getattr(binding, "role", getattr(binding, "kind", "variable"))
    # Statically bounded loop and pattern bindings have explicit local scopes.
    for node, _ancestors in _ast_nodes_with_ancestors(project):
        node_type = type(node).__name__
        if node_type == "BoundedLoop" and getattr(node, "variable", None) == name and _span_contains(getattr(node, "span", None), offset):
            return name, getattr(node, "span", None), "local"
        if node_type == "MatchArm" and getattr(node, "binding_name", None) == name and _span_contains(getattr(node, "span", None), offset):
            return name, getattr(node, "span", None), "local"
        if node_type == "MatchArm" and any(binding_name == name for _field_name, binding_name in getattr(node, "field_bindings", ())):
            if _span_contains(getattr(node, "span", None), offset):
                return name, getattr(node, "span", None), "local"
    global_matches = [entry for entry in _local_declarations(project, None)
                      if entry[0] == name and entry[2] not in {"field", "member", "input", "output", "parameter", "state", "local"}]
    if len(global_matches) == 1:
        return global_matches[0]
    # A repeated project parameter is not expected, but the project root is a
    # clear lexical scope and must still beat unrelated function-local names.
    parameters = [item for item in getattr(project, "parameters", ()) if item.name == name]
    if len(parameters) == 1:
        return name, parameters[0].span, "parameter"
    return None

def _resolve_system_symbol_at(system: object, source: str, line_number: int, character: int) -> tuple[str, object, str] | None:
    """Resolve a system-contract identifier by its exact declaration or unique scope name."""
    token = _source_token_at(source, line_number, character)
    if token is None:
        return None
    name, token_start, token_end = token
    exact = _exact_local_declaration(None, system, name, token_start, token_end, source)
    if exact is not None:
        return exact
    member = _system_member_declaration(system, source, token_start, name)
    if member is not None:
        return member
    if _qualified_receiver_before(source, token_start) is not None:
        return None
    matches = [item for item in _local_declarations(None, system) if item[0] == name]
    return matches[0] if len(matches) == 1 else None

def _local_symbol_at(path: Path, source: str, line_number: int, character: int) -> tuple[str, object, str] | None:
    """Resolve the cursor to its nearest declaration, respecting lexical scope."""
    project, system = _parse_editor_ast(path, source)
    if project is not None:
        return _resolve_project_symbol_at(project, source, line_number, character, _catalogue_for_document(path.as_uri()))
    return _resolve_system_symbol_at(system, source, line_number, character) if system is not None else None

def _identifier_locations(uri: str, source: str, name: str, *, include_declaration: bool = True,
                          target_span: object | None = None, imported_target_path: Path | None = None) -> list[types.Location]:
    """Return identifier tokens that resolve to the same semantic declaration."""
    path = _path_from_uri(uri)
    tokens, _ = Lexer(source, path).tokenize()
    lines = source.splitlines()
    project, system = _parse_editor_ast(path, source) if path else (None, None)
    catalogue = _catalogue_for_document(uri) if path and path.suffix.casefold() == ".uef" else None
    target_bounds = _span_offsets(target_span)
    declaration_tokens = {
        offsets
        for declared_name, span, _kind in _local_declarations(project, system)
        if declared_name == name and (offsets := _name_offsets(name, span, source)) is not None
    }
    imported_match = False
    if imported_target_path is not None and project is not None and path is not None:
        imported_match = any(
            candidate_path.resolve(strict=False) == imported_target_path.resolve(strict=False)
            and _span_offsets(candidate_span) == target_bounds
            for candidate_path, _candidate_source, candidate_span, _kind in _imported_contract_declarations(project, path, name)
        )
    locations: list[types.Location] = []
    for token in tokens:
        if token.kind.value != "identifier" or token.text != name:
            continue
        token_bounds = (token.span.start.offset, token.span.end.offset)
        if not include_declaration and token_bounds in declaration_tokens:
            continue
        if target_bounds is not None:
            line_index = token.span.start.line - 1
            line = lines[line_index] if line_index < len(lines) else ""
            character = _column_to_utf16(line, token.span.start.column)
            resolved = (_resolve_project_symbol_at(project, source, line_index, character, catalogue)
                        if project is not None else _resolve_system_symbol_at(system, source, line_index, character)
                        if system is not None else None)
            resolved_path = (getattr(resolved[1], "path", None) or path) if resolved is not None else None
            target_path = getattr(target_span, "path", None)
            same_path = (resolved_path is not None and target_path is not None
                         and os.path.normcase(str(Path(resolved_path).resolve(strict=False)))
                         == os.path.normcase(str(Path(target_path).resolve(strict=False))))
            if resolved is None or _span_offsets(resolved[1]) != target_bounds or (target_path is not None and not same_path):
                if not (imported_match and resolved is None):
                    continue
        location = _location_for_span(token.span, path, source=source)
        if location:
            locations.append(location)
    return locations

def _source_for_path(path: Path) -> str | None:
    """Read an open editor buffer first, then the saved source file."""
    target = os.path.normcase(str(path.resolve(strict=False)))
    for uri, document in getattr(server.workspace, "text_documents", {}).items():
        open_path = _path_from_uri(uri)
        if open_path and os.path.normcase(str(open_path.resolve(strict=False))) == target:
            return document.source
    try:
        return path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError):
        return None

def _imported_contract_declarations(project: object, source_path: Path, name: str):
    """Find the uniquely imported `.uefs` declaration visible under `name`."""
    from uef_gen.language.frontends.project.contract_imports import resolve_contract_import_path
    found: dict[tuple[str, int, int], tuple[Path, str, object, str]] = {}
    for import_decl in getattr(project, "contract_imports", ()):
        contract_path, diagnostics = resolve_contract_import_path(import_decl, source_path)
        if diagnostics or contract_path is None:
            continue
        contract_path = contract_path.resolve(strict=False)
        contract_source = _source_for_path(contract_path)
        if contract_source is None:
            continue
        tokens, _ = Lexer(contract_source, contract_path).tokenize()
        system, _ = SystemParser(tokens, contract_path).parse()
        if system is None:
            continue
        for declared_name, span, kind in _local_declarations(None, system):
            if declared_name != name:
                continue
            start = getattr(getattr(span, "start", None), "offset", -1)
            end = getattr(getattr(span, "end", None), "offset", -1)
            key = (str(contract_path), start, end)
            found.setdefault(key, (contract_path, contract_source, span, kind))
    return list(found.values())

def _symbol_target_at(uri: str, source: str, line_number: int, character: int):
    """Resolve a local declaration or one imported system-contract declaration."""
    path = _path_from_uri(uri)
    if path is None:
        return None
    local = _local_symbol_at(path, source, line_number, character)
    if local is not None and local[2] == "property":
        return None
    if local:
        declaration_path = getattr(local[1], "path", None)
        if declaration_path is not None:
            declaration_path = Path(declaration_path).resolve(strict=False)
        if declaration_path is not None and declaration_path != path.resolve(strict=False):
            declaration_source = _source_for_path(declaration_path)
            if declaration_source is not None:
                return declaration_path, declaration_source, local[0], local[1], local[2], True
        return path, source, local[0], local[1], local[2], False
    if path.suffix.casefold() != ".uef":
        return None
    lines = source.splitlines()
    if line_number < 0 or line_number >= len(lines):
        return None
    word = _word_at(lines[line_number], _utf16_to_codepoint(lines[line_number], character))
    if word is None:
        return None
    project = _parse_partial_source(source, path)
    if _resolve_project_symbol_at(project, source, line_number, character, _catalogue_for_document(uri)) is not None:
        return None
    imported = _imported_contract_declarations(project, path, word[0])
    if len(imported) != 1:
        return None
    target_path, target_source, span, kind = imported[0]
    return target_path, target_source, word[0], span, kind, True

def _cursor_word_location(uri: str, source: str, line_number: int, character: int) -> types.Location | None:
    """Return the identifier range under the request cursor in its own document."""
    path = _path_from_uri(uri)
    lines = source.splitlines(keepends=True)
    if path is None or line_number < 0 or line_number >= len(lines):
        return None
    line = lines[line_number].rstrip("\r\n")
    word = _word_at(line, _utf16_to_codepoint(line, character))
    if word is None:
        return None
    start = sum(len(item) for item in lines[:line_number]) + word[1]
    span = _span_from_offsets(path, source, start, start + len(word[0]))
    return _location_for_span(span, path, source=source)

def _project_imports_contract(project: object, project_path: Path, target_path: Path) -> bool:
    """Check an explicit contract import by canonical resolved path."""
    from uef_gen.language.frontends.project.contract_imports import resolve_contract_import_path
    expected = os.path.normcase(str(target_path.resolve(strict=False)))
    for import_decl in getattr(project, "contract_imports", ()):
        path, diagnostics = resolve_contract_import_path(import_decl, project_path)
        if not diagnostics and path and os.path.normcase(str(path.resolve(strict=False))) == expected:
            return True
    return False

def _symbol_locations(target_path: Path, target_source: str, name: str, *, include_declaration: bool,
                      target_span: object | None = None) -> list[types.Location]:
    """Find same-file references and, for `.uefs` types, importing project uses."""
    uri = target_path.resolve(strict=False).as_uri()
    locations = _identifier_locations(uri, target_source, name, include_declaration=include_declaration, target_span=target_span)
    if target_path.suffix.casefold() not in {".uefs", ".json", ".h"}:
        return locations
    if include_declaration and target_span is not None and target_path.suffix.casefold() in {".json", ".h"}:
        declaration = _location_for_span(target_span, target_path, name=name, source=target_source)
        if declaration is not None:
            locations.append(declaration)
    for project_uri, project_path, project_source in _workspace_sources(server.workspace, name):
        if project_path.suffix.casefold() != ".uef":
            continue
        project = _parse_partial_source(project_source, project_path)
        if target_path.suffix.casefold() == ".uefs" and not _project_imports_contract(project, project_path, target_path):
            continue
        # Imported shared-contract names are ambiguous when shadowed locally.
        # Header-backed UETD members are different: each candidate use is
        # resolved against its receiver type below, so unrelated locals named
        # ``x`` or ``w`` must not suppress vector-component references.
        if target_path.suffix.casefold() == ".uefs" and any(
            item[0] == name for item in _local_declarations(project, None)
        ):
            continue
        locations.extend(_identifier_locations(project_uri, project_source, name, target_span=target_span,
                                               imported_target_path=target_path))
    unique: dict[tuple[object, ...], types.Location] = {}
    for location in locations:
        key = (location.uri, location.range.start.line, location.range.start.character,
               location.range.end.line, location.range.end.character)
        unique.setdefault(key, location)
    return list(unique.values())

def _rename_conflicts(target_path: Path, name: str, new_name: str, target_span: object | None = None,
                      target_kind: str | None = None) -> bool:
    """Refuse collisions with local declarations, imported names, or keywords."""
    try:
        registry = _language_registry()
        if new_name in registry.constructs or new_name in registry.builtins:
            return True
    except (OSError, ValueError):
        pass
    if target_path.suffix.casefold() == ".uefs":
        if new_name in _system_language_words():
            return True
        source = _source_for_path(target_path)
        if source is None:
            return True
        system = _parse_editor_ast(target_path, source)[1]
        if any(item[0] == new_name for item in _local_declarations(None, system)):
            return True
        for _uri, project_path, project_source in _workspace_sources(server.workspace, new_name):
            if project_path.suffix.casefold() != ".uef":
                continue
            project = _parse_partial_source(project_source, project_path)
            if not _project_imports_contract(project, project_path, target_path):
                continue
            if any(item[0] == new_name for item in _local_declarations(project, None)):
                return True
            from uef_gen.language.frontends.project.contract_imports import resolve_contract_import_path
            for import_decl in getattr(project, "contract_imports", ()):
                other_path, diagnostics = resolve_contract_import_path(import_decl, project_path)
                if diagnostics or other_path is None or other_path.resolve(strict=False) == target_path.resolve(strict=False):
                    continue
                other_source = _source_for_path(other_path)
                if other_source is None:
                    continue
                other_system = _parse_editor_ast(other_path, other_source)[1]
                if any(item[0] == new_name for item in _local_declarations(None, other_system)):
                    return True
        return False
    source = _source_for_path(target_path)
    if source is None:
        return True
    project = _parse_partial_source(source, target_path)
    target_bounds = _span_offsets(target_span)
    if target_bounds is not None and project is not None:
        target_node = None
        target_ancestors: tuple[object, ...] = ()
        for node, ancestors in _ast_nodes_with_ancestors(project):
            if getattr(node, "name", None) == name and _span_offsets(_node_span(node)) == target_bounds:
                target_node, target_ancestors = node, ancestors
                break
        if target_node is not None:
            node_type = type(target_node).__name__
            owner = next((item for item in reversed(target_ancestors) if type(item).__name__ == "Declaration"), None)
            stage = next((item for item in reversed(target_ancestors) if type(item).__name__ == "AlgorithmStage"), None)
            if node_type in {"TypedBinding", "CompositionLocal", "CompositionOutput", "CompositionStage"} and owner is not None:
                for node, ancestors in _ast_nodes_with_ancestors(project):
                    if getattr(node, "name", None) != new_name:
                        continue
                    candidate_owner = next((item for item in reversed(ancestors) if type(item).__name__ == "Declaration"), None)
                    candidate_stage = next((item for item in reversed(ancestors) if type(item).__name__ == "AlgorithmStage"), None)
                    if candidate_owner is owner and (stage is None or candidate_stage is stage):
                        return True
                return False
            parent_type = next((item for item in reversed(target_ancestors)
                                if type(item).__name__ in {"RecordDefinition", "EnumDefinition", "VariantDefinition", "FlagsDefinition"}), None)
            if parent_type is not None:
                if any(getattr(member, "name", None) == new_name
                       for collection in ("fields", "variants", "cases", "members")
                       for member in getattr(parent_type, collection, ())):
                    return True
                return False
            if target_node in getattr(project, "declarations", ()) or target_node in tuple(_walk_nodes(project)):
                top_level_names = {getattr(item, "name", None) for item in getattr(project, "declarations", ())}
                top_level_names.update(getattr(item, "name", None) for collection in ("records", "enums", "variants", "flags", "fixed_point_types") for item in getattr(project, collection, ()))
                top_level_names.update(item.name for item in getattr(project, "parameters", ()))
                if new_name in top_level_names and new_name != name:
                    return True
                # An FSM transition/action label is an enum member reference;
                # its rename must remain unique within the state type.
                if target_kind == "member":
                    return any(item[0] == new_name and item[2] == "member" for item in _local_declarations(project, None))
                return False
    return any(item[0] == new_name for item in _local_declarations(project, None))

@server.feature(types.TEXT_DOCUMENT_REFERENCES)
def references(_ls: LanguageServer, params: types.ReferenceParams) -> list[types.Location]:
    """Find local references and explicitly imported `.uefs` contract uses."""
    document = server.workspace.get_text_document(params.text_document.uri)
    target = _symbol_target_at(params.text_document.uri, document.source, params.position.line, params.position.character)
    if target is None:
        return []
    return _symbol_locations(target[0], target[1], target[2], include_declaration=params.context.include_declaration,
                             target_span=target[3])

@server.feature(types.TEXT_DOCUMENT_PREPARE_RENAME)
def prepare_rename(ls: LanguageServer, params: types.PrepareRenameParams) -> types.PrepareRenamePlaceholder | types.Range | None:
    """Enable rename only for an unambiguous source or imported-contract symbol."""
    document = server.workspace.get_text_document(params.text_document.uri)
    target = _symbol_target_at(params.text_document.uri, document.source, params.position.line, params.position.character)
    if target is None or target[4] in {"registry_member", "registry_type"}:
        return None
    location = (_cursor_word_location(params.text_document.uri, document.source, params.position.line, params.position.character)
                if target[5] else _location_for_span(target[3], target[0], name=target[2], source=target[1]))
    if location is None:
        return None
    return types.PrepareRenamePlaceholder(range=location.range, placeholder=target[2])

@server.feature(types.TEXT_DOCUMENT_RENAME, types.RenameOptions(prepare_provider=True))
def rename(_ls: LanguageServer, params: types.RenameParams) -> types.WorkspaceEdit | None:
    """Rename a resolved symbol without touching comments, strings, or collisions."""
    document = server.workspace.get_text_document(params.text_document.uri)
    new_name = params.new_name
    if not _WORD.fullmatch(new_name):
        return None
    target = _symbol_target_at(params.text_document.uri, document.source, params.position.line, params.position.character)
    if target is None or target[4] in {"registry_member", "registry_type"} or target[2] == new_name or _rename_conflicts(target[0], target[2], new_name, target[3], target[4]):
        return None
    locations = _symbol_locations(target[0], target[1], target[2], include_declaration=True, target_span=target[3])
    changes: dict[str, list[types.TextEdit]] = {}
    for location in locations:
        changes.setdefault(location.uri, []).append(types.TextEdit(range=location.range, new_text=new_name))
    if not changes:
        return None
    for edits in changes.values():
        edits.sort(key=lambda item: (item.range.start.line, item.range.start.character), reverse=True)
    return types.WorkspaceEdit(changes=changes)

@server.feature(types.TEXT_DOCUMENT_DOCUMENT_HIGHLIGHT)
def document_highlights(_ls: LanguageServer, params: types.DocumentHighlightParams) -> list[types.DocumentHighlight]:
    """Highlight occurrences of a local or explicitly imported symbol in this file."""
    document = server.workspace.get_text_document(params.text_document.uri)
    target = _symbol_target_at(params.text_document.uri, document.source, params.position.line, params.position.character)
    if target is None:
        return []
    locations = _identifier_locations(params.text_document.uri, document.source, target[2], target_span=target[3])
    return [types.DocumentHighlight(range=item.range, kind=types.DocumentHighlightKind.Text) for item in locations]

def _flatten_document_symbols(items: list[types.DocumentSymbol], path: Path, container: str | None = None):
    for item in items:
        yield types.SymbolInformation(name=item.name, kind=item.kind,
                                     location=types.Location(uri=path.as_uri(), range=item.selection_range),
                                     container_name=container)
        yield from _flatten_document_symbols(list(item.children or ()), path, item.name)

def _workspace_sources(workspace: object, query: str):
    """Yield current open buffers and matching `.uef` files under workspace roots."""
    seen: set[str] = set()
    documents = getattr(workspace, "text_documents", {})
    for uri, document in documents.items():
        path = _path_from_uri(uri)
        if path is None or path.suffix.casefold() not in {".uef", ".uefs"}:
            continue
        key = os.path.normcase(str(path))
        seen.add(key)
        yield uri, path, document.source
    if not query:
        return
    folders = getattr(workspace, "folders", {})
    roots = [_path_from_uri(folder.uri) for folder in folders.values()]
    if not roots and getattr(workspace, "root_path", None):
        roots = [Path(workspace.root_path)]
    count = 0
    for root in roots:
        if root is None or not root.is_dir():
            continue
        for current, directories, files_ in os.walk(root):
            directories[:] = sorted((name for name in directories if name not in _IGNORED_WORKSPACE_DIRS and not name.startswith(".")), key=str.casefold)
            for filename in sorted(files_, key=str.casefold):
                path = Path(current, filename)
                if path.suffix.casefold() not in {".uef", ".uefs"}:
                    continue
                key = os.path.normcase(str(path.resolve(strict=False)))
                if key in seen:
                    continue
                seen.add(key)
                count += 1
                if count > 3000:
                    return
                try:
                    source = path.read_text(encoding="utf-8-sig")
                except (OSError, UnicodeError):
                    continue
                if query.casefold() not in source.casefold():
                    continue
                yield path.as_uri(), path, source

@server.feature(types.WORKSPACE_SYMBOL, types.WorkspaceSymbolOptions())
def workspace_symbols(_ls: LanguageServer, params: types.WorkspaceSymbolParams) -> list[types.SymbolInformation]:
    """Search current and saved UEF-PL sources for declarations by name."""
    query = params.query.strip()
    matches: list[types.SymbolInformation] = []
    for _uri, path, source in _workspace_sources(server.workspace, query):
        project, system = _parse_editor_ast(path, source)
        symbols = _project_document_symbols(project, source, path) if project else _system_document_symbols(system, source, path) if system else []
        for symbol in _flatten_document_symbols(symbols, path):
            if not query or query.casefold() in symbol.name.casefold():
                matches.append(symbol)
                if len(matches) >= 1000:
                    return matches
    return matches

def _semantic_tokens_for_document(uri: str, source: str) -> list[int]:
    """Encode registry-aware lexical classifications as an LSP semantic-token stream."""
    path = _path_from_uri(uri)
    lines = source.splitlines()
    tokens, _ = Lexer(source, path).tokenize()
    try:
        language = _language_registry()
    except (OSError, ValueError):
        language = None
    catalogue = _catalogue_for_document(uri)
    project, system = _parse_editor_ast(path, source) if path else (None, None)
    keyword_words = set(_system_language_words()) if path and path.suffix.casefold() == ".uefs" else set()
    if language:
        productions = language.grammar.get("productions", {})
        keyword_words.update(re.findall(r"'([a-z][a-z0-9_]*)'", " ".join(str(value) for value in productions.values())))
        keyword_words.update(language.constructs)
        keyword_words.update(language.builtins)
    keyword_words.update({"true", "false", "and", "or", "not"})
    type_words = set(language.core_types.get("signed_integers", ())) | set(language.core_types.get("unsigned_integers", ())) if language else set()
    if language:
        type_words.update(language.core_types.get("floating_point", ()))
        type_words.update(language.core_types.get("fixed_point", ()))
        type_words.update({"bool", "string", "vector", "matrix", "vec2", "vec3", "vec4", "Option", "Result", "Bytes", "Utf8", "Received"})
    module_words = set(catalogue.dsl_descriptors) if catalogue else set()
    math_words = set(catalogue.math_functions) if catalogue else set()
    unit_words = set(catalogue.units) if catalogue else set()
    project_keywords = {"project", "target", "numeric", "contract", "implementation"}
    type_keywords = {"record", "shared_record", "persistent_record", "enum", "variant", "flags", "fixed", "case"}
    hardware_keywords = {"peripheral", "bus", "sensor", "actuator", "fpga", "plc", "watchdog", "supervisor", "fault", "log"}
    algorithm_keywords = {"algorithm", "custom", "hybrid", "protocol", "subsystem", "model", "state_machine"}
    runtime_keywords = {"operatingsystem", "runtime", "loop", "taskset", "scheduler", "task", "interrupt", "channel", "resource", "startup"}
    binding_keywords = {"input", "output", "parameter", "local", "signal", "field", "state", "let", "stage", "use", "as"}
    flow_keywords = {"if", "else", "for", "in", "when", "otherwise", "transition", "run", "initialize", "predict", "update", "step", "choose", "on_failure", "after"}
    declaration_categories = {
        **{name: "projectKeyword" for name in project_keywords},
        **{name: "typeKeyword" for name in type_keywords},
        **{name: "hardwareKeyword" for name in hardware_keywords},
        **{name: "algorithmKeyword" for name in algorithm_keywords},
        **{name: "runtimeKeyword" for name in runtime_keywords},
        **{name: "bindingKeyword" for name in binding_keywords},
        **{name: "flowKeyword" for name in flow_keywords},
    }
    enum_members: set[str] = set()
    if catalogue:
        for definitions in catalogue.types.values():
            for item in definitions:
                if item.dsl_name:
                    type_words.add(item.dsl_name)
                type_words.add(item.c_name)
                enum_members.update(item.enum_members)
    if project:
        for collection in ("records", "enums", "variants", "flags", "fixed_point_types"):
            for item in getattr(project, collection, ()):
                type_words.add(item.name)
                enum_members.update(getattr(case, "name", "") for case in (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ())))
    if system:
        for collection in ("records", "enums", "variants", "flags_types", "fixed_point_types"):
            for item in getattr(system, collection, ()):
                type_words.add(item.name)
                enum_members.update(getattr(case, "name", "") for case in (*getattr(item, "variants", ()), *getattr(item, "cases", ()), *getattr(item, "members", ())))
    project_routines: dict[str, set[str]] = {}
    if project is not None:
        for declaration in getattr(project, "declarations", ()):
            if getattr(declaration, "kind", None) == "project_routine":
                owner = getattr(declaration, "registry_key", None)
                name = getattr(declaration, "name", None)
                if owner and name:
                    project_routines.setdefault(owner, set()).add(name)
    declaration_types: dict[tuple[int, int], str] = {}
    for declared_name, span, kind in _local_declarations(project, system):
        offsets = _name_offsets(declared_name, span, source)
        if offsets is None:
            continue
        if kind in {"system", "project"}:
            semantic_type = "projectName"
        elif kind in {"record", "enum", "variant", "flags", "fixed_point", "message", "fragment"}:
            semantic_type = "typeName"
        elif kind in {"sensor", "actuator", "peripheral", "bus", "watchdog", "supervisor", "fault", "log", "fpga", "plc", "node", "external_endpoint"}:
            semantic_type = "hardwareName"
        elif kind in {"operatingsystem", "taskset", "task", "loop", "interrupt", "channel", "resource", "startup", "delivery_policy"}:
            semantic_type = "runtimeName"
        elif kind in {"model", "state_machine", "subsystem", "custom_protocol", "protocol", "hybrid_system", "algorithm", "custom_algorithm", "hybrid_algorithm"}:
            semantic_type = "algorithmName"
        elif kind in {"run", "stage"}:
            semantic_type = "function"
        elif kind in {"parameter", "input", "output", "state", "field"}:
            semantic_type = "parameter" if kind == "parameter" else "recordField"
        elif kind == "member":
            semantic_type = "enumMember"
        else:
            semantic_type = "variable"
        declaration_types[offsets] = semantic_type
    token_index = {name: index for index, name in enumerate(_SEMANTIC_TOKEN_TYPES)}
    encoded: list[int] = []
    previous_line = 0
    previous_character = 0
    for index, token in enumerate(tokens):
        if token.kind.value == "eof" or token.kind.value == "newline":
            continue
        if token.kind.value == "string":
            token_type = "string"
        elif token.kind.value == "number":
            token_type = "number"
        elif token.kind.value in {"=", "==", "!=", "<", "<=", ">", ">=", "->", "..", "@", ";", "|", "&", "+", "-", "*", "**", "/", "^"}:
            token_type = "operator"
        elif token.kind.value == "identifier":
            name = token.text
            prior = tokens[index - 1] if index else None
            following = tokens[index + 1] if index + 1 < len(tokens) else None
            token_offsets = (token.span.start.offset, token.span.end.offset)
            if token_offsets in declaration_types:
                token_type = declaration_types[token_offsets]
            elif name in declaration_categories:
                token_type = declaration_categories[name]
            elif name in keyword_words:
                token_type = "keyword"
            elif name in type_words:
                token_type = "type"
            elif name in module_words:
                token_type = "class"
            elif name in math_words or (following and following.kind.value == "("):
                token_type = "function"
            elif name in unit_words:
                token_type = "unit"
            elif prior and prior.kind.value == ".":
                receiver = tokens[index - 2] if index >= 2 else None
                is_project_routine = (
                    receiver is not None
                    and receiver.kind.value == "identifier"
                    and name in project_routines.get(receiver.text, ())
                )
                token_type = "function" if is_project_routine else "enumMember" if name in enum_members else "property"
            elif following and following.kind.value == "=":
                token_type = "property"
            else:
                token_type = "variable"
        else:
            continue
        line_index = token.span.start.line - 1
        line = lines[line_index] if line_index < len(lines) else ""
        start_character = _column_to_utf16(line, token.span.start.column)
        length = len(token.text.encode("utf-16le")) // 2
        delta_line = line_index - previous_line
        delta_character = start_character - previous_character if delta_line == 0 else start_character
        encoded.extend((delta_line, delta_character, length, token_index[token_type], 0))
        previous_line = line_index
        previous_character = start_character
    return encoded

@server.feature(types.TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL, _SEMANTIC_TOKEN_LEGEND)
async def semantic_tokens(_ls: LanguageServer, params: types.SemanticTokensParams) -> types.SemanticTokens:
    """Classify symbols off the event loop so large files do not stall LSP requests."""
    document = server.workspace.get_text_document(params.text_document.uri)
    data = await asyncio.to_thread(_semantic_tokens_for_document, params.text_document.uri, document.source)
    return types.SemanticTokens(data=data)

def _formatted_source(source: str, tab_size: int, insert_spaces: bool, trim_trailing: bool) -> str:
    """Apply deterministic brace indentation while preserving token spelling and line endings."""
    path = None
    tokens, _ = Lexer(source, path).tokenize()
    source_lines = source.splitlines()
    newline = "\r\n" if "\r\n" in source else "\n"
    by_line: dict[int, list[object]] = {}
    for token in tokens:
        by_line.setdefault(token.span.start.line - 1, []).append(token)
    indentation = " " * max(1, tab_size) if insert_spaces else "\t"
    depth = 0
    rendered: list[str] = []
    for line_number, original in enumerate(source_lines):
        stripped = original.strip()
        line_tokens = [item for item in by_line.get(line_number, ()) if item.kind.value != "newline"]
        leading_closes = 0
        for item in line_tokens:
            if item.kind.value == "}":
                leading_closes += 1
            else:
                break
        prefix = indentation * max(0, depth - leading_closes) if stripped else ""
        content = stripped if stripped else ""
        if not trim_trailing:
            content = original.lstrip(" \t")
        rendered.append(prefix + content)
        for item in line_tokens:
            if item.kind.value == "{":
                depth += 1
            elif item.kind.value == "}":
                depth = max(0, depth - 1)
    formatted = newline.join(rendered)
    return formatted

def _formatting_edit(source: str, tab_size: int, insert_spaces: bool, trim_trailing: bool,
                     insert_final_newline: bool | None, trim_final_newlines: bool | None,
                     start_line: int | None = None, end_line: int | None = None) -> types.TextEdit | None:
    """Format one complete document or selected line interval with safe indentation."""
    lines = source.splitlines()
    if not lines:
        return None
    formatted = _formatted_source(source, tab_size, insert_spaces, trim_trailing)
    formatted_lines = formatted.splitlines()
    if len(formatted_lines) < len(lines):
        formatted_lines.extend([""] * (len(lines) - len(formatted_lines)))
    first = max(0, start_line or 0)
    last = min(len(lines) - 1, end_line if end_line is not None else len(lines) - 1)
    if first > last:
        return None
    if start_line is None:
        output = formatted
        if trim_final_newlines:
            output = output.rstrip("\r\n")
        if insert_final_newline:
            output = output.rstrip("\r\n") + ("\r\n" if "\r\n" in source else "\n")
        if output == source:
            return None
        end_line_index = len(lines) - 1
        end_char = len(lines[-1].encode("utf-16le")) // 2
        return types.TextEdit(range=types.Range(start=types.Position(0, 0), end=types.Position(end_line_index, end_char)), new_text=output.rstrip("\r\n") if source.endswith(("\n", "\r")) else output)
    newline = "\r\n" if "\r\n" in source else "\n"
    old_text = newline.join(lines[first:last + 1])
    new_text = newline.join(formatted_lines[first:last + 1])
    if old_text == new_text:
        return None
    return types.TextEdit(range=types.Range(start=types.Position(first, 0),
                                            end=types.Position(last, len(lines[last].encode("utf-16le")) // 2)), new_text=new_text)

@server.feature(types.TEXT_DOCUMENT_FORMATTING, types.DocumentFormattingOptions())
def formatting(_ls: LanguageServer, params: types.DocumentFormattingParams) -> list[types.TextEdit]:
    """Format brace indentation and trailing whitespace without rewriting expressions."""
    document = server.workspace.get_text_document(params.text_document.uri)
    edit = _formatting_edit(document.source, params.options.tab_size, params.options.insert_spaces,
                            params.options.trim_trailing_whitespace is not False,
                            params.options.insert_final_newline, params.options.trim_final_newlines)
    return [edit] if edit else []

@server.feature(types.TEXT_DOCUMENT_RANGE_FORMATTING, types.DocumentRangeFormattingOptions())
def range_formatting(_ls: LanguageServer, params: types.DocumentRangeFormattingParams) -> list[types.TextEdit]:
    """Format only selected lines while calculating indentation from the whole file."""
    document = server.workspace.get_text_document(params.text_document.uri)
    edit = _formatting_edit(document.source, params.options.tab_size, params.options.insert_spaces,
                            params.options.trim_trailing_whitespace is not False, None, None,
                            params.range.start.line, params.range.end.line)
    return [edit] if edit else []

@server.feature(types.TEXT_DOCUMENT_FOLDING_RANGE, types.FoldingRangeOptions())
def folding_ranges(_ls: LanguageServer, params: types.FoldingRangeParams) -> list[types.FoldingRange]:
    """Fold brace-delimited declarations and nested blocks."""
    document = server.workspace.get_text_document(params.text_document.uri)
    path = _path_from_uri(params.text_document.uri)
    tokens, _ = Lexer(document.source, path).tokenize()
    stack: list[object] = []
    folds: list[types.FoldingRange] = []
    for token in tokens:
        if token.kind.value == "{":
            stack.append(token)
        elif token.kind.value == "}" and stack:
            opening = stack.pop()
            start_line = opening.span.start.line - 1
            end_line = token.span.start.line - 1
            if end_line > start_line:
                folds.append(types.FoldingRange(start_line=start_line, end_line=end_line, kind=types.FoldingRangeKind.Region))
    return sorted(folds, key=lambda item: (item.start_line, -item.end_line))

@server.feature(types.TEXT_DOCUMENT_CODE_ACTION, types.CodeActionOptions(code_action_kinds=[types.CodeActionKind.QuickFix]))
def code_actions(_ls: LanguageServer, params: types.CodeActionParams) -> list[types.CodeAction]:
    """Offer safe source fixes for known UEF-PL diagnostics."""
    document = server.workspace.get_text_document(params.text_document.uri)
    actions: list[types.CodeAction] = []
    catalogue = _catalogue_for_document(params.text_document.uri)
    if catalogue is None:
        return actions
    path = _path_from_uri(params.text_document.uri)
    tokens, _ = Lexer(document.source, path).tokenize()
    for diagnostic in params.context.diagnostics:
        code = getattr(diagnostic.code, "value", diagnostic.code)
        if code != "uetd-umath-c-name-hidden":
            continue
        start_offset = _offset_from_lsp(document.source, diagnostic.range.start)
        end_offset = _offset_from_lsp(document.source, diagnostic.range.end)
        raw_tokens = [token for token in tokens
                      if token.kind.value == "identifier"
                      and token.text in catalogue.math_functions_by_c_name
                      and start_offset <= token.span.start.offset
                      and token.span.end.offset <= end_offset]
        if len(raw_tokens) != 1:
            continue
        raw_token = raw_tokens[0]
        raw_name = raw_token.text
        contract = catalogue.math_functions_by_c_name[raw_name]
        location = _location_for_span(raw_token.span, path, source=document.source)
        if location is None:
            continue
        edit = types.WorkspaceEdit(changes={params.text_document.uri: [types.TextEdit(range=location.range, new_text=contract.dsl_name)]})
        actions.append(types.CodeAction(title=f"Use registered UEF-PL name `{contract.dsl_name}`", kind=types.CodeActionKind.QuickFix,
                                        diagnostics=[diagnostic], is_preferred=True, edit=edit))
    return actions

def main(argv: list[str] | None = None) -> None:
    """Start the shared stdio LSP server for editor clients."""
    global _explicit_uetd_root
    parser = argparse.ArgumentParser(prog="uef-gen-language-server")
    parser.add_argument("--uetd-root", type=Path, help="explicit UETD checkout root")
    parser.add_argument("--log-file", type=Path, help="write language-server diagnostics to this file (default: per-user UEF-PL log)")
    parser.add_argument("--log-level", choices=("DEBUG", "INFO", "WARNING", "ERROR"), help="log detail level; DEBUG includes every compiler diagnostic")
    args = parser.parse_args(argv)
    _explicit_uetd_root = args.uetd_root.expanduser().resolve() if args.uetd_root else None
    _configure_logging(args.log_file, args.log_level)
    _LOGGER.info("Language server configured uetd_root=%s", _explicit_uetd_root or "auto-discovery")
    server.start_io()
