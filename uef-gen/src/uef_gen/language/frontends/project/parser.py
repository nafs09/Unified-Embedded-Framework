"""Deterministic recursive-descent parser for standalone ``.uef`` files."""

from __future__ import annotations
from pathlib import Path
from uef_gen.language.frontends.project.ast import (AlgorithmStage, BinaryExpression, BoundedLoop, CastExpression, CompositionLocal, CompositionOutput, CompositionStage,
                                          ConditionalStatement, ContractImport, Declaration, EnumDefinition, EnumVariant, Equation, FixedPointDefinition,
                                          FlagsDefinition, FlagsMember, Identifier, IndexExpression, IndexSubscript, ListValue, Literal, MatchArm, MatchExpression,
                                          MatrixLiteral, MathCallExpression, Property, ProjectDocument, RecordBoundary, RecordDefinition, RecordValue, Reference,
                                          SelectValue, SourcePosition, SourceSpan, TypeSpec, TypedBinding, UnaryExpression, UnitConversionExpression, VariantCaseDefinition,
                                          VariantConstructorExpression, VariantDefinition)
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic, source_error
from uef_gen.language.frontends.project.tokens import Token, TokenKind

_INTEGER_TYPES = {"u8", "u16", "u32", "u64", "i8", "i16", "i32", "i64"}
_ROLE_KEYWORDS = {"peripheral", "bus", "sensor", "actuator", "algorithm", "protocol"}
_PROJECT_TYPES = {"record", "enum", "variant", "flags", "fixed", "shared_record", "persistent_record"}
_PRECEDENCE = {
    "or": 1,
    "and": 2,
    "|": 3,
    "^": 4,
    "&": 5,
    "==": 6,
    "!=": 6,
    "<": 6,
    "<=": 6,
    ">": 6,
    ">=": 6,
    "+": 7,
    "-": 7,
    "*": 8,
    "/": 8,
    "**": 9,
}

class _ParseFailure(Exception):
    """Internal signal used to recover at the enclosing declaration."""

class Parser:
    """Parse UEF-PL tokens into a source-located AST without resolving names."""

    def __init__(self, tokens: list[Token], path: Path | None = None) -> None:
        self.tokens = tokens
        self.path = path
        self.index = 0
        self.diagnostics: list[LanguageDiagnostic] = []
        self._signatures: dict[str, Declaration] = {}
        self._project_routines: set[str] = set()
        self._implemented: set[str] = set()

    def parse(self) -> tuple[ProjectDocument | None, list[LanguageDiagnostic]]:
        self._skip_newlines()
        try:
            document = self._parse_project()
        except _ParseFailure:
            document = None
        return document, self.diagnostics

    def _parse_project(self) -> ProjectDocument:
        start = self._expect_keyword("project")
        name = self._expect_identifier("project name")
        self._expect(TokenKind.LBRACE, "'{' after project name")
        declarations: list[Declaration] = []
        parameters: list[TypedBinding] = []
        records: list[RecordDefinition] = []
        enums: list[EnumDefinition] = []
        variants: list[VariantDefinition] = []
        flags: list[FlagsDefinition] = []
        fixed: list[FixedPointDefinition] = []
        imports: list[ContractImport] = []
        seen: set[str] = set()
        runtime_seen = False
        implementation_seen = False

        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            before = self.index
            try:
                word = self._word()
                if word == "target":
                    self._claim_section(seen, "target")
                    declarations.append(self._parse_target())
                elif word == "contract":
                    imports.append(self._parse_contract_import())
                elif word == "numeric":
                    self._claim_section(seen, "numeric")
                    declarations.append(self._parse_numeric())
                elif word == "parameter":
                    self._require_before_runtime(runtime_seen, "project parameter")
                    parameters.append(self._parse_project_parameter())
                elif word in _PROJECT_TYPES:
                    self._require_before_runtime(runtime_seen, "project type")
                    item = self._parse_project_type()
                    declarations.append(item[0])
                    if isinstance(item[1], RecordDefinition): records.append(item[1])
                    elif isinstance(item[1], EnumDefinition): enums.append(item[1])
                    elif isinstance(item[1], VariantDefinition): variants.append(item[1])
                    elif isinstance(item[1], FlagsDefinition): flags.append(item[1])
                    elif isinstance(item[1], FixedPointDefinition): fixed.append(item[1])
                elif word == "project" and self._peek_text(1) == "routine":
                    self._require_before_runtime(runtime_seen, "project routine")
                    routine = self._parse_project_routine_signature()
                    declarations.append(routine)
                    qualified_name = f"{routine.registry_key}.{routine.name}" if routine.registry_key else routine.name or ""
                    if qualified_name in self._project_routines:
                        self._error("duplicate_project_routine", f"Project routine {qualified_name!r} is declared more than once.", routine.span)
                    self._project_routines.add(qualified_name)
                elif word == "operatingsystem":
                    if runtime_seen:
                        self._error("duplicate_operatingsystem", "A project may declare only one operatingsystem block.", self._current().span)
                    runtime_seen = True
                    declarations.append(self._parse_operatingsystem())
                elif word == "implementation":
                    if not runtime_seen:
                        self._error("implementation_before_operatingsystem", "The implementation block must follow operatingsystem.", self._current().span)
                    if implementation_seen:
                        self._error("duplicate_implementation", "A project may declare only one implementation block.", self._current().span)
                    implementation_seen = True
                    declarations.append(self._parse_implementation_block())
                else:
                    self._require_before_runtime(runtime_seen, "component signature")
                    signature = self._parse_constructor_signature()
                    declarations.append(signature)
                    if signature.name:
                        if signature.name in self._signatures:
                            self._error("duplicate_component_signature", f"Component {signature.name!r} is declared more than once.", signature.span)
                        else:
                            self._signatures[signature.name] = signature
            except _ParseFailure:
                self._synchronize()
            if self.index == before and not self._at(TokenKind.RBRACE, TokenKind.EOF):
                self._advance()

        end = self._expect(TokenKind.RBRACE, "'}' after project body")
        if not self._at(TokenKind.EOF):
            self._error("trailing_source", "Unexpected content after the project declaration.", self._current().span)
        if "target" not in seen:
            self._error("missing_target", "A project must declare exactly one target.", start.span)
        if not runtime_seen:
            self._error("missing_operatingsystem", "A project must declare one operatingsystem block.", end.span)
        if not implementation_seen:
            self._error("missing_implementation", "A project must declare one implementation block after operatingsystem.", end.span)
        # Inline machine states are the source of truth. Materialize their
        # nominal enum type in the semantic AST so typed component ports can
        # use the machine's state type without a second, parallel enum block.
        existing_enum_names = {item.name for item in enums}
        def visit_state_machines(items: tuple[Declaration, ...] | list[Declaration]) -> None:
            for declaration in items:
                if declaration.kind == "state_machine" and declaration.registry_key:
                    states = [child for child in declaration.children if child.kind == "machine_state"]
                    if states:
                        if declaration.registry_key in existing_enum_names:
                            self._error(
                                "state_machine_duplicate_state_type",
                                f"State type {declaration.registry_key!r} is declared by both an enum and inline machine states; remove the separate enum.",
                                declaration.span,
                            )
                        else:
                            variants = []
                            for child in states:
                                tag = next((prop.value.value for prop in child.properties
                                            if prop.name == "tag" and isinstance(prop.value, Literal)
                                            and isinstance(prop.value.value, int)), None)
                                variants.append(EnumVariant(child.name or "", tag, child.span))
                            enums.append(EnumDefinition(declaration.registry_key, "u8", tuple(variants), declaration.span))
                            existing_enum_names.add(declaration.registry_key)
                visit_state_machines(declaration.children)
        visit_state_machines(declarations)
        span = self._join(start.span, end.span)
        return ProjectDocument(
            name=name.text,
            declarations=tuple(declarations),
            span=span,
            source_path=self.path,
            parameters=tuple(parameters),
            enums=tuple(enums),
            records=tuple(records),
            variants=tuple(variants),
            flags=tuple(flags),
            fixed_point_types=tuple(fixed),
            contract_imports=tuple(imports),
        )

    def _parse_project_routine_signature(self) -> Declaration:
        """Declare a zero-argument project callback with an explicit ABI result."""
        start = self._expect_keyword("project")
        self._expect_keyword("routine")
        reference = self._parse_reference()
        if len(reference.path) not in {1, 2}:
            self._error("project_routine_name_invalid", "A project routine is a name or component.operation; deeper paths are not callable routines.", reference.span)
        self._expect(TokenKind.LPAREN, "'(' after project routine name")
        self._expect(TokenKind.RPAREN, "')' after the zero-argument project routine signature")
        return_type = "void"
        properties: tuple[Property, ...] = ()
        if self._match_keyword("returns"):
            result = self._expect_identifier("project routine result type")
            if result.text not in {"void", "status"}:
                self._error("project_routine_result_invalid", "Project routines return either void or UETD status; state-machine hooks require 'returns status'.", result.span)
            return_type = result.text
            properties = (Property("return_type", Identifier(result.text, result.span), result.span),)
        if self._match_keyword("context"):
            self._expect(TokenKind.EQUAL, "'=' after project routine context")
            context = self._expect_identifier("project routine execution context")
            context_span = self._join(self._previous().span, context.span)
            properties = (*properties, Property("context", Identifier(context.text, context.span), context_span))
        self._expect_semicolon()
        owner = reference.path[0] if len(reference.path) == 2 else None
        name = reference.path[-1] if reference.path else ""
        return Declaration("project_routine", name, owner, f"{return_type}()", properties=properties,
                           span=self._join(start.span, self._previous().span), prototype=True)

    def _parse_target(self) -> Declaration:
        start = self._advance()
        self._expect(TokenKind.EQUAL, "'=' after target")
        value = self._parse_primary()
        self._expect_semicolon()
        return Declaration("target", None, None, properties=(Property("value", value, self._join(start.span, self._previous().span)),), span=self._join(start.span, self._previous().span))

    def _parse_numeric(self) -> Declaration:
        start = self._advance()
        props = self._parse_property_block()
        return Declaration("numeric", None, None, properties=tuple(props), span=self._join(start.span, self._previous().span))

    def _parse_contract_import(self) -> ContractImport:
        start = self._expect_keyword("contract")
        self._expect_keyword("import")
        value = self._expect(TokenKind.STRING, "quoted relative .uefs path")
        raw = value.text
        path_text = self._decode_string(raw)
        if not path_text or "\\" in path_text or ":" in path_text or path_text.startswith("/"):
            self._error("invalid_contract_import_path", "Contract imports must use a non-empty relative forward-slash path.", value.span)
        if not path_text.endswith(".uefs"):
            self._error("invalid_contract_import_suffix", "Contract import paths must end in .uefs.", value.span)
        if any(part in ("", ".", "..") for part in path_text.split("/")):
            self._error("invalid_contract_import_path", "Contract import paths cannot contain empty, '.' or '..' path segments.", value.span)
        self._expect_semicolon()
        return ContractImport(path_text, self._join(start.span, self._previous().span))

    def _parse_project_parameter(self) -> TypedBinding:
        start = self._expect_keyword("parameter")
        name = self._expect_identifier("parameter name")
        self._expect(TokenKind.COLON, "':' after parameter name")
        type_spec = self._parse_type_spec()
        value = None
        if self._match(TokenKind.EQUAL): value = self._parse_expression()
        self._expect_semicolon()
        return TypedBinding(name.text, type_spec, value, self._join(start.span, self._previous().span), "parameter")

    def _parse_project_type(self):
        word = self._word()
        if word == "record":
            node = self._parse_record()
            return Declaration("record", node.name, None, span=node.span), node
        if word == "enum":
            node = self._parse_enum()
            return Declaration("enum", node.name, None, span=node.span), node
        if word == "variant":
            node = self._parse_variant()
            return Declaration("variant", node.name, None, span=node.span), node
        if word == "flags":
            node = self._parse_flags()
            return Declaration("flags", node.name, None, span=node.span), node
        if word == "fixed":
            node = self._parse_fixed_type()
            return Declaration("fixed_point", node.name, None, span=node.span), node
        boundary_kind = "shared" if word == "shared_record" else "persistent"
        node = self._parse_boundary_record(boundary_kind=boundary_kind)
        return Declaration("record", node.name, None, span=node.span), node

    def _parse_record(self) -> RecordDefinition:
        start = self._expect_keyword("record")
        name = self._expect_identifier("record name")
        self._expect(TokenKind.LBRACE, "'{' after record name")
        fields: list[TypedBinding] = []
        names: set[str] = set()
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                field_name = self._expect_identifier("record field name")
                self._expect(TokenKind.COLON, "':' after field name")
                type_spec = self._parse_type_spec()
                self._expect_semicolon()
                if field_name.text in names: self._error("duplicate_record_field", f"Field {field_name.text!r} is duplicated.", field_name.span)
                names.add(field_name.text)
                fields.append(TypedBinding(field_name.text, type_spec, None, self._join(field_name.span, self._previous().span), "field"))
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after record")
        return RecordDefinition(name.text, tuple(fields), self._join(start.span, end.span))

    def _parse_variant(self) -> VariantDefinition:
        start = self._expect_keyword("variant")
        name = self._expect_identifier("variant name")
        self._expect(TokenKind.COLON, "':' before variant tag type")
        tag_type = self._expect_identifier("integer tag type").text
        if tag_type not in _INTEGER_TYPES: self._error("invalid_variant_tag_type", "Variant tags require a fixed-width integer type.", self._previous().span)
        self._expect(TokenKind.LBRACE, "'{' before variant cases")
        cases: list[VariantCaseDefinition] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                case_start = self._expect_keyword("case")
                case_name = self._expect_identifier("variant case name")
                tag = self._parse_integer_literal() if self._match(TokenKind.EQUAL) else None
                fields: list[TypedBinding] = []
                if self._match(TokenKind.LBRACE):
                    while not self._at(TokenKind.RBRACE, TokenKind.EOF):
                        field = self._expect_identifier("variant payload field")
                        self._expect(TokenKind.COLON, "':' after payload field")
                        type_spec = self._parse_type_spec()
                        self._expect_semicolon()
                        fields.append(TypedBinding(field.text, type_spec, None, self._join(field.span, self._previous().span), "field"))
                    self._expect(TokenKind.RBRACE, "'}' after variant payload")
                self._match(TokenKind.COMMA)
                self._match(TokenKind.SEMICOLON)
                cases.append(VariantCaseDefinition(case_name.text, tag, tuple(fields), self._join(case_start.span, self._previous().span)))
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after variant")
        return VariantDefinition(name.text, tag_type, tuple(cases), self._join(start.span, end.span))

    def _parse_flags(self) -> FlagsDefinition:
        start = self._expect_keyword("flags")
        name = self._expect_identifier("flags type name")
        self._expect(TokenKind.COLON, "':' before flags storage type")
        storage = self._expect_identifier("integer storage type").text
        if storage not in _INTEGER_TYPES: self._error("invalid_flags_storage_type", "Flags storage must be a fixed-width integer type.", self._previous().span)
        self._expect(TokenKind.LBRACE, "'{' before flags members")
        members: list[FlagsMember] = []
        seen: set[str] = set()
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                member = self._expect_identifier("flag member")
                self._expect(TokenKind.EQUAL, "'=' before bit position")
                bit = self._parse_integer_literal()
                self._expect_semicolon()
                if member.text in seen: self._error("duplicate_flag_member", f"Flag {member.text!r} is duplicated.", member.span)
                seen.add(member.text)
                members.append(FlagsMember(member.text, bit, self._join(member.span, self._previous().span)))
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after flags")
        return FlagsDefinition(name.text, storage, tuple(members), self._join(start.span, end.span))

    def _parse_fixed_type(self) -> FixedPointDefinition:
        start = self._expect_keyword("fixed")
        name = self._expect_identifier("fixed-point type name")
        self._expect(TokenKind.COLON, "':' before fixed-point storage")
        storage = self._expect_identifier("signed integer storage type").text
        props = self._parse_property_block()
        values = {p.name: p.value for p in props}
        bits = self._literal_integer(values.get("fractional_bits"))
        unit = self._literal_name(values.get("unit"))
        rounding = self._literal_name(values.get("rounding"))
        overflow = self._literal_name(values.get("overflow"))
        if storage not in {"i8", "i16", "i32", "i64"}: self._error("invalid_fixed_storage_type", "Fixed-point storage must be signed fixed-width integer.", start.span)
        return FixedPointDefinition(name.text, storage, bits if bits is not None else 0, unit or "1", rounding or "nearest_even", overflow or "reject", self._join(start.span, self._previous().span))

    def _parse_boundary_record(self, *, boundary_kind: str) -> RecordDefinition:
        start = self._advance()
        name = self._expect_identifier("record name")
        self._expect(TokenKind.LBRACE, "'{' after record name")
        fields: list[TypedBinding] = []
        metadata: dict[str, object] = {}
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                if self._word() in {"version", "schema_id", "alignment", "memory_order", "cache_policy", "integrity", "migration_policy"}:
                    prop = self._parse_property()
                    metadata[prop.name] = prop.value
                else:
                    field = self._expect_identifier("record field name")
                    self._expect(TokenKind.COLON, "':' after field name")
                    field_type = self._parse_type_spec()
                    self._expect_semicolon()
                    fields.append(TypedBinding(field.text, field_type, None, self._join(field.span, self._previous().span), "field"))
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after boundary record")
        version = self._literal_integer(metadata.get("version")) or 1
        boundary = RecordBoundary(boundary_kind, version, self._literal_name(metadata.get("schema_id")), self._literal_integer(metadata.get("alignment")), integrity=self._literal_name(metadata.get("integrity")), migration_policy=self._literal_name(metadata.get("migration_policy")), memory_order=self._literal_name(metadata.get("memory_order")), cache_policy=self._literal_name(metadata.get("cache_policy")), span=self._join(start.span, end.span))
        return RecordDefinition(name.text, tuple(fields), self._join(start.span, end.span), boundary)

    def _parse_constructor_signature(self) -> Declaration:
        word = self._word()
        start = self._current()
        if word == "custom":
            self._advance()
            role = self._expect_identifier("custom component kind").text
            name = self._expect_identifier("component name")
            if role == "algorithm": signature_type = self._expect_identifier("custom algorithm kind").text if self._match(TokenKind.COLON) else None
            else: signature_type = None
            self._expect_semicolon()
            return Declaration(f"custom_{role}", name.text, None, signature_type, span=self._join(start.span, self._previous().span), prototype=True)
        if word == "hybrid":
            self._advance(); self._expect_keyword("algorithm"); name = self._expect_identifier("hybrid algorithm name"); self._expect_semicolon()
            return Declaration("hybrid_algorithm", name.text, None, span=self._join(start.span, self._previous().span), prototype=True)
        if word == "subsystem":
            self._advance(); name = self._expect_identifier("subsystem name"); self._expect_semicolon()
            return Declaration("subsystem", name.text, None, span=self._join(start.span, self._previous().span), prototype=True)
        if word == "model":
            self._advance(); name = self._expect_identifier("model name"); self._expect(TokenKind.COLON, "':' before model form"); form = self._expect_identifier("continuous or discrete").text; self._expect_semicolon()
            return Declaration("model", name.text, None, form, span=self._join(start.span, self._previous().span), prototype=True)
        if word in _ROLE_KEYWORDS:
            self._advance(); name = self._expect_identifier(f"{word} name"); self._expect(TokenKind.COLON, "':' before UETD descriptor"); descriptor = self._expect_identifier("UETD DSL descriptor"); self._expect_semicolon()
            return Declaration(word, name.text, descriptor.text, span=self._join(start.span, self._previous().span), prototype=True)
        descriptor = self._expect_identifier("UETD DSL descriptor")
        name = self._expect_identifier("component instance name")
        self._expect_semicolon()
        return Declaration("module", name.text, descriptor.text, span=self._join(start.span, self._previous().span), prototype=True)

    def _parse_implementation_block(self) -> Declaration:
        start = self._expect_keyword("implementation")
        self._expect(TokenKind.LBRACE, "'{' after implementation")
        children: list[Declaration] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                if self._word() == "signal": children.append(self._parse_signal())
                else: children.append(self._parse_implementation_definition())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after implementation")
        return Declaration("implementation_block", None, None, children=tuple(children), span=self._join(start.span, end.span))

    def _parse_implementation_definition(self) -> Declaration:
        start = self._current()
        word = self._word()
        kind: str
        if word == "custom":
            self._advance(); role = self._expect_identifier("custom component kind").text; name = self._expect_identifier("component name").text; kind = f"custom_{role}"
        elif word == "hybrid":
            self._advance(); self._expect_keyword("algorithm"); name = self._expect_identifier("hybrid algorithm name").text; kind = "hybrid_algorithm"
        elif word in {"peripheral", "bus", "sensor", "actuator", "algorithm", "protocol", "model", "subsystem"}:
            self._advance(); name = self._expect_identifier("component name").text; kind = "model" if word == "model" else word
        else:
            descriptor = self._expect_identifier("UETD descriptor or component role")
            name = self._expect_identifier("component name").text
            kind = "module"
            signature = self._signatures.get(name)
            if signature and signature.registry_key != descriptor.text:
                self._error("implementation_descriptor_mismatch", f"Implementation descriptor for {name!r} differs from its signature.", descriptor.span)
        signature = self._signatures.get(name)
        if signature is None:
            self._error("implementation_without_signature", f"No header signature declares {name!r}.", start.span)
            registry_key = None
            signature_type = None
        else:
            registry_key = signature.registry_key
            signature_type = signature.signature_type
            if signature.kind != kind:
                self._error("implementation_role_mismatch", f"Implementation for {name!r} uses role {kind!r}; its signature uses {signature.kind!r}.", start.span)
            if name in self._implemented: self._error("duplicate_implementation_body", f"Component {name!r} has multiple implementation bodies.", start.span)
            self._implemented.add(name)
        if kind == "custom_algorithm":
            body = self._parse_custom_algorithm(name=name, signature=signature, start=start)
        elif kind == "model":
            body = self._parse_model(name=name, signature=signature, start=start)
        elif kind == "hybrid_algorithm":
            body = self._parse_hybrid_algorithm(name=name, signature=signature, start=start)
        elif kind == "subsystem":
            body = self._parse_subsystem(name=name, signature=signature, start=start)
        elif kind == "custom_protocol":
            body = self._parse_custom_protocol(name=name, signature=signature, start=start)
        else:
            body = self._parse_registry_component(component_kind=kind, name=name, signature=signature,
                                                  start=start, registry_key=registry_key,
                                                  signature_type=signature_type)
        return self._parse_implementation_descriptor_suffix(body, signature)

    def _parse_implementation_descriptor_suffix(self, body: Declaration,
                                                signature: Declaration | None) -> Declaration:
        """Require a visible trailing UETD descriptor for registered components.

        The constructor declaration remains the canonical contract source. The
        repeated suffix makes each implementation self-identifying and is
        checked against that declaration, so it cannot silently change the
        module selected by the project.
        """
        expected = getattr(signature, "registry_key", None) if signature is not None else None
        if not expected:
            return body
        current = self._current()
        following = self._peek_kind(1)
        if current.kind != TokenKind.IDENTIFIER or following != TokenKind.SEMICOLON:
            # Preserve a partially typed descriptor at the body boundary so
            # the editor can still build a partial AST and offer the exact
            # expected module completion without treating the token as a new
            # component declaration.
            if current.kind == TokenKind.IDENTIFIER and expected.casefold().startswith(current.text.casefold()):
                descriptor = self._advance()
                self._error(
                    "missing_implementation_descriptor_terminator",
                    f"Finish the implementation descriptor as '}} {expected};'.",
                    descriptor.span,
                )
                return body
            self._error(
                "missing_implementation_descriptor",
                f"End the {body.kind} implementation with '}} {expected};' to show its UETD module.",
                current.span,
            )
            return body
        descriptor = self._advance()
        self._advance()  # The descriptor's terminating semicolon.
        if descriptor.text != expected:
            self._error(
                "implementation_descriptor_mismatch",
                f"Implementation for {body.name!r} ends with UETD descriptor {descriptor.text!r}, "
                f"but its declaration selects {expected!r}.",
                descriptor.span,
            )
        return body

    def _parse_registry_component(self, *, component_kind: str, name: str, signature: Declaration | None, start: Token, registry_key: str | None = None, signature_type: str | None = None) -> Declaration:
        properties: list[Property] = []
        bindings: list[TypedBinding] = []
        children: list[Declaration] = []
        output_assignments: list[CompositionOutput] = []
        self._expect(TokenKind.LBRACE, "'{' before component settings")
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                if self._word() == "output" and self._peek_kind(2) == TokenKind.EQUAL:
                    output_assignments.append(self._parse_output_assignment())
                elif self._word() in {"input", "output", "state", "parameter", "local"}:
                    bindings.append(self._parse_port_binding() if self._word() in {"input", "output"} else self._parse_typed_binding())
                elif self._word() == "use" or (self._word() == "peripheral" and self._peek_kind(1) != TokenKind.EQUAL):
                    children.append(self._parse_reference_binding())
                elif self._word() in {"framing", "header", "sequence", "payload", "checksum", "behavior", "receiver", "state_machine", "message"}:
                    children.append(self._parse_nested_component_block())
                else:
                    properties.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after component settings")
        return Declaration(component_kind, name, registry_key, signature_type, tuple(properties), tuple(children), tuple(bindings), span=self._join(start.span, end.span), output_assignments=tuple(output_assignments))

    def _parse_dsl_builtin_call(self, builtin_id: str):
        start = self._expect_identifier(builtin_id)
        self._expect(TokenKind.LPAREN, "'(' after builtin")
        if builtin_id == "select":
            condition = self._parse_expression(); self._expect(TokenKind.COMMA, "',' after condition"); yes = self._parse_expression(); self._expect(TokenKind.COMMA, "',' after true value"); no = self._parse_expression()
            end = self._expect(TokenKind.RPAREN, "')' after select arguments")
            return SelectValue(condition, yes, no, self._join(start.span, end.span))
        fields: list[tuple[str, object]] = []
        seen: set[str] = set()
        if not self._at(TokenKind.RPAREN):
            while True:
                field = self._expect_identifier("map field name"); self._expect(TokenKind.EQUAL, "'=' after map field name"); value = self._parse_expression()
                if field.text in seen: self._error("duplicate_map_field", f"Map field {field.text!r} is duplicated.", field.span)
                seen.add(field.text); fields.append((field.text, value))
                if not self._match(TokenKind.COMMA): break
        end = self._expect(TokenKind.RPAREN, "')' after map fields")
        return RecordValue(tuple(fields), self._join(start.span, end.span))

    def _parse_custom_algorithm(self, *, name: str, signature: Declaration | None, start: Token) -> Declaration:
        self._expect(TokenKind.LBRACE, "'{' before custom algorithm")
        bindings: list[TypedBinding] = []; stages: list[AlgorithmStage] = []; props: list[Property] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                word = self._word()
                if word in {"input", "output", "state", "parameter", "local"}: bindings.append(self._parse_port_binding() if word in {"input", "output"} else self._parse_typed_binding())
                elif word in {"model", "sample_time"}: props.append(self._parse_property_or_binding())
                elif self._current().kind == TokenKind.IDENTIFIER and self._peek_kind(1) == TokenKind.LPAREN:
                    stages.append(self._parse_algorithm_stage())
                else:
                    # Custom algorithm implementation properties are open-ended
                    # metadata; only a named call-shaped block starts a lifecycle stage.
                    props.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after custom algorithm")
        return Declaration("custom_algorithm", name, None, signature.signature_type if signature else None, tuple(props), bindings=tuple(bindings), stages=tuple(stages), span=self._join(start.span, end.span))

    def _parse_model(self, *, name: str, signature: Declaration | None, start: Token) -> Declaration:
        self._expect(TokenKind.LBRACE, "'{' before model")
        bindings: list[TypedBinding] = []; props: list[Property] = []; equations: list[object] = []; children: list[Declaration] = []
        model_form: str | None = None
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                word = self._word()
                if word in {"state", "input", "output", "parameter"}: bindings.append(self._parse_typed_binding())
                elif word == "discretize" and self._peek_kind(1) == TokenKind.IDENTIFIER and self._peek_text(1) == "with":
                    props.append(self._parse_model_discretization())
                elif word in {"sample_time", "state_order", "input_order", "output_order", "discretize"}: props.append(self._parse_property_or_binding())
                elif word in {"linear", "jacobians"}:
                    if word == "linear":
                        if model_form is not None: self._error("duplicate_model_form", "A model may declare one primary dynamics form.", self._current().span)
                        model_form = "linear"
                    children.append(self._parse_matrix_block())
                elif word == "equations":
                    if model_form is not None: self._error("duplicate_model_form", "A model may declare one primary dynamics form.", self._current().span)
                    model_form = "equations"
                    equations.extend(self._parse_model_equations())
                else:
                    if model_form is None: model_form = "equations"
                    equations.append(self._parse_equation())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after model")
        return Declaration("model", name, None, signature.signature_type if signature else None, tuple(props), tuple(children), tuple(bindings), equations=tuple(equations), span=self._join(start.span, end.span), model_form=model_form)

    def _parse_model_discretization(self) -> Property:
        """Parse the explicit `discretize with method at period` model form."""
        start = self._expect_keyword("discretize")
        self._expect_keyword("with")
        method = self._parse_reference()
        self._expect_keyword("at")
        period = self._parse_expression()
        self._expect_semicolon()
        value = RecordValue((("method", method), ("period", period)), self._join(start.span, self._previous().span))
        return Property("discretize", value, value.span)

    def _parse_hybrid_algorithm(self, *, name: str, signature: Declaration | None, start: Token) -> Declaration:
        return self._parse_composition("hybrid_algorithm", name, signature, start)

    def _parse_subsystem(self, *, name: str, signature: Declaration | None, start: Token) -> Declaration:
        return self._parse_composition("subsystem", name, signature, start)

    def _parse_composition(self, kind: str, name: str, signature: Declaration | None, start: Token) -> Declaration:
        self._expect(TokenKind.LBRACE, "'{' before composition")
        bindings: list[TypedBinding] = []; children: list[Declaration] = []; stages: list[CompositionStage] = []; locals_: list[CompositionLocal] = []; outputs: list[CompositionOutput] = []; props: list[Property] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                word = self._word()
                if word == "input" or (word == "output" and self._peek_kind(2) == TokenKind.COLON):
                    binding = self._parse_port_binding()
                    bindings.append(binding)
                    if word == "output" and binding.value is not None: outputs.append(CompositionOutput(binding.name, binding.value, binding.span))
                elif word == "use": children.append(self._parse_use())
                elif word == "stage": stages.append(self._parse_composition_stage())
                elif word == "let": locals_.append(self._parse_composition_local())
                elif word == "choose": children.append(self._parse_choose())
                elif word == "output": outputs.append(self._parse_composition_output())
                else: props.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after composition")
        return Declaration(kind, name, None, signature.signature_type if signature else None, tuple(props), tuple(children), tuple(bindings), composition_stages=tuple(stages), composition_locals=tuple(locals_), composition_outputs=tuple(outputs), span=self._join(start.span, end.span))

    def _parse_enum(self) -> EnumDefinition:
        start = self._expect_keyword("enum"); name = self._expect_identifier("enum name"); self._expect(TokenKind.COLON, "':' before enum storage type"); storage = self._expect_identifier("fixed-width integer type")
        if storage.text not in _INTEGER_TYPES: self._error("invalid_enum_storage_type", "Enum storage must be a fixed-width integer type.", storage.span)
        self._expect(TokenKind.LBRACE, "'{' before enum cases"); variants: list[EnumVariant] = []; names: set[str] = set()
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            case = self._expect_identifier("enum case")
            value = self._parse_integer_literal() if self._match(TokenKind.EQUAL) else None
            if case.text in names: self._error("duplicate_enum_case", f"Enum case {case.text!r} is duplicated.", case.span)
            names.add(case.text); variants.append(EnumVariant(case.text, value, self._join(case.span, self._previous().span)))
            if not self._match(TokenKind.COMMA) and not self._at(TokenKind.RBRACE): self._expect_semicolon()
        end = self._expect(TokenKind.RBRACE, "'}' after enum")
        return EnumDefinition(name.text, storage.text, tuple(variants), self._join(start.span, end.span))

    def _parse_match_expression(self):
        start = self._expect_keyword("match"); subject = self._parse_expression(); self._expect(TokenKind.LBRACE, "'{' before match arms")
        arms: list[MatchArm] = []; default = None
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            arm_start = self._current()
            if self._match_keyword("otherwise") or self._match_keyword("default"):
                self._expect(TokenKind.COLON, "':' after default pattern"); default = self._parse_expression(); self._expect_semicolon(); continue
            self._expect_keyword("case")
            pattern = self._parse_reference()
            binding = None; field_bindings: list[tuple[str, str]] = []
            if self._match(TokenKind.LPAREN):
                if not self._at(TokenKind.RPAREN):
                    binding = self._expect_identifier("match binding").text
                    while self._match(TokenKind.COMMA):
                        field = self._expect_identifier("payload field"); self._expect(TokenKind.EQUAL, "'=' in payload binding"); target = self._expect_identifier("payload binding"); field_bindings.append((field.text, target.text))
                self._expect(TokenKind.RPAREN, "')' after match bindings")
            self._expect(TokenKind.COLON, "':' after match pattern"); expression = self._parse_expression(); self._expect_semicolon()
            arms.append(MatchArm(".".join(pattern.path), binding, tuple(field_bindings), expression, self._join(arm_start.span, self._previous().span)))
        end = self._expect(TokenKind.RBRACE, "'}' after match")
        return MatchExpression(subject, tuple(arms), default, self._join(start.span, end.span))

    def _parse_variant_constructor(self):
        start = self._parse_reference()
        self._expect(TokenKind.LPAREN, "'(' after variant constructor")
        positional = None; fields: list[tuple[str, object]] = []
        if not self._at(TokenKind.RPAREN):
            if self._current().kind == TokenKind.IDENTIFIER and self._peek_kind(1) == TokenKind.EQUAL:
                seen: set[str] = set()
                while True:
                    field = self._expect_identifier("variant payload field"); self._expect(TokenKind.EQUAL, "'=' after field"); value = self._parse_expression()
                    if field.text in seen: self._error("duplicate_variant_payload", f"Payload field {field.text!r} is duplicated.", field.span)
                    seen.add(field.text); fields.append((field.text, value))
                    if not self._match(TokenKind.COMMA): break
            else: positional = self._parse_expression()
        end = self._expect(TokenKind.RPAREN, "')' after variant constructor")
        return VariantConstructorExpression(".".join(start.path), positional, tuple(fields), self._join(start.span, end.span))

    def _parse_flags_operation(self):
        start = self._expect_identifier("flags operation"); self._expect(TokenKind.LPAREN, "'(' after flags operation"); value = self._parse_expression(); self._expect(TokenKind.COMMA, "',' after flags value"); flag = self._parse_reference(); end = self._expect(TokenKind.RPAREN, "')' after flags operation")
        if start.text not in {"has", "set", "clear"}: self._error("unknown_flags_operation", "Flags operation must be has, set, or clear.", start.span)
        if len(flag.path) != 2: self._error("invalid_flag_member", "A flags member must use Type.member syntax.", flag.span)
        return FlagsOperationExpression(start.text, value, ".".join(flag.path), self._join(start.span, end.span))

    def _parse_state_machine(self) -> Declaration:
        start = self._expect_keyword("state_machine"); name = self._expect_identifier("state machine name"); self._expect(TokenKind.COLON, "':' before generated state type name"); state_type = self._expect_identifier("generated state type name"); self._expect(TokenKind.LBRACE, "'{' before state machine declarations")
        children: list[Declaration] = []; props: list[Property] = []; state_names: set[str] = set(); phase = 0
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                word = self._word()
                if word == "state":
                    kw = self._advance()
                    state = self._expect_identifier("state name")
                    tag = None
                    tag_span = None
                    if self._match(TokenKind.EQUAL):
                        tag = self._parse_integer_literal()
                        tag_span = self._previous().span
                    self._expect_semicolon()
                    if phase != 0:
                        self._error("state_declaration_order", "Declare every state before the initial state and transitions.", kw.span)
                    if state.text in state_names:
                        self._error("duplicate_machine_state", f"State {state.text!r} is declared more than once.", state.span)
                    state_names.add(state.text)
                    properties = ()
                    if tag is not None:
                        properties = (Property("tag", Literal(tag, None, tag_span), tag_span),)
                    children.append(Declaration("machine_state", state.text, None, properties=properties,
                                                span=self._join(kw.span, self._previous().span)))
                elif word == "initial":
                    kw = self._advance(); state = self._parse_reference(); self._expect_semicolon(); props.append(Property("initial", Identifier(".".join(state.path), state.span), self._join(kw.span, self._previous().span)))
                    if phase > 1:
                        self._error("state_initial_order", "Declare the initial state before transitions.", kw.span)
                    phase = max(phase, 1)
                elif word == "transition":
                    phase = 2
                    kw = self._advance(); src = self._expect_identifier("source state"); self._expect(TokenKind.ARROW, "'->' in transition"); dst = self._expect_identifier("destination state"); self._expect_keyword("when"); guard = self._parse_expression(); self._expect_semicolon(); children.append(Declaration("transition", src.text, dst.text, properties=(Property("when", guard, self._join(kw.span, self._previous().span)),), span=self._join(kw.span, self._previous().span)))
                elif word in {"on_enter", "on_exit"}:
                    phase = 2
                    kw = self._advance(); state = self._expect_identifier("state name"); self._expect_keyword("call"); op = self._parse_reference(); self._expect_semicolon(); children.append(Declaration("state_action", state.text, kw.text, properties=(Property("operation", op, op.span),), span=self._join(kw.span, self._previous().span)))
                else: props.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after state machine")
        if not state_names:
            self._error("state_machine_states_required", "A state machine must declare its states explicitly with 'state name;' before 'initial'.", self._join(start.span, end.span))
        return Declaration("state_machine", name.text, state_type.text, properties=tuple(props), children=tuple(children), span=self._join(start.span, end.span))

    def _parse_custom_protocol(self, *, name: str, signature: Declaration | None, start: Token) -> Declaration:
        declaration = self._parse_generic_named_block("custom_protocol", name, start, signature)
        return declaration

    def _parse_taskset(self) -> Declaration:
        start = self._expect_keyword("taskset"); name = self._expect_identifier("taskset name"); self._expect(TokenKind.LBRACE, "'{' before taskset"); children: list[Declaration] = []; props: list[Property] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                if self._word() in {"task", "interrupt", "channel", "resource", "constraint"}: children.append(self._parse_os_member())
                else: props.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after taskset")
        return Declaration("taskset", name.text, None, properties=tuple(props), children=tuple(children), span=self._join(start.span, end.span))

    def _parse_operatingsystem(self) -> Declaration:
        start = self._expect_keyword("operatingsystem"); name = self._expect_identifier("operatingsystem name"); self._expect(TokenKind.LBRACE, "'{' before operating-system declarations"); children: list[Declaration] = []; props: list[Property] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                word = self._word()
                if word == "state_machine": children.append(self._parse_state_machine())
                elif word == "taskset": children.append(self._parse_taskset())
                elif word in {"loop", "task", "interrupt", "channel", "resource", "constraint"}: children.append(self._parse_os_member())
                else: props.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after operatingsystem")
        return Declaration("operatingsystem", name.text, None, properties=tuple(props), children=tuple(children), span=self._join(start.span, end.span))

    def _parse_os_member(self) -> Declaration:
        start = self._current(); kind = self._advance().text; name = self._expect_identifier(f"{kind} name")
        if kind in {"task", "loop"}:
            self._expect(TokenKind.LBRACE, "'{' before scheduled work")
            children: list[Declaration] = []; props: list[Property] = []
            while not self._at(TokenKind.RBRACE, TokenKind.EOF):
                mark = self.index
                try:
                    if self._word() == "run": children.append(self._parse_run_clause())
                    else: props.append(self._parse_property())
                except _ParseFailure: self._synchronize()
                if mark == self.index: self._advance()
            end = self._expect(TokenKind.RBRACE, "'}' after scheduled work")
            return Declaration(kind, name.text, None, properties=tuple(props), children=tuple(children), span=self._join(start.span, end.span))
        properties = self._parse_property_block()
        return Declaration(kind, name.text, None, properties=tuple(properties), span=self._join(start.span, self._previous().span))

    def _parse_run_clause(self) -> Declaration:
        start = self._expect_keyword("run"); self._expect(TokenKind.EQUAL, "'=' after run"); callee = self._parse_reference(); args: list[Property] = []
        if self._match(TokenKind.LPAREN):
            if not self._at(TokenKind.RPAREN):
                while True:
                    arg = self._expect_identifier("run argument name"); self._expect(TokenKind.EQUAL, "'=' after run argument"); value = self._parse_expression(); args.append(Property(arg.text, value, self._join(arg.span, self._previous().span)))
                    if not self._match(TokenKind.COMMA): break
            self._expect(TokenKind.RPAREN, "')' after run arguments")
        self._expect_semicolon()
        return Declaration("run", ".".join(callee.path), None, properties=tuple(args), span=self._join(start.span, self._previous().span))

    def _parse_typed_binding(self) -> TypedBinding:
        role = self._expect_identifier("binding role")
        name = self._expect_identifier(f"{role.text} name"); self._expect(TokenKind.COLON, "':' before binding type"); type_spec = self._parse_type_spec()
        value = self._parse_expression() if self._match(TokenKind.EQUAL) else None
        self._expect_semicolon()
        return TypedBinding(name.text, type_spec, value, self._join(role.span, self._previous().span), role.text)

    def _parse_port_binding(self) -> TypedBinding:
        role = self._advance(); name = self._expect_identifier(f"{role.text} name"); self._expect(TokenKind.COLON, "':' before port type"); type_spec = self._parse_type_spec(); props: list[Property] = []
        has_contract_block = self._match(TokenKind.LBRACE)
        if has_contract_block:
            while not self._at(TokenKind.RBRACE, TokenKind.EOF): props.append(self._parse_property())
            self._expect(TokenKind.RBRACE, "'}' after port contract")
        value = None
        if self._match(TokenKind.EQUAL): value = self._parse_expression()
        if has_contract_block: self._match(TokenKind.SEMICOLON)
        else: self._expect_semicolon()
        return TypedBinding(name.text, type_spec, value, self._join(role.span, self._previous().span), role.text, tuple(props))

    def _parse_signal(self) -> Declaration:
        start = self._expect_keyword("signal"); name = self._expect_identifier("signal name"); binding = None; value = None
        if self._match(TokenKind.COLON):
            type_spec = self._parse_type_spec(); value = self._parse_expression() if self._match(TokenKind.EQUAL) else None
            binding = TypedBinding(name.text, type_spec, value, self._join(start.span, self._previous().span), "signal")
        elif self._match(TokenKind.EQUAL): value = self._parse_expression(); binding = TypedBinding(name.text, None, value, self._join(start.span, self._previous().span), "signal")
        else: self._fail("invalid_signal", "A signal requires a type or initializer.", name.span)
        self._expect_semicolon()
        return Declaration("signal", name.text, None, bindings=(binding,), span=self._join(start.span, self._previous().span))

    def _parse_equation(self, *, mode: str = "assign"):
        target = self._parse_reference()
        if self._match(TokenKind.LBRACKET):
            subs = self._parse_subscripts(TokenKind.RBRACKET); end = self._expect(TokenKind.RBRACKET, "']' after equation target")
            target = IndexExpression(target, tuple(subs), self._join(target.span, end.span))
        self._expect(TokenKind.EQUAL, "'=' in equation")
        expression = self._parse_expression(); self._expect_semicolon()
        return Equation(target, expression, self._join(target.span, self._previous().span), mode)

    def _parse_bounded_loop(self):
        start = self._expect_keyword("for"); variable = self._expect_identifier("loop index"); self._expect_keyword("in"); low = self._parse_expression(); self._expect(TokenKind.RANGE, "'..' in bounded loop"); high = self._parse_expression(); body = self._parse_statement_block(); end = self._previous()
        return BoundedLoop(variable.text, low, high, tuple(body), self._join(start.span, end.span))

    def _parse_conditional_statement(self):
        start = self._expect_keyword("if"); self._match(TokenKind.LPAREN); condition = self._parse_expression(); self._match(TokenKind.RPAREN); then_body = self._parse_statement_block(); else_body = self._parse_statement_block() if self._match_keyword("else") else []; end = self._previous()
        return ConditionalStatement(condition, tuple(then_body), tuple(else_body), self._join(start.span, end.span))

    def _parse_algorithm_statement(self):
        if self._word() == "for": return self._parse_bounded_loop()
        if self._word() == "if": return self._parse_conditional_statement()
        return self._parse_equation()

    def _parse_algorithm_stage(self) -> AlgorithmStage:
        start = self._expect_identifier("algorithm stage name"); self._expect(TokenKind.LPAREN, "'(' after stage name"); params: list[TypedBinding] = []
        if not self._at(TokenKind.RPAREN):
            while True:
                param = self._expect_identifier("stage parameter"); self._expect(TokenKind.COLON, "':' before parameter type"); type_spec = self._parse_type_spec(); params.append(TypedBinding(param.text, type_spec, None, self._join(param.span, self._previous().span), "parameter"))
                if not self._match(TokenKind.COMMA): break
        self._expect(TokenKind.RPAREN, "')' after stage parameters"); body = self._parse_statement_block(); end = self._previous()
        return AlgorithmStage(start.text, tuple(params), tuple(body), self._join(start.span, end.span))

    def _parse_statement_block(self):
        self._expect(TokenKind.LBRACE, "'{' before algorithm statements"); statements: list[object] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try: statements.append(self._parse_algorithm_statement())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        self._expect(TokenKind.RBRACE, "'}' after algorithm statements")
        return statements

    def _parse_expression(self, minimum_precedence: int = 0):
        left = self._parse_unary()
        while True:
            operator = self._word()
            if self._current().kind in {TokenKind.PLUS, TokenKind.MINUS, TokenKind.STAR, TokenKind.SLASH, TokenKind.POWER, TokenKind.EQUAL_EQUAL, TokenKind.NOT_EQUAL, TokenKind.LESS, TokenKind.LESS_EQUAL, TokenKind.GREATER, TokenKind.GREATER_EQUAL, TokenKind.PIPE, TokenKind.AMPERSAND, TokenKind.CARET}:
                operator = self._current().text
            precedence = _PRECEDENCE.get(operator, -1)
            if precedence < minimum_precedence: break
            op = self._advance()
            right = self._parse_expression(precedence if operator == "**" else precedence + 1)
            left = BinaryExpression(operator, left, right, self._join(left.span, right.span))
            next_operator = self._current().text if self._current().kind in {TokenKind.EQUAL_EQUAL, TokenKind.NOT_EQUAL, TokenKind.LESS, TokenKind.LESS_EQUAL, TokenKind.GREATER, TokenKind.GREATER_EQUAL} else self._word()
            if operator in {"==", "!=", "<", "<=", ">", ">="} and _PRECEDENCE.get(next_operator) == precedence:
                self._error("chained_comparison", "Comparisons are non-associative; combine them with 'and'.", self._current().span)
            _ = op
        return left

    def _parse_unary(self):
        token = self._current()
        if token.kind in {TokenKind.PLUS, TokenKind.MINUS} or self._word() == "not":
            self._advance(); operand = self._parse_unary(); return UnaryExpression(token.text, operand, self._join(token.span, operand.span))
        return self._parse_primary()

    def _parse_primary(self):
        token = self._current()
        if token.kind == TokenKind.NUMBER:
            self._advance(); value = self._number_value(token); unit = self._parse_optional_unit()
            end = self._previous().span if unit else token.span
            return Literal(value, unit, self._join(token.span, end))
        if token.kind == TokenKind.STRING:
            self._advance(); return Literal(self._decode_string(token.text), None, token.span)
        if token.kind == TokenKind.LPAREN:
            self._advance(); value = self._parse_expression(); self._expect(TokenKind.RPAREN, "')'"); return value
        if token.kind == TokenKind.LBRACKET:
            return self._parse_matrix_or_vector()
        if self._word() == "None":
            start = self._advance()
            return VariantConstructorExpression("None", span=start.span)
        if self._word() == "true" or self._word() == "false":
            self._advance(); return Literal(token.text == "true", None, token.span)
        if self._word() == "match": return self._parse_match_expression()
        if self._word() in {"map", "select"}: return self._parse_dsl_builtin_call(self._word())
        if self._word() == "convert_unit": return self._parse_unit_conversion_expression()
        if self._word() == "cast":
            start = self._advance(); self._expect(TokenKind.LESS, "'<' before cast type"); dest = self._parse_type_spec(); self._expect(TokenKind.GREATER, "'>' after cast type"); self._expect(TokenKind.LPAREN, "'(' before cast value"); value = self._parse_expression(); end = self._expect(TokenKind.RPAREN, "')' after cast value"); return CastExpression(dest, value, self._join(start.span, end.span))
        if self._word() in {"has", "set", "clear"}: return self._parse_flags_operation()
        if token.kind == TokenKind.IDENTIFIER:
            reference_start = self.index
            ref = self._parse_reference()
            if self._match(TokenKind.LPAREN):
                if ref.path[-1] in {"Some", "None", "Ok", "Err"} or len(ref.path) > 1:
                    self.index = reference_start
                    return self._parse_variant_constructor()
                args: list[object] = []
                if not self._at(TokenKind.RPAREN):
                    while True:
                        args.append(self._parse_expression())
                        if not self._match(TokenKind.COMMA): break
                end = self._expect(TokenKind.RPAREN, "')' after function arguments")
                return MathCallExpression(".".join(ref.path), tuple(args), self._join(ref.span, end.span))
            value: object = ref
            if self._match(TokenKind.LBRACKET):
                subs = self._parse_subscripts(TokenKind.RBRACKET); end = self._expect(TokenKind.RBRACKET, "']' after index"); value = IndexExpression(ref, tuple(subs), self._join(ref.span, end.span))
            return value
        self._fail("expected_expression", "Expected a literal, reference, or parenthesized expression.", token.span)

    def _parse_unit_conversion_expression(self):
        start = self._expect_keyword("convert_unit"); self._expect(TokenKind.LESS, "'<' before target unit"); unit = self._parse_unit(); self._expect(TokenKind.GREATER, "'>' after target unit"); self._expect(TokenKind.LPAREN, "'(' before converted value"); value = self._parse_expression(); end = self._expect(TokenKind.RPAREN, "')' after converted value")
        return UnitConversionExpression(unit, value, self._join(start.span, end.span))

    def _parse_type_spec(self) -> TypeSpec:
        start = self._expect_identifier("type name")
        name = start.text; args: list[TypeSpec] = []; shape: list[int | str] = []; unit = None
        if name in {"Received", "Option", "Result"} and self._match(TokenKind.LESS):
            args.append(self._parse_type_spec())
            while self._match(TokenKind.COMMA): args.append(self._parse_type_spec())
            self._expect(TokenKind.GREATER, "'>' after type arguments")
        if self._match(TokenKind.LBRACKET):
            while True:
                if self._current().kind == TokenKind.IDENTIFIER:
                    parts = [self._advance().text]
                    while self._match(TokenKind.DOT):
                        parts.append(self._expect_identifier("array dimension member").text)
                    shape.append(".".join(parts))
                else:
                    bound = self._expect(TokenKind.NUMBER, "positive array dimension")
                    number = self._number_value(bound)
                    if not isinstance(number, int) or number <= 0: self._error("invalid_array_dimension", "Array dimensions must be positive integer literals or constants.", bound.span)
                    shape.append(number)
                if not self._match(TokenKind.COMMA): break
            self._expect(TokenKind.RBRACKET, "']' after dimensions")
        if self._match(TokenKind.LESS):
            unit = self._parse_unit(); self._expect(TokenKind.GREATER, "'>' after unit")
        end = self._previous().span
        return TypeSpec(name, tuple(shape), unit, self._join(start.span, end), tuple(args))

    def _parse_matrix_literal(self):
        return self._parse_matrix_or_vector()

    def _parse_matrix_or_vector(self):
        start = self._expect(TokenKind.LBRACKET, "'['")
        if self._at(TokenKind.RBRACKET):
            end = self._advance(); return ListValue((), self._join(start.span, end.span))
        if self._at(TokenKind.LBRACKET):
            rows: list[tuple[object, ...]] = []
            while True:
                self._expect(TokenKind.LBRACKET, "'[' before matrix row")
                row: list[object] = []
                if self._at(TokenKind.RBRACKET): self._fail("empty_matrix_row", "Matrix rows cannot be empty.", self._current().span)
                row.append(self._parse_expression())
                while self._match(TokenKind.COMMA): row.append(self._parse_expression())
                self._expect(TokenKind.RBRACKET, "']' after matrix row")
                rows.append(tuple(row))
                if not self._match(TokenKind.COMMA): break
                if not self._at(TokenKind.LBRACKET): self._fail("matrix_row_expected", "Expected a bracketed row after matrix comma.", self._current().span)
            end = self._expect(TokenKind.RBRACKET, "']' after matrix literal")
            if len({len(row) for row in rows}) != 1: self._error("ragged_matrix", "Matrix rows must have equal lengths.", self._join(start.span, end.span))
            return MatrixLiteral(tuple(rows), self._join(start.span, end.span))
        first = self._parse_expression()
        items = [first]
        while self._match(TokenKind.COMMA):
            if self._at(TokenKind.RBRACKET): self._error("trailing_comma", "A trailing comma is not allowed in a vector or matrix literal.", self._current().span); break
            items.append(self._parse_expression())
        end = self._expect(TokenKind.RBRACKET, "']' after vector literal")
        return ListValue(tuple(items), self._join(start.span, end.span))

    def _parse_property_block(self) -> list[Property]:
        self._expect(TokenKind.LBRACE, "'{' before properties"); props: list[Property] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try: props.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        self._expect(TokenKind.RBRACE, "'}' after properties")
        return props

    def _parse_property(self) -> Property:
        start = self._parse_reference(); self._expect(TokenKind.EQUAL, "'=' in property"); value = self._parse_expression(); self._expect_semicolon()
        return Property(".".join(start.path), value, self._join(start.span, self._previous().span))

    def _parse_property_or_binding(self) -> Property:
        start = self._current(); name = self._expect_identifier("property name"); self._expect(TokenKind.EQUAL, "'=' after property name"); value = self._parse_expression(); self._expect_semicolon()
        return Property(name.text, value, self._join(start.span, self._previous().span))

    def _parse_reference_binding(self) -> Declaration:
        start = self._advance(); target = self._expect_identifier("binding name"); self._expect(TokenKind.EQUAL, "'=' after binding name"); value = self._parse_reference(); self._expect_semicolon()
        return Declaration(start.text, target.text, ".".join(value.path), span=self._join(start.span, self._previous().span))

    def _parse_use(self) -> Declaration:
        start = self._expect_keyword("use"); ref = self._parse_reference(); alias = ref.path[-1]
        if self._match_keyword("as"): alias = self._expect_identifier("member alias").text
        self._expect_semicolon()
        return Declaration("use", alias, ".".join(ref.path), span=self._join(start.span, self._previous().span))

    def _parse_composition_stage(self) -> CompositionStage:
        start = self._expect_keyword("stage"); name = self._expect_identifier("stage result"); self._expect(TokenKind.EQUAL, "'=' after stage result"); member = self._parse_reference(); self._expect(TokenKind.LPAREN, "'(' after stage member"); args: list[tuple[str, object]] = []
        if not self._at(TokenKind.RPAREN):
            while True:
                arg = self._expect_identifier("stage argument"); self._expect(TokenKind.EQUAL, "'=' after argument name"); args.append((arg.text, self._parse_expression()))
                if not self._match(TokenKind.COMMA): break
        self._expect(TokenKind.RPAREN, "')' after stage arguments")
        guard = None; on_failure = None
        if self._match_keyword("when"): guard = self._parse_expression()
        if self._match_keyword("on_failure"): self._expect(TokenKind.EQUAL, "'=' after on_failure"); on_failure = self._parse_expression()
        self._expect_semicolon()
        return CompositionStage(name.text, ".".join(member.path), tuple(args), guard, on_failure, self._join(start.span, self._previous().span))

    def _parse_composition_local(self) -> CompositionLocal:
        start = self._expect_keyword("let"); name = self._expect_identifier("local name"); self._expect(TokenKind.COLON, "':' before local type"); type_spec = self._parse_type_spec(); self._expect(TokenKind.EQUAL, "'=' before local value"); value = self._parse_expression(); self._expect_semicolon()
        return CompositionLocal(name.text, type_spec, value, self._join(start.span, self._previous().span))

    def _parse_composition_output(self) -> CompositionOutput:
        start = self._expect_keyword("output"); name = self._parse_reference(); self._expect(TokenKind.EQUAL, "'=' before output expression"); value = self._parse_expression(); self._expect_semicolon()
        return CompositionOutput(".".join(name.path), value, self._join(start.span, self._previous().span))

    def _parse_choose(self) -> Declaration:
        start = self._expect_keyword("choose"); name = self._expect_identifier("choice output name"); self._expect(TokenKind.LBRACE, "'{' before choose branches"); branches: list[Property] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            branch_start = self._current()
            if self._match_keyword("otherwise"):
                self._expect(TokenKind.COLON, "':' after otherwise"); value = self._parse_expression(); self._expect_semicolon(); branches.append(Property("otherwise", value, self._join(branch_start.span, self._previous().span)))
            else:
                self._expect_keyword("when"); condition = self._parse_expression(); self._expect(TokenKind.COLON, "':' after choose condition"); value = self._parse_expression(); self._expect_semicolon(); branches.append(Property("when", RecordValue((("condition", condition), ("value", value)), self._join(branch_start.span, self._previous().span)), self._join(branch_start.span, self._previous().span)))
        end = self._expect(TokenKind.RBRACE, "'}' after choose")
        return Declaration("choose", name.text, None, properties=tuple(branches), span=self._join(start.span, end.span))

    def _parse_generic_named_block(self, kind: str, name: str, start: Token, signature: Declaration | None) -> Declaration:
        self._expect(TokenKind.LBRACE, "'{' before component body"); props: list[Property] = []; children: list[Declaration] = []; bindings: list[TypedBinding] = []; output_assignments: list[CompositionOutput] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                if self._word() == "output" and self._peek_kind(2) == TokenKind.EQUAL:
                    output_assignments.append(self._parse_output_assignment())
                elif self._word() in {"input", "output", "state", "parameter", "local"}:
                    bindings.append(self._parse_port_binding() if self._word() in {"input", "output"} else self._parse_typed_binding())
                elif self._word() in {"framing", "header", "sequence", "payload", "checksum", "behavior", "receiver", "message", "fragment", "state_machine"}: children.append(self._parse_nested_component_block())
                else: props.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if mark == self.index: self._advance()
        end = self._expect(TokenKind.RBRACE, "'}' after component body")
        return Declaration(kind, name, None, signature.signature_type if signature else None, tuple(props), tuple(children), tuple(bindings), span=self._join(start.span, end.span), output_assignments=tuple(output_assignments))

    def _parse_output_assignment(self) -> CompositionOutput:
        """Parse a typed component's `output field = expression` initializer."""
        start = self._expect_keyword("output")
        name = self._expect_identifier("output name")
        self._expect(TokenKind.EQUAL, "'=' after output name")
        value = self._parse_expression()
        self._expect_semicolon()
        return CompositionOutput(name.text, value, self._join(start.span, self._previous().span))

    def _parse_nested_component_block(self) -> Declaration:
        start = self._advance()
        name: str | None = None
        if start.text == "message":
            name = self._expect_identifier("message name").text
            self._expect(TokenKind.LBRACE, "'{' before protocol message")
            properties: list[Property] = []
            children: list[Declaration] = []
            while not self._at(TokenKind.RBRACE, TokenKind.EOF):
                mark = self.index
                try:
                    if self._word() == "field":
                        children.append(self._parse_protocol_field_declaration())
                    elif self._word() == "flag":
                        children.append(self._parse_protocol_flag_declaration())
                    elif self._word() == "fragment":
                        children.append(self._parse_nested_component_block())
                    else:
                        properties.append(self._parse_property())
                except _ParseFailure:
                    self._synchronize()
                if mark == self.index:
                    self._advance()
            end = self._expect(TokenKind.RBRACE, "'}' after protocol message")
            return Declaration("message", name, None, properties=tuple(properties), children=tuple(children), span=self._join(start.span, end.span))
        if start.text == "field":
            self.index -= 1
            return self._parse_protocol_field_declaration()
        if start.text == "fragment":
            if self._current().kind == TokenKind.NUMBER:
                name = str(self._number_value(self._advance()))
            elif self._current().kind == TokenKind.IDENTIFIER:
                name = self._advance().text
            self._expect(TokenKind.LBRACE, "'{' before protocol fragment")
            properties: list[Property] = []
            children: list[Declaration] = []
            while not self._at(TokenKind.RBRACE, TokenKind.EOF):
                mark = self.index
                try:
                    if self._word() == "field":
                        children.append(self._parse_protocol_field_declaration())
                    elif self._word() == "flag":
                        children.append(self._parse_protocol_flag_declaration())
                    else:
                        properties.append(self._parse_property())
                except _ParseFailure:
                    self._synchronize()
                if mark == self.index:
                    self._advance()
            end = self._expect(TokenKind.RBRACE, "'}' after protocol fragment")
            return Declaration("fragment", name, None, properties=tuple(properties), children=tuple(children), span=self._join(start.span, end.span))
        if start.text == "state_machine":
            self._expect(TokenKind.LBRACE, "'{' before protocol parser states")
            children: list[Declaration] = []
            state_names: set[str] = set()
            while not self._at(TokenKind.RBRACE, TokenKind.EOF):
                mark = self.index
                try:
                    if self._match_keyword("state"):
                        state = self._expect_identifier("protocol parser state")
                        self._expect_semicolon()
                        if state.text in state_names:
                            self._error("duplicate_protocol_state", f"Protocol parser state {state.text!r} is repeated.", state.span)
                        state_names.add(state.text)
                        children.append(Declaration("protocol_state", state.text, None,
                                                    span=self._join(state.span, self._previous().span)))
                    elif self._word() == "on":
                        transition_start = self._advance()
                        event = self._expect_identifier("protocol event")
                        self._expect_keyword("from")
                        source = self._expect_identifier("source protocol state")
                        self._expect(TokenKind.ARROW, "'->' in protocol transition")
                        destination = self._expect_identifier("destination protocol state")
                        self._expect_semicolon()
                        children.append(Declaration("protocol_transition", event.text, source.text,
                                                    signature_type=destination.text,
                                                    span=self._join(transition_start.span, self._previous().span)))
                    else:
                        self._fail("invalid_protocol_state", "Expected state declaration or on-event transition.", self._current().span)
                except _ParseFailure:
                    self._synchronize()
                if self.index == mark:
                    self._advance()
            end = self._expect(TokenKind.RBRACE, "'}' after protocol parser state machine")
            return Declaration("state_machine", None, None, children=tuple(children), span=self._join(start.span, end.span))
        name = self._expect_identifier(f"{start.text} block name").text if self._current().kind == TokenKind.IDENTIFIER and self._peek_kind(1) == TokenKind.LBRACE else None
        props = self._parse_property_block()
        return Declaration(start.text, name, None, properties=tuple(props), span=self._join(start.span, self._previous().span))

    def _parse_protocol_field_declaration(self) -> Declaration:
        start = self._expect_keyword("field")
        name = self._expect_identifier("protocol field name")
        self._expect(TokenKind.COLON, "':' before protocol field type")
        type_spec = self._parse_type_spec()
        props = self._parse_property_block() if self._at(TokenKind.LBRACE) else []
        binding = TypedBinding(name.text, type_spec, None, self._join(start.span, self._previous().span), "field")
        return Declaration("field", name.text, None, properties=tuple(props), bindings=(binding,), span=self._join(start.span, self._previous().span))

    def _parse_protocol_flag_declaration(self) -> Declaration:
        start = self._expect_keyword("flag")
        name = self._expect_identifier("protocol flag name")
        self._expect(TokenKind.EQUAL, "'=' after protocol flag name")
        source = self._expect_identifier("source flags field")
        self._expect(TokenKind.DOT, "'.bit' after protocol flags field")
        self._expect_keyword("bit")
        self._expect(TokenKind.LBRACKET, "'[' before protocol flag bit index")
        bit = self._parse_integer_literal()
        self._expect(TokenKind.RBRACKET, "']' after protocol flag bit index")
        self._expect_semicolon()
        return Declaration("protocol_flag", name.text, source.text, signature_type=str(bit),
                           span=self._join(start.span, self._previous().span))

    def _parse_matrix_block(self) -> Declaration:
        start = self._advance(); name = None; props = self._parse_property_block(); return Declaration(start.text, name, None, properties=tuple(props), span=self._join(start.span, self._previous().span))

    def _parse_model_equations(self):
        start = self._expect_keyword("equations"); self._expect(TokenKind.LBRACE, "'{' before model equations"); values: list[object] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            # `derivative.state = ...` is an ordinary field assignment. The
            # `derivative state = ...` form uses the keyword as a mode marker.
            if self._word() == "derivative" and self._peek_kind(1) != TokenKind.DOT:
                self._advance()
                values.append(self._parse_equation(mode="derivative"))
            elif self._word() == "next" and self._peek_kind(1) != TokenKind.DOT:
                self._advance()
                values.append(self._parse_equation(mode="next"))
            else:
                values.append(self._parse_equation())
        self._expect(TokenKind.RBRACE, "'}' after equations"); return values

    def _parse_subscripts(self, closing: TokenKind) -> list[IndexSubscript]:
        subs: list[IndexSubscript] = []
        if self._at(closing): return subs
        while True:
            low = self._parse_expression()
            if self._match(TokenKind.COLON): subs.append(IndexSubscript(low, self._parse_expression()))
            else: subs.append(IndexSubscript(low))
            if not self._match(TokenKind.COMMA): break
        return subs

    def _parse_reference(self) -> Reference:
        first = self._expect_identifier("identifier")
        parts = [first.text]
        while self._match(TokenKind.DOT): parts.append(self._expect_identifier("member name after '.'").text)
        return Reference(tuple(parts), self._join(first.span, self._previous().span))

    def _parse_optional_unit(self) -> str | None:
        # Boolean connectives can follow a dimensionless literal (`x == 0 or
        # y == 1`). They are operators, never unit names, and must not be
        # swallowed by the quantity parser.
        if self._word() in {"and", "or"}:
            return None
        if self._current().kind == TokenKind.IDENTIFIER or (self._current().kind == TokenKind.NUMBER and self._current().text == "1"):
            if self._current().kind == TokenKind.IDENTIFIER or self._unit_operator_follows(self._current()):
                return self._parse_unit()
        return None

    def _unit_operator_follows(self, token: Token) -> bool:
        """Return whether a contiguous `*` or `/` starts a unit after token.

        Arithmetic operators separated by whitespace belong to the expression
        grammar (`1000 mAh / (3600 A*s)`). Unit products and quotients are
        written contiguously (`m/s^2`, `A*s`) to keep numeric suffixes
        unambiguous without needing registry data in the parser.
        """
        if self.index + 1 >= len(self.tokens):
            return False
        operator_index = self.index + 1 if token is self._current() else self.index
        if operator_index >= len(self.tokens):
            return False
        operator = self.tokens[operator_index]
        return (
            operator.kind in {TokenKind.STAR, TokenKind.SLASH}
            and token.span.end.offset == operator.span.start.offset
            and operator_index + 1 < len(self.tokens)
            and operator.span.end.offset == self.tokens[operator_index + 1].span.start.offset
            and self.tokens[operator_index + 1].kind == TokenKind.IDENTIFIER
        )

    def _parse_unit(self) -> str:
        pieces: list[str] = []
        if self._current().kind == TokenKind.NUMBER and self._current().text == "1":
            previous = self._advance()
            pieces.append(previous.text)
        else:
            previous = self._expect_identifier("unit name")
            pieces.append(previous.text)
        if self._unit_exponent_follows(previous):
            previous = self._advance()
            sign = "-" if self._match(TokenKind.MINUS) else ("+" if self._match(TokenKind.PLUS) else "")
            exponent = self._expect(TokenKind.NUMBER, "integer unit exponent")
            if not exponent.text.isdecimal(): self._error("invalid_unit_exponent", "Unit exponents must be integer literals.", exponent.span)
            pieces.extend(("^", sign + exponent.text))
            previous = exponent
        while self._unit_operator_follows(previous):
            operator = self._advance(); pieces.append(operator.text)
            previous = self._expect_identifier("unit name")
            pieces.append(previous.text)
            if self._unit_exponent_follows(previous):
                self._advance()
                sign = "-" if self._match(TokenKind.MINUS) else ("+" if self._match(TokenKind.PLUS) else "")
                exponent = self._expect(TokenKind.NUMBER, "integer unit exponent")
                if not exponent.text.isdecimal(): self._error("invalid_unit_exponent", "Unit exponents must be integer literals.", exponent.span)
                pieces.extend(("^", sign + exponent.text))
                previous = exponent
        return "".join(pieces)

    def _unit_exponent_follows(self, token: Token) -> bool:
        if self._current().kind != TokenKind.CARET:
            return False
        caret = self._current()
        exponent_index = self.index + 1
        if exponent_index >= len(self.tokens):
            return False
        exponent = self.tokens[exponent_index]
        if exponent.kind == TokenKind.MINUS or exponent.kind == TokenKind.PLUS:
            exponent_index += 1
            if exponent_index >= len(self.tokens):
                return False
            exponent = self.tokens[exponent_index]
        return token.span.end.offset == caret.span.start.offset and caret.span.end.offset == exponent.span.start.offset and exponent.kind == TokenKind.NUMBER

    def _parse_integer_literal(self) -> int:
        token = self._expect(TokenKind.NUMBER, "integer literal")
        try:
            if any(char in token.text for char in ".eE"): raise ValueError
            return int(token.text, 0) if token.text.lower().startswith("0x") else int(token.text, 10)
        except ValueError:
            self._error("expected_integer", "Expected an integer literal.", token.span)
            return 0

    def _parse_statement_block_for_role(self, role: str) -> list[object]:
        return self._parse_statement_block()

    def _parse_block(self, *, block_kind: str):
        if block_kind == "properties": return self._parse_property_block()
        if block_kind == "algorithm": return self._parse_statement_block()
        self._expect(TokenKind.LBRACE, "'{' before block"); values: list[object] = []
        while not self._at(TokenKind.RBRACE, TokenKind.EOF):
            mark = self.index
            try:
                if block_kind == "implementation": values.append(self._parse_implementation_definition())
                elif block_kind == "operatingsystem": values.append(self._parse_os_member())
                else: values.append(self._parse_property())
            except _ParseFailure: self._synchronize()
            if self.index == mark: self._advance()
        self._expect(TokenKind.RBRACE, "'}' after block"); return values

    def _expect(self, kind: TokenKind, description: str) -> Token:
        if self._current().kind == kind: return self._advance()
        token = self._current()
        self._error("expected_token", f"Expected {description}; found {token.text or 'end of file'!r}.", token.span)
        if token.kind not in {TokenKind.RBRACE, TokenKind.RBRACKET, TokenKind.RPAREN, TokenKind.EOF}:
            return Token(kind, "", SourceSpan(self.path, token.span.start, token.span.start))
        raise _ParseFailure

    def _expect_semicolon(self) -> None:
        if self._match(TokenKind.SEMICOLON): return
        token = self._current()
        self._error("expected_semicolon", "Expected ';' after declaration.", token.span)
        if token.kind in {TokenKind.RBRACE, TokenKind.EOF}: return
        self._synchronize()

    def _synchronize(self) -> None:
        depth = 0
        while not self._at(TokenKind.EOF):
            kind = self._current().kind
            if depth == 0 and kind in {TokenKind.RBRACE, TokenKind.RBRACKET, TokenKind.RPAREN}:
                return
            if depth == 0 and kind == TokenKind.SEMICOLON:
                self._advance(); return
            if kind in {TokenKind.LBRACE, TokenKind.LBRACKET, TokenKind.LPAREN}: depth += 1
            elif kind in {TokenKind.RBRACE, TokenKind.RBRACKET, TokenKind.RPAREN}: depth = max(0, depth - 1)
            self._advance()

    def _expect_keyword(self, value: str) -> Token:
        if self._word() == value: return self._advance()
        self._fail("expected_keyword", f"Expected {value!r}.", self._current().span)

    def _expect_identifier(self, description: str) -> Token:
        if self._current().kind == TokenKind.IDENTIFIER: return self._advance()
        self._fail("expected_identifier", f"Expected {description}.", self._current().span)

    def _claim_section(self, seen: set[str], section: str) -> None:
        if section in seen: self._error("duplicate_section", f"The {section} section may appear only once.", self._current().span)
        seen.add(section)

    def _require_before_runtime(self, runtime_seen: bool, description: str) -> None:
        if runtime_seen: self._error("declaration_after_operatingsystem", f"{description.capitalize()} declarations must precede operatingsystem.", self._current().span)

    def _parse_project_declaration(self):
        return self._parse_project()

    def _parse_declaration(self):
        word = self._word()
        if word in _ROLE_KEYWORDS or word in {"custom", "hybrid", "subsystem", "model"}: return self._parse_constructor_signature()
        if word in _PROJECT_TYPES: return self._parse_project_type()
        self._fail("unknown_declaration", f"Unknown project declaration {word!r}.", self._current().span)

    def _parse_registered_module_signature(self):
        return self._parse_constructor_signature()

    def _parse_match_pattern(self):
        return self._parse_reference()

    def _parse_type_reference(self):
        return self._parse_type_spec()

    def _parse_matrix_or_vector_literal(self):
        return self._parse_matrix_or_vector()

    def _parse_map_or_select(self, builtin_id: str):
        return self._parse_dsl_builtin_call(builtin_id)

    def _parse_property_or_signal(self):
        return self._parse_property()

    def _number_value(self, token: Token) -> int | float:
        try:
            if token.text.lower().startswith("0x"): return int(token.text, 16)
            if "." not in token.text and "e" not in token.text.lower(): return int(token.text, 10)
            return float(token.text)
        except ValueError:
            self._error("invalid_number", "Numeric literals must be finite decimal or hexadecimal values.", token.span)
            return 0

    def _decode_string(self, spelling: str) -> str:
        if len(spelling) < 2: return ""
        body = spelling[1:-1]
        mapping = {"\\\\": "\\", "\\\"": "\"", "\\'": "'", "\\n": "\n", "\\r": "\r", "\\t": "\t"}
        for source, value in mapping.items(): body = body.replace(source, value)
        return body

    def _literal_integer(self, value: object) -> int | None:
        if isinstance(value, Literal) and isinstance(value.value, int) and not isinstance(value.value, bool): return value.value
        return None

    def _literal_name(self, value: object) -> str | None:
        if isinstance(value, Identifier): return value.value
        if isinstance(value, Literal) and isinstance(value.value, str): return value.value
        return None

    def _current(self) -> Token:
        self._skip_newlines()
        return self.tokens[min(self.index, len(self.tokens) - 1)]

    def _previous(self) -> Token:
        return self.tokens[max(0, self.index - 1)]

    def _peek_kind(self, distance: int) -> TokenKind:
        index = self.index + distance
        while index < len(self.tokens) and self.tokens[index].kind == TokenKind.NEWLINE: index += 1
        return self.tokens[min(index, len(self.tokens) - 1)].kind

    def _peek_text(self, distance: int) -> str:
        """Return the spelling at a lookahead offset, ignoring newlines."""
        index = self.index
        remaining = distance
        while index < len(self.tokens) and remaining:
            index += 1
            if index < len(self.tokens) and self.tokens[index].kind != TokenKind.NEWLINE:
                remaining -= 1
        while index < len(self.tokens) and self.tokens[index].kind == TokenKind.NEWLINE:
            index += 1
        return self.tokens[min(index, len(self.tokens) - 1)].text

    def _word(self) -> str:
        token = self._current()
        return token.text if token.kind == TokenKind.IDENTIFIER else ""

    def _at(self, *kinds: TokenKind) -> bool:
        return self._current().kind in kinds

    def _match(self, kind: TokenKind) -> bool:
        if self._at(kind): self._advance(); return True
        return False

    def _match_keyword(self, value: str) -> bool:
        if self._word() == value: self._advance(); return True
        return False

    def _advance(self) -> Token:
        self._skip_newlines()
        token = self.tokens[min(self.index, len(self.tokens) - 1)]
        if token.kind != TokenKind.EOF: self.index += 1
        return token

    def _skip_newlines(self) -> None:
        while self.index < len(self.tokens) and self.tokens[self.index].kind == TokenKind.NEWLINE: self.index += 1

    def _join(self, start: SourceSpan, end: SourceSpan) -> SourceSpan:
        return SourceSpan(self.path or start.path, start.start, end.end)

    def _error(self, code: str, message: str, span: SourceSpan | None) -> None:
        self.diagnostics.append(source_error(code, message, span))

    def _fail(self, code: str, message: str, span: SourceSpan | None) -> None:
        self._error(code, message, span)
        raise _ParseFailure
