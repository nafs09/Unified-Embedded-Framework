"""Conservative source formatting shared by the CLI and both LSP clients."""

from __future__ import annotations

from uef_gen.language.frontends.project.lexer import Lexer


def format_source(source: str, *, tab_size: int = 2, insert_spaces: bool = True,
                  trim_trailing: bool = True, insert_final_newline: bool | None = None,
                  trim_final_newlines: bool | None = None) -> str:
    """Indent blocks and continued expressions without rewriting tokens or comments.

    Keep the original newline convention and final newline unless the caller
    explicitly requests otherwise. Unterminated strings are left untouched so
    formatting an incomplete buffer cannot change its literal contents.
    """
    if not source:
        return source
    tokens, diagnostics = Lexer(source).tokenize()
    open_string_lines = {item.span.start.line - 1 for item in diagnostics
                         if item.code == "unterminated_string" and item.span is not None}
    by_line: dict[int, list] = {}
    for token in tokens:
        if token.kind.value not in {"newline", "eof"}:
            by_line.setdefault(token.span.start.line - 1, []).append(token)
    newline = "\r\n" if "\r\n" in source else "\r" if "\r" in source else "\n"
    lines = source.replace("\r\n", "\n").replace("\r", "\n").split("\n")
    indent = " " * max(1, min(tab_size, 16)) if insert_spaces else "\t"
    pairs = {"}": "{", ")": "(", "]": "["}
    stack: list[str] = []
    rendered: list[str] = []
    for number, line in enumerate(lines):
        line_tokens = by_line.get(number, [])
        leading = list(stack)
        for token in line_tokens:
            closing = pairs.get(token.kind.value)
            if closing is None or not leading or leading[-1] != closing:
                break
            leading.pop()
        content = line.lstrip(" \t")
        # A lexically invalid string can consume trailing spaces as literal text.
        if trim_trailing and number not in open_string_lines:
            content = content.rstrip(" \t")
        rendered.append((indent * len(leading) + content) if content else "")
        for token in line_tokens:
            kind = token.kind.value
            if kind in pairs.values():
                stack.append(kind)
            elif kind in pairs and stack and stack[-1] == pairs[kind]:
                stack.pop()
    output = newline.join(rendered)
    if trim_final_newlines:
        had_newline = output.endswith(("\n", "\r"))
        output = output.rstrip("\r\n")
        if had_newline and output:
            output += newline
    if insert_final_newline and output and not output.endswith(("\n", "\r")):
        output += newline
    return output
