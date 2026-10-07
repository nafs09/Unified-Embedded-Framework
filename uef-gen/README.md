# uef-gen

`uef-gen` is the host-side project assembler for UEF-based embedded firmware. It validates hardware configuration, resolves target resources, selects UEF modules, writes a structured project scaffold, and records build inputs and provenance. It is a Python tool; generated firmware does not require Python or NEXUS at runtime.

## Ownership

- **NEXUS** authors the application and serializes its control graph as versioned ControlIR JSON.
- **UEF** owns reusable embedded source, public headers, module/API metadata, the UMATH mathematical foundation and catalogue, UCON algorithms, generic UPROTO APIs, and every reusable UCON/UPROTO template.
- **uef-gen** owns the JSON bridge, hardware/resource resolution, module selection, project scaffolding, and the adapter that renders templates from the separately located UEF checkout.

The generator does not contain a UEF source snapshot, algorithm-specific `AlgorithmIR` types/builders, or local protocol/algorithm templates. It validates ControlIR, walks top-level and nested composite nodes, resolves each node's dispatch key through UEF's `registry/control_ir/registry.json`, and places serialized node data plus the resolved target context in a stable template context. The index points to one UEF detail manifest per node, alias, or algorithm under `UEF/registry/control_ir/manifest/`. Node fields such as `model`, `features`, `structural_constants`, `config_params`, `plant_constants`, ports, and execution-group references stay available to those templates. UEF detail files own the domain, template/output paths, required modules, and declared manual actions. NEXUS supplies ControlIR; uef-gen does not infer equations or synthesize implementation requirements.

## Requirements and installation

- Python 3.11 or newer
- PyYAML and jsonschema (installed as package dependencies)
- Jinja2 when a ControlIR request renders UEF templates; install the `templates` extra

Install from this repository:

```powershell
python -m pip install -e .\uef-gen
Set-Location .\uef-gen
python -m pip install -e ".[templates]"
```

The second command is only needed when rendering UEF templates. It can also be installed in one operation from the `uef-gen/` directory with `python -m pip install -e ".[templates]"`.

## Locating UEF

At generation time the locator checks, in order:

1. `UEF_PATH`
2. `uef.path` in the hardware configuration
3. `../UEF`, `../../UEF`, and `./UEF`, relative to the configuration/project directory

The located checkout must contain uef_api.json and every registry index and detail manifest declared by that contract. Every selectable module declares its source files and public-header closure; generation copies only those files for the resolved module dependency closure. A versioned checkout also contains `version.json`; the generator checks its compatibility range. The source fingerprint covers the live registry/detail trees. Generated provenance includes the indexes and referenced detail files. No copy of UEF is packaged in the Python wheel.

For example, when `UEF/` and `uef-gen/` are sibling folders:

```powershell
$env:UEF_PATH = (Resolve-Path .\UEF).Path
```

Alternatively, set `uef.path` in the project YAML:

```yaml
uef:
  path: ../UEF
```

## Commands

Validate target/resource configuration:

```powershell
uef-gen validate .\uef-gen\examples\project.yaml
```

Resolve the selected module closure without writing a project:

```powershell
uef-gen generate .\uef-gen\examples\project.yaml --output .\build\firmware --dry-run
```

Generate a project from a hardware YAML file and a NEXUS ControlIR JSON document:

```powershell
uef-gen generate .\uef-gen\examples\project.yaml `
  --control-ir .\build\control-ir.json `
  --output .\build\firmware `
  --work .\build\uef-gen-work
```

The NEXUS bridge can invoke the same versioned JSON contract on stdin or with `--request`. A request has `contract_version`, `operation`, `hardware_configuration`, optional `control_ir`, `output_directory`, `work_directory`, and optional `project_directory`; the response is one JSON object containing `ok`, diagnostics, and (on success) the manifest.

Inspection commands include targets list, targets list --include-scaffolds --uef PATH, targets show <chip> --uef PATH, modules list/show, snapshot verify, and version. The scaffold listing reads UEF-owned family and chip-base catalogues at runtime. Its 38 exact-part base records expose sourced identity/core/package/memory facts across the named vendor groups, but remain non-generatable until complete resource and board mappings are verified.

## Generated project and current limits

Generated output includes selected UEF files under `uef/`, pinned UEF-managed dependency files under their declared project-local roots, application/config/board/target scaffolding, canonical `project_*.txt` source/include/define/compiler/link lists, optional Make/CMake fragments, and JSON provenance under `manifest/`. `manifest/manual_actions.json` lists path-qualified driver, protocol, board, and other work declared by UEF; matching actions also appear as warnings in the response. The generated startup and linker files are placeholders until a verified target profile supplies real values.

For a registered UCON node, UEF contributes two complementary pieces: the selected module’s generic C implementation and public header, plus the template output that gives this ControlIR instance a small named state/configuration wrapper and forwards calls to that generic implementation. The template does not emit a second copy of the control law. The generated project now copies only the selected modules’ source files and declared transitive public-header closure, so unrelated UEF APIs are not staged.

UEF's shared ControlIR manifest registers the initial UPROTO wrapper templates for DSHOT, CRSF, SBUS, PPM, MAVLink, and UAVCAN/DroneCAN, plus UCON PID and lead-lag wrappers. Registered entries produce their UEF-declared files and manual-action records; the generic C implementations, target driver, external dialect/CAN stack, and board binding still determine runtime readiness. Every other UCON catalog key remains `registered: false`; a request returns `algorithm_unregistered` before publishing output. The adapter validates each key's individual header/source/module registration and includes its path and roadmap priority in the diagnostic. The module selector rejects scaffold-only entries even when an application requests them explicitly. The adapter contains no local UCON algorithms or template fallback. Hardware-only project assembly remains available independently.

The family roadmap is owned by UEF at `registry/targets/families/registry.json`, with
one file per family under `registry/targets/families/manifest/`, and read through the
path declared in `uef_api.json`. The chip-base registry is
`registry/targets/chips/registry.json`, with one file per exact part under
`registry/targets/chips/manifest/`. These include six Phase 0
profiles, the Phase 1 SDMMC target families STM32H7/F7/L4+ (L4+ is a new scaffold), every later target named in Part XV,
and broader STM32/Espressif series coverage. Use
uef-gen targets list --include-scaffolds to list it; pass --uef PATH when
the UEF checkout is outside UEF_PATH or documented relative paths. Catalogue
rows are never ChipSpec targets and cannot be generated.

For FreeRTOS, UEF owns the source package and exact pin/profile metadata. `uef-gen` never downloads dependencies: generation fails with a path-specific diagnostic until the locked checkout, license text files, and a matching architecture/compiler profile are present. The selected kernel sources, headers, license files, and revision/profile provenance are copied into the generated project. FatFS remains an explicitly consumer-supplied optional dependency.

The sample target data is not verified for physical hardware. Project assembly does not mean the output is ready to flash. Build, artifact verification, physical target discovery, and deployment remain incomplete; inspect [`TODO.md`](TODO.md) before relying on those paths.

See [`ARCHITECTURE.md`](ARCHITECTURE.md) for the current source map and ownership boundaries, and [`TODO.md`](TODO.md) for the detailed implementation plan.
