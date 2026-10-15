# Future capability catalogue

`registry.json` is the planning index for capability gaps whose design has
been made concrete enough to review but is not yet an implemented UETD or
`uef-gen` capability. Each entry is deliberately marked `design_only` and
`generation: blocked`. The normal module resolver must not treat this index as
a provider registry, and ordinary project completion must not offer these
features as selectable.

An entry links its contract document, an optional type-level API outline, and
the shared record schemas where applicable. API outlines under `include/*/future`
are not included by public umbrella headers and are not part of the active
CMake sources. They define intended data ownership and operation boundaries;
they do not promise linkable functions.

Promotion requires all of the following, as appropriate to the capability:

1. A reviewed stable contract with exact bounds, units, layout/encoding,
   execution context, ownership, concurrency and failure semantics.
2. A registry schema and implementation manifest with accurate dependency
   closure and explicit target capability/resource mapping.
3. Implementations and generated templates that preserve the same contract;
   unsupported target combinations fail during resolution.
4. Host tests and target evidence for timing, memory, errors, recovery and
   applicable safety/security properties, linked to exact source, artifact,
   configuration, board and toolchain identities.
5. A deliberate promotion that changes the entry out of this future index and
   into the normal selectable registry. A future entry alone cannot support
   an equivalence or qualification claim.

Use `uef-gen`'s `load_future_catalogue` only for roadmap inspection and link
integrity. It validates that the index, schemas and referenced design files
exist; it does not mean a contract is implemented or suitable for generation.
