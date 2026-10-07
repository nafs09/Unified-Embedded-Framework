# UCON templates and algorithm catalog

UEF owns reusable algorithm behavior and every reusable UCON template. NEXUS authors the ControlGraph and serializes it as ControlIR. uef-gen resolves the requested algorithm key from UEF's `registry/control_ir/registry.json` index and its per-algorithm detail file, then renders only outputs declared by a registered entry.

## Registered templates

| Algorithm | Template | Generic implementation |
|---|---|---|
| PID / PI / PD / 2-DOF PID | `wrappers/pid.h.tmpl` | `uef/ucon/control/pid.h` |
| First-order lead-lag | `wrappers/lead_lag.h.tmpl` | `uef/ucon/control/lead_lag.h` |

Each template emits a small named instance wrapper and forwards init/reset/step calls to the UEF generic C implementation. It does not synthesize a second control law. The consuming application maps graph ports, supplies validated configuration, and completes any manifest-declared manual actions.
The selected module manifest supplies the generic implementation source and its public-header closure to the generated project; the template output is instance-specific glue, not another copy of the algorithm.

## Unregistered algorithms

Every unregistered catalog key has its own header and source file in a family directory. Initial entries are placed beside implemented APIs under `include/uef/ucon/<family>/` and `src/ucon/<family>/`; for example, Kalman estimators use `estimators/kalman/<algorithm>.h` and `.c`. Later entries are under the corresponding `future/<family>/` path. `estimation/` in the catalog maps to the public directory name `estimators/`.

Each file contains a detailed `TODO(UCON-KEY)`, one function declaration/definition, and a fail-closed body that returns `UCON_NOT_IMPLEMENTED` without mutating state or outputs. The shared development envelope is internal at `include/uef/ucon/detail/algorithm_call.h` and is not included by `ucon.h`. A file existing in the repository does not make its key directly usable or renderable.

The catalog's `FIRST_PASS` entries are in the initial tree. Selected `PLANNED` entries are also in that tree because they are common project building blocks or match known NEXUS projects: standard filter designs and PID autotuning; nonlinear drone estimators/control, trajectory profiles, and safety constraints; QCWDRSSTC predictive-control variants and square-root UKF; and medical PK/PD target control. The exact keys and staging reasons are listed in `uef_api.json` and repeated on each algorithm manifest row. Other planned and all deferred entries remain under `future/`.

All unregistered entries have `registered: false`, `renderable: false`, empty outputs, and a per-algorithm scaffold module marked `scaffold_only`. uef-gen verifies that each row resolves to its own header/source/module and rejects the key as `algorithm_unregistered`; module selection also refuses scaffold-only modules. CMake excludes `src/ucon/future/` by default.

Regenerate the individual placeholder files and synchronized manifest/module/API paths after catalog edits:

```powershell
python tools/generate_ucon_scaffolds.py
```

Do not edit generated algorithm placeholders as the source of truth. To promote a key, replace its placeholder with a typed generic implementation, define dimensions/units/bounds/failure semantics/static memory, add an appropriate UEF-owned wrapper if needed, and update module/API metadata. Set `registered: true` only when the real implementation and every declared output exist and have been reviewed.
