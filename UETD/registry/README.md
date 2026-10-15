# UETD registries

Every UETD catalogue lives under this directory. Catalogues with separate
per-record files use a `registry.json` index and a manifest tree; compact
catalogues such as types, UMATH functions, and units keep their records in one
index file. Schemas define the shape of indexes and records where declared.
`api.json` is the public path index consumed by tools such as `uef-gen`, not a
duplicate listing of every record.


For the full UEF-PL discovery flow and a decision guide for which JSON to
change, see the repository's [UEF-PL registries and discovery guide](../../docs/guides/UEF_PL_REGISTRIES.md).

```text
registry/
  targets/
    families/           registry.json + manifest/<vendor>/<family>.json
    chips/              registry.json + manifest/<vendor>/<part>.json
    development_boards/  registry.json + manifest/<vendor>/<board>.json
    toolchains/          registry.json + manifest/<toolchain>.json
  modules/               registry.json + manifest/<UETD module path>.json
  types/                 public-header-derived type index + schema
  umath/                 public UMATH function index + schema
  units/                 project-language dimensions and unit scales
  future/                 design-only cross-layer capability catalogue and schema
```

To add a target/module record, add its detail manifest and one entry in that
catalogue's index. Shared catalogue policy belongs in the index; record-specific
facts belong in the manifest. Do not create duplicate aliases or catalogues.
The top-level `api.json` links the type, unit, and UMATH catalogues used by language
tooling. Their C declarations are generated from `include/**/*.h`, while
module readiness continues to come from the ordinary module manifests.

### Project-language type and UMATH indexes

`types/registry.json` indexes public `typedef` declarations from the UETD public
headers, including the foundational aliases in `ucore/types.h`, configured
UMATH scalar aliases from `umath/config.h`, value structs, and enum members.
The index is a source map, not a claim that every public driver handle is a
project-language value. The language's built-in scalar spellings map to UETD's
fixed-width aliases; all other by-value use is controlled by the owning UETD
module contract.

The `umath_vec2_t`, `umath_vec3_t`, and `umath_vec4_t` entries have explicit
`dsl_name` aliases and a `fixed_numeric_vector` language contract. Their fixed
component count is authored as `vector_dimension`; the selected numeric profile
provides each component's scalar representation. UEF-PL may attach a registered
unit to the whole vector (`vec3<rad/s>`), but the contract does not expose C
members such as `x`, `y`, and `z`. `umath_quaternion_t` is discoverable through
the `quaternion` alias and remains opaque until UETD publishes a field or
operation contract for it.

`umath/registry.json` indexes public `umath_*` prototypes and inline functions
from UMATH headers. The generator removes a known UETD type/family prefix from
each symbol to derive its canonical operation name (`umath_vec3_cross`
becomes `cross`). It preserves the family-qualified spelling (`vec3_cross`) as a
compatibility alias in that same UETD entry. Functions with the same operation
name form one overload set; the compiler selects a single entry from the
reviewed logical parameter types, semantic/C declaration kinds, generic type
arguments, numeric representation, units, and shapes. The same matcher handles
scalars, fixed-point values, vectors, matrices, nominal UETD types, and generic
language values; there is no vector-specific dispatch path or per-family
Python switch. The selected C symbol remains hidden from source and is carried
into the backend input after semantic resolution.
A parameter type descriptor may use an exact DSL/C type name or one of the
generic categories `numeric_scalar`, `scalar`, `real`, `numeric`, `integer`,
`float`, `fixed`, `boolean`, `vector`, `matrix`, `array`, `tensor`, `enum`,
`record`, `variant`, `flags`, or `opaque`. It can also constrain `signed`,
`storage_bits`, `unit`, and symbolic `shape`, or recursively describe generic
type `arguments`. These constraints are conjunctive; no implicit conversion
is used to make an overload match.
A C signature alone does not establish the
DSL-visible argument/result shape, pointer direction, buffer bounds, aliasing,
or failure behavior. Each callable entry therefore needs a reviewed
`dsl_binding_contract` in this registry. A `reviewed` contract may supply the
logical signature first; tooling can select and explain it, but generation
remains disabled. A `ready` contract must additionally declare failure
behavior, availability, bounded cost, and the exact C ABI mapping. The generic
backend maps that typed contract to the C ABI, allocates statically bounded
output/scratch storage, and
propagates UETD status failures through the enclosing algorithm/model operation.
The optional `operators` list in that same binding declares exact operator
spellings that can use its typed signature as an overload. The checker rejects
multiple matches and applies the same context, readiness, and failure rules to
operation names, compatibility aliases, and explicitly registered operator
overloads. Operators without a matching registered overload keep their
language-defined arithmetic behavior. Scalar arithmetic continues to use the
project's numeric profile unless its UETD contract explicitly binds it to a
checked operation.
Only reviewed functions in implemented UETD modules are callable, and only in
custom algorithm or nonlinear model bodies. Scaffold functions remain visible
for editor hints but carry a warning until their binding is ready.

`units/registry.json` defines the dimensions, symbols, exact decimal or
rational-times-pi scales, and affine offsets used by type checking. Compound
units are formed from registered symbols and operators. Runtime conversion is
always explicit through the checked `convert_unit<TARGET_UNIT>(value)` form;
affine units such as Celsius are only valid as standalone absolute values.

After changing a public header or UMATH module manifest, regenerate the
header-derived declarations and registry reference pages with:

```powershell
python UETD/tools/generate_project_language_catalogues.py
```

The generator uses only Python's standard library. It rebuilds
`registry/types/registry.json`, refreshes C declaration facts in
`registry/umath/registry.json`, and writes the generated language catalogue and
dimension-grouped unit reference under `docs/reference/`. It does not rewrite
the human-maintained handbook or technical guide. For UMATH entries,
`dsl_aliases`, `dsl_binding_status`, `dsl_signature`, and
`dsl_binding_contract` are UETD-owned authoring metadata preserved across
regeneration; edit them in the single `umath/registry.json` index. The canonical
operation name is derived from the UETD function family and C symbol; aliases
and callable binding details remain authored metadata. Do not edit C signature
fields by hand. Type-index records are regenerated from headers,
so do not use that index as a hand-maintained place for module usability rules.
Edit unit definitions in `registry/units/registry.json` and keep its schema
valid. For the public explanation of JSON ownership and discovery, see the
[UEF-PL registries and discovery guide](../../docs/guides/UEF_PL_REGISTRIES.md).

For the first-time source walkthrough, read the
[UEF-PL user handbook](../../docs/guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md).
The [technical language guide](../../docs/guides/UEF_PROJECT_LANGUAGE_GUIDE.md)
documents the source contract; the generated inventories are in the
[catalogue reference](../../docs/reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md)
and [unit reference](../../docs/reference/UEF_PL_UNITS_REFERENCE.md).

`future/registry.json` indexes accepted design work across UETD and uef-gen. It
links to the owning future contract and marks every entry `design_only` with
generation blocked. Promote an item into the selectable module registry only
after its public API, source ownership, target constraints, dependency closure,
and generation route have been reviewed. The future index keeps planned work
discoverable without allowing generated projects to select it.

## Module identifiers in `.uef`

`registry/modules/registry.json` is the sole module ID and language descriptor
index. Every directly authorable module gets one unique, case-sensitive
`DSL_descriptor`. Role-specific source uses that descriptor after the role,
for example `sensor left_wheel : encoder;` or
`algorithm rate : pid;`. A module with no dedicated role is written directly
as its own identifier, such as `watchdog safety_timer;` or `fpga coil_link;`.
There is no generic `component` declaration and no descriptor allowlist in
uef-gen.

The descriptor makes a module discoverable; it does not assert implementation
readiness. Its linked manifest supplies authoring kind, fields, ports,
operations, dependencies, target/runtime/numeric limits, readiness, and the
code-generation route. Missing contract details should be reported against
the UETD manifest while the name remains visible to editor completion.

### Registering an authorable module

1. Add/update the module files and dependencies in its existing manifest and
   index the manifest once in `registry/modules/registry.json`.
2. Add the module's unique `DSL_descriptor` to its module-index entry. Use the
   short spelling that should appear in source; do not repeat it in the
   authoring contract.
3. Add an authoring contract to the manifest with the correct kind, typed
   settings and ports, operations, target/runtime/numeric/scheduling bounds,
   composition capabilities where applicable, and generation route.
4. Keep authoring and generation readiness at `draft` until the generated UETD
   closure, target build, and runtime behavior are implemented and reviewed.
5. Add or update examples using the regular language role or direct identifier.
   Do not add a parser branch or copy the UETD descriptor into uef-gen.

A UETD-native algorithm remains an algorithm even when the standalone DSL
composes it inside `hybrid algorithm`. For example, `enmpc` is the explicit
nonlinear-MPC policy-table algorithm; `nmpc_rti` is the online RTI-SQP
algorithm. A project-level hybrid declares the typed interface, member
bindings, table-validity/SQP selection policy, failure behavior, and execution
budget. Neither UETD module is itself a hybrid strategy. Their target-specific
implementations and readiness remain independent of this language-level
composition.

`registry/modules/authoring_contract.schema.json` defines the UETD manifest
contract. Its `hybrid_strategy` kind remains available for a future UETD module
that truly implements a reusable strategy, but `hybrid algorithm` syntax does
not require one.

## Target records

Chip manifests own silicon/package facts. Development-board records bind a
chip to board clocks, reserved pins, build/flash settings, and optional
on-board memory/peripheral data. General development boards have their own
`development_boards/` catalogue; `modules/` remains the UETD software-module
catalogue. Add a family record only when introducing a new silicon family.
