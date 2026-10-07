# UCON public headers

The UCON API is owned by UEF. NEXUS serializes graph nodes into ControlIR;
`uef-gen` supplies those raw nodes and project context to templates declared by
`registry/control_ir/registry.json` and its individual algorithm detail manifests.

## Current direct-use API

- `ucon_status.h` defines common statuses, including `UCON_NOT_IMPLEMENTED` for
  development outlines only.
- Shared scalar, vector, quaternion, complex, and matrix primitives live under `uef/umath/`; UCON algorithm headers include only the math operations they use.
- `ucon_scalar_t` is a compatibility alias for `umath_scalar_t`, declared in `ucon_types.h`.
- `control/pid.h` declares the generic two-degree-of-freedom PID/PI/PD API.
- `control/lead_lag.h` declares the generic first-order bilinear lead-lag API.
- `ucon.h` includes the supported shared and direct-use APIs.

## Algorithm outlines and future backlog

Every unregistered catalogue entry has one family-organized header/source pair.
Each pair contains a named outline of its required module operations. Estimators
separate validation, initialization/reset, prediction, correction, and estimator
step; predictive controllers separate problem preparation, solve, verification,
and command publication; filter design separates specification validation,
coefficient generation, and coefficient publication. Other families have
corresponding operation profiles. Every function has its own TODO and currently
returns `UCON_NOT_IMPLEMENTED` without changing state or output data.

The shared `ucon_algorithm_scaffold_call_t` in `detail/algorithm_call.h` is a
temporary type-erased development boundary. It is not a typed algorithm
contract and is intentionally excluded from `ucon.h`; unregistered entries
cannot be selected by `uef-gen`. Replace it with typed fixed-size configuration
and state only after dimensions, units, equations, numeric policy, memory/work
bounds, and failure behavior have been specified.

First-pass entries and the selected common/project planned entries live under
their normal family paths. Other planned and all deferred entries live under
`future/`. Fifty additional future proposals are listed in
[`../../../ROADMAP_EXTENSIONS.md`](../../../ROADMAP_EXTENSIONS.md). The
full family audit and unresolved specification conflicts are recorded in
[`../../../UCON_CATALOGUE_AUDIT.md`](../../../UCON_CATALOGUE_AUDIT.md).

When implementing an entry, review its function outline against the algorithm
contract, remove inapplicable operations, and add any missing required ones.
Then replace the scaffold with a typed C implementation, update manifest and
module/API metadata, and add a matching UEF-owned template only if generation
needs structural specialization. Keep one algorithm per header/source pair.
