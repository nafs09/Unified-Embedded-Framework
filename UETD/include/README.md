# UETD public headers

This directory is the public C header tree for UETD. <code>uetd.h</code> is the project-wide umbrella for the supported APIs it includes; each module README describes its purpose and includes a scope-grouped file inventory (overview, key declarations, and status) plus a complete header-file tree. Topic folders group related APIs. Most later interfaces remain under family-specific <code>future/</code> folders; a lone later outline may share its topic folder, with scope recorded in its module manifest. Scope placement does not establish readiness; check their own Doxygen contract and module notes before treating them as stable APIs.

## Modules

| Module | Contents | Module guide |
|---|---|---|
| UACTUATOR | Typed actuator command/authority/safe-output scaffold | [uactuator/README.md](uactuator/README.md) |
| USEC | Shared security-provider scaffold | [usec/README.md](usec/README.md) |
| UAPP | Application lifecycle, state machines, supervision, faults, and watchdog gating | [uapp/README.md](uapp/README.md) |
| UCON | Control, estimation, filtering, trajectory, and related algorithm APIs | [ucon/README.md](ucon/README.md) |
| UCORE | Shared types, statuses, limits, time, assertions, bit operations, and checksums | [ucore/README.md](ucore/README.md) |
| UHAL | Hardware abstraction, target configuration, and target-family profiles | [uhal/README.md](uhal/README.md) |
| UMATH | Numerical types, algebra, geometry, probability, signal, and numerical methods | [umath/README.md](umath/README.md) |
| UMID | Middleware services and sensor-facing driver interfaces | [umid/README.md](umid/README.md) |
| UOS | Operating-system and bare-metal execution abstraction | [uos/README.md](uos/README.md) |
| UPAL | Portable peripheral-access APIs | [upal/README.md](upal/README.md) |
| UPHYSICS | Physical constants and planned unit-aware APIs | [uphysics/README.md](uphysics/README.md) |
| UPROTO | Portable communication-protocol APIs | [uproto/README.md](uproto/README.md) |

The project-wide umbrella is [<code>uetd.h</code>](uetd.h). Include a module-specific header when an application needs only one capability.

## Project umbrella

| Scope | File | Overview | Details | Status |
|---|---|---|---|---|
| include root | [uetd.h](uetd.h) | Public umbrella for the selected layer headers and registered UCON algorithms. | Header visibility does not establish implementation, generation or target readiness. Includes UAPP, UCON, UCORE, UHAL, UMATH, UMID, UOS, UPAL, and UPROTO entry headers; excludes future-only header groups. | Project umbrella |

## Generating the API reference

Install Doxygen 1.9 or newer and make <code>doxygen</code> available on <code>PATH</code>. From the repository root, run:

~~~powershell
.\UETD\tools\generate_api_docs.ps1
~~~

The script uses [<code>Doxyfile</code>](../Doxyfile) and writes:

- HTML report: <code>build/UETD/doxygen/html/index.html</code>
- XML output: <code>build/UETD/doxygen/xml/</code>
- Warning log: <code>build/UETD/doxygen/warnings.log</code>

To generate the same reports through CMake, configure with <code>-DUETD_BUILD_DOCS=ON</code> and build the <code>uetd-api-docs</code> target. See [the UETD API documentation section](../README.md#api-documentation) for setup details.
