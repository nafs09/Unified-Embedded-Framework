# UPHYSICS physical quantities and constants

UPHYSICS is reserved for physical constants and unit-aware physical value APIs.

## Contents

- The current header is <code>constants.h</code>, a future SI-constant catalogue with its scope and source described in [the constants design](../../docs/design/UPHYSICS_CONSTANTS.md).
- No stable UPHYSICS umbrella or unit-aware quantity API is currently published; treat the future header as an outline rather than an established runtime interface.

## Files by scope

Each row describes one header. The file link opens its source; the overview summarizes its purpose, and key declarations list the principal API symbols.

Type names may be defined in the linked header or supplied by an included dependency. Status guide: **Public contract** describes an API surface but does not guarantee every target has an implementation; **Future / planned** is a design or backlog header; **Fail-closed scaffold** declares development operations that are not implemented; **Target-family scaffold** is not a part or board profile; **Internal helper** is not an application-facing include; **Build configuration** supplies compile-time selection and capability macros.

| Scope | File | Overview | Key declarations and details | Status |
|---|---|---|---|---|
| module root | [constants.h](constants.h) | Future SI physical-constant catalogue; not available for generation. | Includes: <code>umath/config.h</code>, <code>umath/constants.h</code> | Future / planned |

## Header file structure

This tree lists every .h file in the module and is refreshed from the current include directory.

~~~text
uphysics/
└── future/
    └── constants.h
~~~

For the full public-header index and Doxygen report instructions, see the [UETD include README](../README.md).
