"""Recursive-descent parser for the declarative multi-target ``.uefs`` DSL."""

from __future__ import annotations

from pathlib import Path
from typing import Any

from uef_gen.language.frontends.project.ast import EnumDefinition, FixedPointDefinition, FlagsDefinition, Property, RecordDefinition, Reference, TypeSpec, VariantDefinition
from uef_gen.language.frontends.project.parser import Parser, _ParseFailure
from uef_gen.language.frontends.project.tokens import Token, TokenKind
from uef_gen.language.frontends.system.ast import (
    ExternalEndpointSpec,
    HybridSystemSpec,
    SystemDocument,
    SystemLinkSpec,
    SystemNodeSpec,
    SystemProtocolSpec,
    SystemRecordSpec,
)


_SYSTEM_PRECEDENCE = {"or": 1, "and": 2, "==": 3, "!=": 3, "<": 3, "<=": 3, ">": 3, ">=": 3, "+": 4, "-": 4, "*": 5, "/": 5}


class SystemParser(Parser):
    """Parse system contracts using the shared token/span and expression AST."""

    def __init__(self, tokens: list[Token], path: Path | None = None) -> None:
        super().__init__(tokens, path)

    def parse(self) -> tuple[SystemDocument | None, list]:
        self._skip_newlines()
        try:
            document = self._parse_system()
        except _ParseFailure:
            document = None
        return document, self.diagnostics

    def _parse_system(self) -> SystemDocument:
        start = self._expect_keyword("system")
        name = self._expect_identifier("system name")
        self._expect(TokenKind.LBRACE, "'{' after system name")
        nodes: list[SystemNodeSpec] = []
        endpoints: list[ExternalEndpointSpec] = []
        records: list[SystemRecordSpec] = []
        enums: list[EnumDefinition] = []
        protocols: list[SystemProtocolSpec] = []
        links: list[SystemLinkSpec] = []
        hybrids: list[HybridSystemSpec] = []
        delivery: list[dict[str, Any]] = []
        variants: list[VariantDefinition] = []
        flags: list[FlagsDefinition] = []
        fixed_types: list[FixedPointDefinition] = []
        names: set[tuple[str, str]] = set()
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                word = self._word()
                if word == "node":
                    item = self._parse_node(); self._unique(names, "endpoint", item.name, item.source_span); nodes.append(item)
                elif word == "external":
                    item = self._parse_external_endpoint(); self._unique(names, "endpoint", item.name, item.source_span); endpoints.append(item)
                elif word == "record":
                    record = self._parse_shared_record(); self._unique(names, "type", record.name, record.source_span); records.append(record)
                elif word == "enum":
                    item = self._parse_enum(); self._unique(names, "type", item.name, item.span); enums.append(item)
                elif word == "variant":
                    item = self._parse_variant(); self._unique(names, "type", item.name, item.span); variants.append(item)
                elif word == "flags":
                    item = self._parse_flags(); self._unique(names, "type", item.name, item.span); flags.append(item)
                elif word == "fixed":
                    item = self._parse_fixed_type(); self._unique(names, "type", item.name, item.span); fixed_types.append(item)
                elif word == "custom":
                    item = self._parse_protocol(); self._unique(names, "protocol", item.name, item.source_span); protocols.append(item)
                elif word == "link":
                    item = self._parse_link(); self._unique(names, "link", item.name, item.source_span); links.append(item)
                elif word == "hybrid":
                    item = self._parse_hybrid_system(); self._unique(names, "hybrid system", item.name, item.source_span); hybrids.append(item)
                elif word == "delivery_policy":
                    item = self._parse_delivery_policy(); self._unique(names, "delivery policy", item["name"], item.get("source_span")); delivery.append(item)
                elif word in {"shared_record", "persistent_record"}:
                    self._error("unsupported_system_type_form", f"The .uefs top-level form {word!r} is not declared by the active system grammar.", self._current().span)
                    self._synchronize()
                else:
                    self._fail("unknown_system_declaration", f"Unknown system declaration {word!r}.", self._current().span)
            except _ParseFailure:
                self._synchronize()
            if mark == self.index and not self._at(TokenKind.RBRACE, TokenKind.EOF): self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after system declarations")
        if not self._at(TokenKind.EOF): self._error("trailing_source", "Unexpected content after system declaration.", self._current().span)
        if not nodes: self._error("system_has_no_nodes", "A system must declare at least one node project.", start.span)
        return SystemDocument(
            name.text,
            tuple(nodes),
            tuple(endpoints),
            tuple(records),
            tuple(enums),
            tuple(protocols),
            tuple(links),
            tuple(hybrids),
            tuple(delivery),
            self.path,
            self._join(start.span, end.span),
            tuple(variants),
            tuple(flags),
            tuple(fixed_types),
        )

    def _parse_node(self) -> SystemNodeSpec:
        start = self._expect_keyword("node"); name = self._expect_identifier("node name"); fields = self._parse_named_property_block()
        unknown = set(fields) - {"project", "role"}
        for key in sorted(unknown): self._error("unknown_node_field", f"Node field {key!r} is not supported.", fields[key][1])
        project = self._as_string(fields.get("project", (None, None))[0])
        role = self._as_name(fields.get("role", (None, None))[0])
        if not project: self._error("missing_node_project", "A node requires project = \"relative/path.uef\".", self._join(start.span, self._previous().span))
        elif not project.endswith(".uef"): self._error("invalid_node_project_suffix", "Node project paths must end in .uef.", fields["project"][1])
        if not role: self._error("missing_node_role", "A node requires a role identifier.", self._join(start.span, self._previous().span))
        return SystemNodeSpec(name.text, Path(project or ""), role or "", self._join(start.span, self._previous().span))

    def _parse_external_endpoint(self) -> ExternalEndpointSpec:
        start = self._expect_keyword("external"); name = self._expect_identifier("external endpoint name"); fields = self._parse_named_property_block()
        for required in ("implementation", "role"):
            if required not in fields: self._error("missing_external_field", f"External endpoint requires {required} = ...;", self._join(start.span, self._previous().span))
        implementation = self._as_name(fields.get("implementation", (None, None))[0]) or ""
        role = self._as_name(fields.get("role", (None, None))[0]) or ""
        attached = self._as_reference(fields.get("attached_to", (None, None))[0])
        if implementation != "external_rtl": self._error("invalid_external_implementation", "External endpoint implementation must be external_rtl.", fields.get("implementation", (None, start.span))[1])
        props = {key: value for key, (value, _) in fields.items() if key not in {"implementation", "role", "attached_to"}}
        return ExternalEndpointSpec(name.text, implementation, role, attached, self._join(start.span, self._previous().span), props)

    def _parse_shared_record(self) -> SystemRecordSpec:
        node = self._parse_record()
        fields = tuple((field.name, field.type_spec or TypeSpec("<missing>"), {"source_span": field.span}) for field in node.fields)
        return SystemRecordSpec(node.name, fields, node.span, node.boundary)

    def _parse_record(self) -> RecordDefinition:
        return super()._parse_record()

    def _parse_variant(self) -> Any:
        return super()._parse_variant()

    def _parse_flags(self) -> Any:
        return super()._parse_flags()

    def _parse_fixed_type(self) -> Any:
        return super()._parse_fixed_type()

    def _parse_boundary_record(self, *, boundary_kind: str) -> Any:
        return super()._parse_boundary_record(boundary_kind=boundary_kind)

    def _parse_link(self) -> SystemLinkSpec:
        start = self._expect_keyword("link"); name = self._expect_identifier("link name"); fields = self._parse_named_property_block()
        for required in ("from", "to"):
            if required not in fields: self._error("missing_link_endpoint", f"Link requires {required} = endpoint;", self._join(start.span, self._previous().span))
        selectors = [key for key in ("contract", "protocol") if key in fields]
        if len(selectors) != 1: self._error("link_contract_selector", "A link must select exactly one contract or protocol.", self._join(start.span, self._previous().span))
        source = self._as_reference(fields.get("from", (None, None))[0]) or ""
        destination = self._as_reference(fields.get("to", (None, None))[0]) or ""
        contract = self._as_name(fields.get("contract", (None, None))[0])
        protocol = self._as_name(fields.get("protocol", (None, None))[0])
        excluded = {"from", "to", "contract", "protocol"}
        props = {key: value for key, (value, _) in fields.items() if key not in excluded}
        return SystemLinkSpec(name.text, source, destination, contract, protocol, props, self._join(start.span, self._previous().span))

    def _parse_enum(self) -> EnumDefinition:
        return super()._parse_enum()

    def _parse_hybrid_system(self) -> HybridSystemSpec:
        start = self._expect_keyword("hybrid"); self._expect_keyword("system"); name = self._expect_identifier("hybrid system name"); props = self._parse_property_block()
        return HybridSystemSpec(name.text, {prop.name: prop.value for prop in props}, self._join(start.span, self._previous().span))

    def _parse_delivery_policy(self) -> dict[str, Any]:
        start = self._expect_keyword("delivery_policy"); name = self._expect_identifier("delivery policy name"); props = self._parse_property_block()
        return {"name": name.text, "properties": {prop.name: prop.value for prop in props}, "source_span": self._join(start.span, self._previous().span)}

    def _parse_protocol(self) -> SystemProtocolSpec:
        start = self._expect_keyword("custom"); self._expect_keyword("protocol"); name = self._expect_identifier("protocol name"); self._expect(TokenKind.LBRACE, "'{' before protocol declaration")
        properties: dict[str, Any] = {}; blocks: dict[str, dict[str, Any]] = {}; messages: list[dict[str, Any]] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                word = self._word()
                if word in {"framing", "header", "sequence", "payload", "checksum", "receiver", "behavior"}:
                    block_name = self._advance().text
                    blocks[block_name] = self._parse_property_map()
                elif word == "message": messages.append(self._parse_message())
                elif word == "state_machine": blocks["state_machine"] = self._parse_protocol_state_machine()
                else:
                    prop = self._parse_property(); properties[prop.name] = prop.value
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after protocol")
        transport = self._as_name(properties.get("transport"))
        return SystemProtocolSpec(name.text, transport, blocks.get("framing", {}), blocks.get("header", {}), blocks.get("sequence", {}), blocks.get("checksum", {}), tuple(messages), blocks.get("receiver", {}), {**properties, **{f"{k}_properties": v for k, v in blocks.items() if k not in {"framing", "header", "sequence", "checksum", "receiver"}}}, self._join(start.span, end.span))

    def _parse_message(self) -> dict[str, Any]:
        start = self._expect_keyword("message"); name = self._expect_identifier("message name"); self._expect(TokenKind.LBRACE, "'{' before message fields")
        message_id = None; fields: list[dict[str, Any]] = []; fragments: list[dict[str, Any]] = []; props: dict[str, Any] = {}
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                if self._word() == "fragment": fragments.append(self._parse_fragment())
                elif self._word() == "field": fields.append(self._parse_protocol_field())
                elif self._word() == "id":
                    self._advance(); self._expect(TokenKind.EQUAL, "'=' after id"); message_id = self._parse_integer_literal(); self._expect_semicolon()
                else:
                    prop = self._parse_property(); props[prop.name] = prop.value
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after message")
        return {"name": name.text, "id": message_id, "fields": tuple(fields), "fragments": tuple(fragments), "properties": props, "source_span": self._join(start.span, end.span)}

    def _parse_protocol_field(self) -> dict[str, Any]:
        start = self._expect_keyword("field"); name = self._expect_identifier("protocol field name"); self._expect(TokenKind.COLON, "':' before protocol field type"); type_spec = self._parse_type_ref(); props = self._parse_property_map()
        return {"name": name.text, "type": type_spec, "properties": props, "source_span": self._join(start.span, self._previous().span)}

    def _parse_fragment(self) -> dict[str, Any]:
        start = self._expect_keyword("fragment"); index = self._parse_integer_literal(); self._expect(TokenKind.LBRACE, "'{' before fragment fields"); fields: list[dict[str, Any]] = []; props: dict[str, Any] = {}
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                if self._word() == "field": fields.append(self._parse_protocol_field())
                else:
                    prop = self._parse_property(); props[prop.name] = prop.value
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after fragment")
        return {"index": index, "fields": tuple(fields), "properties": props, "source_span": self._join(start.span, end.span)}

    def _parse_protocol_state_machine(self) -> dict[str, Any]:
        start = self._expect_keyword("state_machine"); self._expect(TokenKind.LBRACE, "'{' before protocol states"); states: list[str] = []; transitions: list[dict[str, Any]] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            if self._match_keyword("state"):
                states.append(self._expect_identifier("state name").text); self._expect_semicolon()
            elif self._match_keyword("on"):
                event = self._expect_identifier("event name"); self._expect_keyword("from"); source = self._expect_identifier("source state"); self._expect(TokenKind.ARROW, "'->'"); destination = self._expect_identifier("destination state"); self._expect_semicolon()
                transitions.append({"event": event.text, "from": source.text, "to": destination.text})
            else: self._fail("invalid_protocol_state", "Expected state or on transition.", self._current().span)
        end = self._expect(TokenKind.RBRACE, "'}' after protocol state machine")
        return {"states": tuple(states), "transitions": tuple(transitions), "source_span": self._join(start.span, end.span)}

    def _parse_named_property_block(self) -> dict[str, tuple[Any, Any]]:
        self._expect(TokenKind.LBRACE, "'{' before fields"); fields: dict[str, tuple[Any, Any]] = {}
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                prop = self._parse_property()
                if prop.name in fields: self._error("duplicate_property", f"Property {prop.name!r} appears more than once.", prop.span)
                else: fields[prop.name] = (prop.value, prop.span)
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        self._expect(TokenKind.RBRACE, "'}' after fields")
        return fields

    def _parse_property_map(self) -> dict[str, Any]:
        props = self._parse_property_block()
        result: dict[str, Any] = {}
        for prop in props:
            if prop.name in result: self._error("duplicate_property", f"Property {prop.name!r} appears more than once.", prop.span)
            result[prop.name] = prop.value
        return result

    def _parse_property(self) -> Property:
        start = self._parse_reference(); self._expect(TokenKind.EQUAL, "'=' in property"); value = self._parse_expression(); self._expect_semicolon()
        return Property(".".join(start.path), value, self._join(start.span, self._previous().span))

    def _parse_type_ref(self) -> TypeSpec:
        type_spec = super()._parse_type_spec()
        if type_spec.name in {"Received", "Option", "Result", "Bytes", "Utf8"}:
            self._error("unsupported_system_type", f"Type constructor {type_spec.name!r} is not allowed in a .uefs contract type.", type_spec.span)
        return type_spec

    def _parse_expression(self, minimum_precedence: int = 0):
        left = self._parse_system_primary()
        while True:
            operator = self._current().text if self._current().kind in {TokenKind.PLUS, TokenKind.MINUS, TokenKind.STAR, TokenKind.SLASH, TokenKind.EQUAL_EQUAL, TokenKind.NOT_EQUAL, TokenKind.LESS, TokenKind.LESS_EQUAL, TokenKind.GREATER, TokenKind.GREATER_EQUAL} else self._word()
            precedence = _SYSTEM_PRECEDENCE.get(operator, -1)
            if precedence < minimum_precedence: break
            self._advance(); right = self._parse_expression(precedence + 1)
            from uef_gen.language.frontends.project.ast import BinaryExpression
            left = BinaryExpression(operator, left, right, self._join(left.span, right.span))
            if precedence == 3 and self._current().text in {"==", "!=", "<", "<=", ">", ">="}:
                self._error("chained_comparison", "Comparisons in .uefs are non-associative.", self._current().span)
        return left

    def _parse_system_primary(self):
        token = self._current()
        if token.kind in {TokenKind.PLUS, TokenKind.MINUS} or self._word() == "not":
            self._advance(); value = self._parse_system_primary()
            from uef_gen.language.frontends.project.ast import UnaryExpression
            return UnaryExpression(token.text, value, self._join(token.span, value.span))
        if token.kind == TokenKind.IDENTIFIER and token.text in {"map", "select"}:
            return self._parse_dsl_builtin_call(token.text)
        if token.kind == TokenKind.LBRACKET:
            return self._parse_matrix_or_vector()
        if token.kind == TokenKind.LPAREN:
            self._advance(); value = self._parse_expression(); self._expect(TokenKind.RPAREN, "')'"); return value
        if token.kind == TokenKind.IDENTIFIER and token.text not in {"true", "false"}:
            ref = self._parse_reference()
            if self._at(TokenKind.LPAREN):
                self._fail("calls_forbidden_in_system", "Function and component calls are not allowed in .uefs expressions.", self._current().span)
            if self._match(TokenKind.LBRACKET):
                subs = self._parse_subscripts(TokenKind.RBRACKET); end = self._expect(TokenKind.RBRACKET, "']' after index")
                from uef_gen.language.frontends.project.ast import IndexExpression
                return IndexExpression(ref, tuple(subs), self._join(ref.span, end.span))
            return ref
        return self._parse_primary()

    def _parse_vector_literal(self) -> Any:
        return self._parse_matrix_or_vector()

    def _parse_map_literal(self) -> Any:
        return self._parse_dsl_builtin_call("map")

    def _parse_select_expression(self) -> Any:
        return self._parse_dsl_builtin_call("select")

    def _unique(self, names: set[tuple[str, str]], category: str, name: str, span: Any) -> None:
        key = (category, name)
        if key in names: self._error("duplicate_system_name", f"Duplicate {category} name {name!r}.", span)
        names.add(key)

    def _as_name(self, value: Any) -> str | None:
        if hasattr(value, "value") and isinstance(value.value, str): return value.value
        if isinstance(value, Reference) and len(value.path) == 1: return value.path[0]
        return None

    def _as_string(self, value: Any) -> str | None:
        return value.value if hasattr(value, "value") and isinstance(value.value, str) else None

    def _as_reference(self, value: Any) -> str | None:
        if isinstance(value, Reference): return ".".join(value.path)
        return self._as_name(value)
