"""Token model shared by the lexer, parser, and future language server."""

from __future__ import annotations
from dataclasses import dataclass
from enum import Enum
from uef_gen.language.frontends.project.ast import SourcePosition, SourceSpan

class TokenKind(str, Enum):
    IDENTIFIER = "identifier"
    STRING = "string"
    NUMBER = "number"
    LBRACE = "{"
    RBRACE = "}"
    LBRACKET = "["
    RBRACKET = "]"
    LPAREN = "("
    RPAREN = ")"
    COLON = ":"
    COMMA = ","
    RANGE = ".."
    DOT = "."
    EQUAL = "="
    EQUAL_EQUAL = "=="
    NOT_EQUAL = "!="
    LESS = "<"
    LESS_EQUAL = "<="
    GREATER = ">"
    GREATER_EQUAL = ">="
    ARROW = "->"
    AT = "@"
    SEMICOLON = ";"
    PIPE = "|"
    AMPERSAND = "&"
    PLUS = "+"
    MINUS = "-"
    STAR = "*"
    POWER = "**"
    SLASH = "/"
    # Caret belongs only to compile-time unit exponents (m/s^2).
    CARET = "^"
    NEWLINE = "newline"
    EOF = "eof"

@dataclass(frozen=True)
class Token:
    """One token retaining raw spelling and exact source range."""

    kind: TokenKind
    text: str
    span: SourceSpan
