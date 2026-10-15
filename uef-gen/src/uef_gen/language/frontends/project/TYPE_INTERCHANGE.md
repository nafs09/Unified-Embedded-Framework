# UEF-PL type interchange contract

**State:** active frontend/type-interchange contract. `.uef` and `.uefs` now
parse the scalar, record, enum, tagged-variant, nominal-flags, fixed-point,
`Received<T>`, `Option<T>`, `Result<T,E>`, and bounded-buffer type forms. The
project compiler resolves types, checks the implemented expression rules, and
lowers type/boundary descriptors into its application model. This does not
mean a firmware backend can emit every representation: generation must remain
blocked until the selected UETD ABI, codec, and target route are implemented.

This document defines how a value keeps its meaning across the UETD registry,
UEF-PL source, `.uefs` contracts, UMATH bindings, generated C, and
wire/shared/persistent boundaries. It is the type-interchange contract, not a
second UETD type catalogue or a list of methods waiting to be filled in.

## 1. Source ownership and resolution

There are four owners of type facts:

1. **UETD registry.** `UETD/api.json` links the public-header type registry,
   unit/function indexes, module index, authoring manifests, and readiness
   records. The headers establish which declarations exist. Registry metadata
   establishes whether a declaration is a usable UEF-PL value and how it maps
   to a C ABI.
2. **UEF-PL project.** A `.uef` file owns its fixed records, fixed-width enums,
   fixed-shape values, numeric policy, and local type declarations.
3. **UEF-PL system contract.** A `.uefs` file owns types shared by independently
   built nodes, protocol layouts, endpoint/link facts, and cross-node type
   identity.
4. **Language core.** UEF-PL owns `bool`, `Received<T>`, and the additional
   constructors below. Numeric names and their C ABI identities come from
   UETD's type registry; the compiler supplies fixed numeric rules for them.

The project type resolver uses an explicit search order and never guesses:

1. language-core names (`bool`, numeric spellings, and registered generic
   constructors);
2. project-local nominal records, enums, variants, flags, and fixed-point
   formats;
3. declarations explicitly imported from the one `.uefs` contract;
4. UETD types by an explicit UETD `dsl_name`, or by exact `c_name` only when the
   owning UETD contract exposes that type in the current value context.

Any collision or ambiguity is an error. Local declarations cannot shadow
language-core names or UETD `dsl_name`s. A C typedef discovered in a header is
not automatically a legal value. The type index distinguishes numeric aliases,
enumerations with registered cases, and opaque nominal types. An opaque type
keeps its exact C name and header but has no DSL constructor, arithmetic, or
field access. Function pointers and future-header declarations remain
unavailable. A component port can name a registered C struct directly without
duplicating or guessing its C layout in the language frontend.

UETD modules, targets, runtimes, units, and UMATH functions are **implicitly
available through the live UETD registry**. They are not imported by writing a
C header name or a Python/C/C++ source path. `contract import "system.uefs";`
is the only explicit UEF-PL source import in V1. It imports only the selected
system's shared type/protocol declarations and facts for this node's local
endpoints. It does not import sibling implementation code, make remote
components callable, or recursively compile the `.uefs` system.

## 2. Existing type interchange

The UETD type index currently discovers 242 public and future C declarations.
Fourteen carry numeric contracts (`f32`, `f64`, `i8` through `i64`, `u8` through
`u64`, and `q15`, `q16`, `q31`, `q63`); their exact `dsl_name` to C alias mapping
comes from UETD. Public enums and non-function-pointer C types also receive
enum or opaque contracts. Future-header declarations and function pointers
remain unavailable. The number 14 therefore counts numeric spellings, not all
UETD types the compiler can resolve. Do not hard-code a parallel C-type list
in `uef-gen`.

The current project values include finite scalars, fixed records, nominal
enums, tagged variants, flags, fixed-point formats, fixed-shape arrays and
matrices, compatible by-value UETD types, and the language constructors
described below. A numeric unit is part of a value's type. `Received<T>` is a
protocol receiver envelope containing logical `T` plus receiver-local
sequence and timestamp metadata; that metadata is not part of the wire
payload. `string` remains a compile-time configuration value; bounded runtime
text uses `Utf8<N>`.

The base language does not provide implicit numeric, enum/integer, unit,
shape, record, or boundary conversions. Explicit casts and unit conversions
are checked against range, exact UETD unit scale/offset, rounding, and overflow
policy. A type identity must retain at least:

- source spelling and exact source span;
- nominal owner and declaration name, if project/system-defined;
- UETD `dsl_name`, canonical C name, declaration kind, visibility, header, and
  readiness, if UETD-defined;
- scalar representation, shape/bounds, unit/dimension, and affine status;
- the context in which it is legal (value, configuration, port, UMATH ABI,
  protocol, shared memory, or persistence).

The registry is discovery metadata, not the semantic type environment. The
type checker combines the registry with project-local declarations and the
specific module contract that authorizes each non-primitive use. Lowered UETD
type references retain the exact C typedef, header, declaration kind, and
language-contract kind. If field-level access is needed, UETD must publish a
reviewed field contract rather than relying on guessed C parsing.

## 3. Additional language types

### 3.1 `Option<T>` and `Result<T, E>`

These are closed, tagged, statically sized constructors:

- `Option<T>` has exactly `Some(T)` and `None` cases.
- `Result<T, E>` has exactly `Ok(T)` and `Err(E)` cases. It describes a
  completed operation outcome, not asynchronous work.
- `Received<T>` remains separate and is not a substitute for absence or an
  error result. A protocol may carry `Result<T, E>` as its logical payload if
  the protocol explicitly defines both cases.

The constructor expression forms are `Some(value)`, `None`, `Ok(value)`, and
`Err(error)`. `match` is exhaustive unless it ends in an explicit `default`
arm. The pattern binding is scoped only to its arm. Both arms of a value match
must produce the same exact type, unit, and shape. No truthiness, zero-value,
integer-sentinel, enum, or `Received<T>` coercion is allowed.

The first implementation should use explicit `match` handling. Do not add a
postfix propagation operator until custom-algorithm stages have an explicit
result/failure return contract; otherwise there is no deterministic enclosing
return path for propagation to target.

### 3.2 Project-owned tagged variants

Declaration form:

```uetd
variant Measurement : u8 {
    case unavailable = 0;
    case sample = 1 { value : f32<m/s>; }
    case fault = 2 { code : u16; }
}
```

The storage type is one fixed-width integer. Each case has one unique nominal
name and integer tag. Tags may be inferred in declaration order only while the
variant is local and never serialized; protocol, shared-memory, persistent,
or external-component use requires every tag to be explicit. Case payloads are
fixed records; nested dynamic shapes are not allowed. Matching must cover all
cases or use an explicit default arm. Pattern fields are available only in
their arm.

Generated storage is a fixed-width tag plus enough statically allocated bytes
for the largest case, with alignment determined by the reviewed backend
contract. The compiler checks the tag before reading case storage. A native C
`union` layout is never treated as wire or persistent ABI.

### 3.3 Nominal flags

Declaration form:

```uetd
flags HealthFlags : u16 {
    estimate_valid = 0;
    link_fault = 1;
    actuator_inhibited = 2;
}
```

Values are bit positions, not pre-shifted masks. Positions must be unique and
less than the storage width. `|`, `&`, and `^` are valid only between values of
the same nominal flags type. `has(value, HealthFlags.link_fault)`,
`set(value, HealthFlags.link_fault)`, and `clear(value,
HealthFlags.link_fault)` are typed DSL operations. Arithmetic and implicit
integer conversions are errors. There is no unrestricted complement operator
because it would set reserved bits. Raw integer conversion is permitted only
inside a declared register or wire boundary with an explicit allowed-bit mask
and reserved-bit policy.

The parser accepts `|`, `&`, and `^` as flags operators in expressions and
the type checker requires both operands to have the same nominal flags type.
`^` remains a unit exponent only inside type/unit syntax. Boolean conjunction
uses the word `and`; `&&` is not part of the language. Flags values can also
use the typed `has`, `set`, and `clear` forms.

### 3.4 Project fixed-point formats

Declaration form:

```uetd
fixed CoilCommand : i32 {
    fractional_bits = 20;
    unit = V;
    rounding = nearest_even;
    overflow = reject;
}
```

V1 custom fixed-point formats use signed storage and an integer number of
fractional bits. The scale is exactly `2^-fractional_bits`; arbitrary decimal
scale is deferred. The storage range and physical range are compile-time
facts. The declaration names rounding and overflow behavior explicitly.
Arithmetic defines intermediate widening, rescaling, division rounding,
literal conversion, and overflow before code generation; C integer
promotions/overflow and target accumulator width do not define those rules.
The cast is explicit and checked. The generated representation may use a UETD
`q*` alias or a backend-specific integer representation only when range,
scale, rounding, overflow, and ABI semantics are identical.

### 3.5 Bounded bytes and UTF-8

The bounded type forms are `Bytes[32]` and `Utf8[24]`; the bracket value is a
compile-time maximum **encoded byte** count. Both have a fixed storage bound
and a separate validated current length; neither is NUL-terminated by
implication and neither allocates from the heap. `Bytes<N>` is opaque bytes.
`Utf8<N>` stores only well-formed UTF-8; V1 rejects invalid sequences during
construction or decode, and replacement-character behavior is not implicit.
Construction, copy, indexing, truncation/rejection, and boundary encoding have
explicit bounds checks. Truncation may occur only at a codepoint boundary and
only when the boundary contract permits it. Compile-time identifier strings
remain separate.

Protocol use additionally declares encoding, padding/termination, maximum
wire length, and truncation/rejection behavior. A fixed buffer does not imply a
wire format.

## 4. Boundary-specific type interchange

One logical record does not acquire every boundary ABI automatically. The
compiler handles each boundary independently:

| Boundary | Required contract | Never infer |
| --- | --- | --- |
| Local project value | Nominal identity, field types, static shape, unit | Wire layout or cross-core ABI |
| UETD module/UMATH call | Reviewed UETD authoring or DSL binding contract, C name, argument/result mapping, output buffers, aliasing, status/failure behavior, target/readiness | A callable ABI from a C prototype alone |
| Protocol wire message | Message/field IDs, byte/bit offsets, width, endian/bit order, scale, rounding, reserved bits, version, checksum, sequence and receiver policy | C struct layout, padding, or host endianness |
| Shared memory | Schema/version, offsets/alignment, atomic fields, producer/consumer ownership, publication sequence, barriers, cache policy | Wire safety from matching field names |
| Persistent storage | Schema ID/version, canonical encoding, integrity envelope, compatibility/migration policy | Compatibility from a C struct or wire layout |

Shared-memory and persistent declarations are separate language forms, not
attributes that silently change ordinary record layout. Their exact surface
syntax remains a promotion gate: first agree the UETD contract/schema fields,
then add the DSL declaration and backend route. Until then, the parser must
reject these constructs as unsupported rather than accepting partial metadata.

For every boundary, lowering produces a descriptor-owned codec/ABI structure.
Two descriptors may refer to the same nominal record but must not share
offset/layout assumptions unless each boundary independently declares the
same values. Record field ordering is semantically irrelevant for local C
values but is explicit for serialized codecs.

## 5. Import and registry behavior

`contract import "relative/path.uefs";` is resolved relative to the importing
`.uef` source directory. The path must be normalized, remain beneath that
directory, name a regular `.uefs` file, and not traverse through a symlink to
outside the permitted root. Imports are parsed as data; no source is executed.
For V1, allow at most one contract import per `.uef` project. Reject duplicate
or conflicting nominal names instead of adding implicit namespaces.

The imported contract pass exposes only the `.uefs` shared records, enums,
variants, flags, fixed-point types, protocol declarations, and endpoint facts
for the node whose project path matches the importing `.uef`. It does not
compile the other node projects.
This direction is acyclic: `.uefs` may name `.uef` nodes, but a `.uef` import
reads only the system contract section and never re-enters node compilation.

Resolution sources remain separate:

- UETD target/module/runtime/type/unit/UMATH data: API-linked UETD registries;
- DSL syntax forms and constructors: `uef-gen` language registry;
- project-local names: this file's AST and scoped symbol table;
- shared system names: the explicitly imported `.uefs` AST.

The language registry must not become a duplicate UETD type list. Future type
constructors may be listed there for parser/completion dispatch only after
their end-to-end implementation is promoted. The TextMate grammar is
presentation only and is not an import resolver or semantic authority.

## 6. Implemented compiler path and ownership

The supported frontend path is deliberately split into small modules:

1. `lexer.py` tokenizes source and records one-based line/column and code-point
   offsets. It does not resolve identifiers. LSP converts columns to UTF-16 at
   the editor boundary.
2. `parser.py` builds the `.uef` AST, including type constructors, matches,
   flags operations, imports, and recoverable source spans. The `.uefs` parser
   reuses the common type nodes for shared declarations.
3. `contract_imports.py` resolves one safe, relative `.uefs` contract import
   and selects only the node-local contract view.
4. `registry_contracts.py` loads UETD's API-linked type, unit, UMATH, target,
   module, and authoring contract data. `../registry/registry.json`
   owns only UEF-PL syntax and language-level type constructors.
5. `type_interchange.py` builds a provenance-preserving type environment,
   resolves types, infers expressions, and checks exact nominal type, shape,
   unit-dimension, flags, variant, and fixed-point rules. UMATH operation names
   may have multiple UETD entries; the checker requires typed parameters for
   every candidate and selects exactly one by argument types, shape, and unit.
6. `validation.py` runs the compiler's declaration and expression checks and
   returns diagnostics before any lowering is accepted.
7. `lowering.py` stores resolved type descriptors, source maps, imported
   contract provenance, and explicit boundary metadata in
   `application_spec.py`. Each UMATH call also carries its selected C symbol
   and binding route, keyed to its source span. Lowering does not invent an ABI
   from the C header and does not emit target C.
8. `hints.py` and `../server/server.py` adapt the same registries and
   compiler APIs for editor completion, hover, signatures, definitions, and
   diagnostics. They do not implement a second type checker.

The active type and boundary entry points include `build_type_environment`,
`resolve_type_spec`, `resolve_uetd_type`, `infer_expression_type`,
`check_assignable`, `check_explicit_cast`, `check_unit_conversion`,
`check_variant_match`, `check_flags_operation`, `check_fixed_point_operation`,
`validate_type_boundaries`, `build_type_interchange_manifest`, and the focused
lowering functions in `lowering.py`. The public entry points for saved and
in-memory projects are `compile_project_file`, `compile_project_text`, and
`compile_project_document`; the corresponding `.uefs` entry points are in
`../system/compiler.py`.

## 7. Current limits and generation gate

The compiler frontend produces a validated, source-mapped application
specification and, for `.uefs`, an aggregate system build plan. Those are
backend inputs, not firmware artifacts. A target backend still needs to:

- resolve every UETD module/UMATH ABI, target route, template, and source
  closure from UETD's registries;
- emit bounded representations for project values, custom algorithm code,
  model equations, protocol codecs, shared-memory layouts, and persistence
  schemas only where their contracts are complete;
- validate the generated project before publishing files and report a stable
  diagnostic for every missing route.

The CLI reports this limitation explicitly. A descriptor in a type manifest
or application specification is not evidence of generated-C support. Every
boundary has its own ABI; protocol wire layout, shared memory, persistent
storage, and ordinary C values must never share inferred layout assumptions.
