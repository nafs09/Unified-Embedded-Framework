# UETD source architecture and readiness

UETD is an allocation-conscious C11 framework for embedded applications. It
provides portable source layers, public C APIs, target/module registries, and
fail-closed outlines where implementation or hardware evidence is incomplete.

## Ownership and dependency direction

Portable framework code depends on public interfaces from lower layers and
must not include vendor device headers. `UETD_TARGET` selects one UHAL backend;
`UETD_UOS_BACKEND` selects one operating-system backend. Concrete part,
package, board, and clock details belong in target data and generated board
configuration rather than generic middleware or algorithms.

## Main source map

| Layer | Main responsibility | Paths |
|---|---|---|
| UCORE | Scalar storage, status, assertions, limits, and time types | `include/ucore`, `src/ucore` |
| UMATH | Scalar, fixed-point, vector, matrix, geometry, and numerical APIs | `include/umath`, `src/umath` |
| UHAL | CPU, GPIO, IRQ, memory, and target boundaries | `include/uhal`, `src/uhal` |
| UPAL | Peripheral interfaces and drivers | `include/upal`, `src/upal` |
| UOS | Bare-metal and FreeRTOS abstractions | `include/uos`, `src/uos` |
| UMID | Sensor interfaces, logging, health, and storage adapters | `include/umid`, `src/umid` |
| UPROTO | Protocol APIs and portable protocol logic | `include/uproto`, `src/uproto` |
| UAPP | Component lifecycle, supervision, and fault handling | `include/uapp`, `src/uapp` |
| UCON | Control and estimation APIs and algorithms | `include/ucon`, `src/ucon` |

## Targets and module selection

The target-family and exact-part registries live under
`registry/targets/`. Development-board records bind a board to a chip and
module configuration. `uef-gen` reads these records at runtime; it does not
keep a second target database. Physical generation remains blocked until the
selected chip and board have sufficient verified resource, startup, linker,
and build information.

The module index at `registry/modules/registry.json` points to individual
module manifests. Manifests declare source files, public headers, dependencies,
and availability. Generated module-route files are planning checklists; they
are not renderable code templates. Placeholder modules remain unavailable to
firmware generation.

## Algorithms and numerical APIs

UMATH uses caller-owned storage and documents scalar, matrix-layout, and
quaternion conventions in its public headers and catalogue. Implemented
operations and scaffold status are listed in `catalogues/UMATH_CATALOGUE.md`.

UCON provides generic PID and lead-lag implementations for direct C use.
Other catalogue algorithms have family-organized API/source outlines whose
functions fail closed with `UCON_NOT_IMPLEMENTED`. A scaffold or planned
catalogue entry does not mean an algorithm is usable. Promote an algorithm
only after its types, dimensions, units, equations, numerical bounds, failure
behavior, memory use, and reset policy are reviewed and implemented.

## Build and readiness

CMake builds portable layer sources and selects one UHAL backend and one UOS
backend. FreeRTOS Kernel is staged under `third_party/` and selected through
its lock and integration profile. FatFS remains consumer supplied. Host builds
and source inspection do not establish embedded timing, electrical behavior,
interrupt safety, or support for a specific board.

See the [UETD README](../README.md) for current status and the
[UETD documentation index](README.md) for audits and engineering references.
