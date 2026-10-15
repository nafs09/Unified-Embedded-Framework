"""Deterministic single-pass lexer for standalone UETD project source."""

from __future__ import annotations

from pathlib import Path

from uef_gen.language.frontends.project.ast import SourcePosition, SourceSpan
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic, source_error
from uef_gen.language.frontends.project.tokens import Token, TokenKind

_ASCII_LETTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
_ASCII_DIGITS = "0123456789"
_HEX_DIGITS = "0123456789abcdefABCDEF"
_STRING_ESCAPES = {"\\", '"', "'", "n", "r", "t"}

_TWO_CHARACTER_TOKENS = {
    "**": TokenKind.POWER,
    "->": TokenKind.ARROW,
    "==": TokenKind.EQUAL_EQUAL,
    "!=": TokenKind.NOT_EQUAL,
    "<=": TokenKind.LESS_EQUAL,
    ">=": TokenKind.GREATER_EQUAL,
    "..": TokenKind.RANGE,
}

_ONE_CHARACTER_TOKENS = {
    "{": TokenKind.LBRACE,
    "}": TokenKind.RBRACE,
    "[": TokenKind.LBRACKET,
    "]": TokenKind.RBRACKET,
    "(": TokenKind.LPAREN,
    ")": TokenKind.RPAREN,
    ":": TokenKind.COLON,
    ",": TokenKind.COMMA,
    ".": TokenKind.DOT,
    "=": TokenKind.EQUAL,
    "<": TokenKind.LESS,
    ">": TokenKind.GREATER,
    "@": TokenKind.AT,
    ";": TokenKind.SEMICOLON,
    "|": TokenKind.PIPE,
    "&": TokenKind.AMPERSAND,
    "+": TokenKind.PLUS,
    "-": TokenKind.MINUS,
    "*": TokenKind.STAR,
    "/": TokenKind.SLASH,
    "^": TokenKind.CARET,
}


class Lexer:
    """Tokenize one project source while collecting recoverable diagnostics.

    Offsets and columns count Python Unicode code points. Lines and columns
    are one-based, and spans are half-open. CRLF is one logical line break.
    """

    def __init__(self, source: str, path: Path | None = None) -> None:
        self.source = source
        self.path = path
        self.offset = 0
        self.line = 1
        self.column = 1
        self.diagnostics: list[LanguageDiagnostic] = []

    def tokenize(self) -> tuple[list[Token], list[LanguageDiagnostic]]:
        """Return every token, followed by exactly one zero-width EOF token."""

        tokens: list[Token] = []
        while self.offset < len(self.source):
            char = self._peek()

            if char in " \t\v\f":
                self._advance()
                continue

            start = self._position()
            start_offset = self.offset

            if char in "\r\n":
                self._advance()
                tokens.append(
                    Token(
                        TokenKind.NEWLINE,
                        self.source[start_offset : self.offset],
                        self._span_from(start),
                    )
                )
                continue

            if char == "#" or (char == "/" and self._peek(1) == "/"):
                self._skip_comment()
                continue

            if self._is_identifier_start(char):
                tokens.append(self._scan_identifier())
                continue

            next_char = self._peek(1)
            if char in _ASCII_DIGITS or (
                char == "." and next_char != "" and next_char in _ASCII_DIGITS
            ):
                tokens.append(self._scan_number())
                continue

            if char in ('"', "'"):
                tokens.append(self._scan_string())
                continue

            two_character = char + self._peek(1)
            two_kind = _TWO_CHARACTER_TOKENS.get(two_character)
            if two_kind is not None:
                self._advance()
                self._advance()
                tokens.append(Token(two_kind, two_character, self._span_from(start)))
                continue

            one_kind = _ONE_CHARACTER_TOKENS.get(char)
            if one_kind is not None:
                self._advance()
                tokens.append(Token(one_kind, char, self._span_from(start)))
                continue

            unexpected = self._advance()
            self._report(
                "unexpected_character",
                f"Unexpected character {unexpected!r} in UEF-PL source.",
                self._span_from(start),
            )

        eof = self._position()
        tokens.append(Token(TokenKind.EOF, "", SourceSpan(self.path, eof, eof)))
        return tokens, self.diagnostics

    def _scan_identifier(self) -> Token:
        """Scan an ASCII identifier; keyword/registry resolution happens later."""

        start = self._position()
        start_offset = self.offset
        self._advance()
        while self._is_identifier_continue(self._peek()):
            self._advance()
        return Token(
            TokenKind.IDENTIFIER,
            self.source[start_offset : self.offset],
            self._span_from(start),
        )

    def _scan_number(self) -> Token:
        """Scan decimal/scientific or hexadecimal numbers, leaving units separate."""

        start = self._position()
        start_offset = self.offset

        if self._peek() == "0" and self._peek(1) in ("x", "X"):
            self._advance()
            self._advance()
            digit_start = self.offset
            while (digit := self._peek()) and digit in _HEX_DIGITS:
                self._advance()
            if self.offset == digit_start:
                self._report(
                    "malformed_hex_literal",
                    "A hexadecimal integer must contain at least one digit after 0x.",
                    self._span_from(start),
                )
            return Token(
                TokenKind.NUMBER,
                self.source[start_offset : self.offset],
                self._span_from(start),
            )

        if self._peek() == ".":
            self._advance()
            self._consume_ascii_digits()
        else:
            self._consume_ascii_digits()
            # Do not consume the first dot of the half-open range operator.
            if self._peek() == "." and self._peek(1) != ".":
                self._advance()
                self._consume_ascii_digits()

        if self._peek() in ("e", "E"):
            exponent_start = self._position()
            self._advance()
            if self._peek() in ("+", "-"):
                self._advance()
            digit_start = self.offset
            self._consume_ascii_digits()
            if self.offset == digit_start:
                self._report(
                    "malformed_exponent",
                    "A scientific exponent must contain at least one digit.",
                    SourceSpan(self.path, exponent_start, self._position()),
                )

        return Token(
            TokenKind.NUMBER,
            self.source[start_offset : self.offset],
            self._span_from(start),
        )

    def _scan_string(self) -> Token:
        """Scan a quoted string without evaluating or decoding its contents."""

        start = self._position()
        start_offset = self.offset
        quote = self._advance()
        closed = False

        while self.offset < len(self.source):
            char = self._peek()
            if char in "\r\n":
                break
            if char == quote:
                self._advance()
                closed = True
                break
            if char != "\\":
                self._advance()
                continue

            escape_start = self._position()
            self._advance()  # backslash
            escaped = self._peek()
            if escaped == "" or escaped in "\r\n":
                break
            self._advance()
            if escaped not in _STRING_ESCAPES:
                self._report(
                    "invalid_string_escape",
                    f"Invalid escape sequence for character {escaped!r}.",
                    SourceSpan(self.path, escape_start, self._position()),
                )

        if not closed:
            if (
                self.offset >= len(self.source)
                and self.offset > start_offset
                and self.source[self.offset - 1] == "\\"
            ):
                reason = "String ends with an incomplete escape sequence."
            else:
                reason = "String must close before the end of the line or file."
            self._report("unterminated_string", reason, self._span_from(start))

        return Token(
            TokenKind.STRING,
            self.source[start_offset : self.offset],
            self._span_from(start),
        )

    def _consume_ascii_digits(self) -> None:
        while (digit := self._peek()) and digit in _ASCII_DIGITS:
            self._advance()

    def _skip_comment(self) -> None:
        """Consume a # or // comment, leaving its newline for tokenization."""

        if self._peek() == "#":
            self._advance()
        else:
            self._advance()
            self._advance()
        while self.offset < len(self.source) and self._peek() not in "\r\n":
            self._advance()

    def _advance(self) -> str:
        """Consume one code point or CRLF pair and update line/column state."""

        if self.offset >= len(self.source):
            return ""
        char = self.source[self.offset]
        self.offset += 1
        if char == "\r":
            if self._peek() == "\n":
                self.offset += 1
            self.line += 1
            self.column = 1
        elif char == "\n":
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        return char

    def _peek(self, offset_delta: int = 0) -> str:
        index = self.offset + offset_delta
        if index < 0 or index >= len(self.source):
            return ""
        return self.source[index]

    def _position(self) -> SourcePosition:
        return SourcePosition(offset=self.offset, line=self.line, column=self.column)

    def _span_from(self, start: SourcePosition) -> SourceSpan:
        return SourceSpan(self.path, start, self._position())

    def _report(self, code: str, message: str, span: SourceSpan) -> None:
        self.diagnostics.append(source_error(code, message, span))

    @staticmethod
    def _is_identifier_start(char: str) -> bool:
        return bool(char) and (char in _ASCII_LETTERS or char == "_")

    @classmethod
    def _is_identifier_continue(cls, char: str) -> bool:
        return bool(char) and (cls._is_identifier_start(char) or char in _ASCII_DIGITS)
