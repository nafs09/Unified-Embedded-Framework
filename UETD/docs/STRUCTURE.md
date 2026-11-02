# UETD folder structure

UETD keeps its U* layer boundaries and groups related capabilities inside each
layer. Headers, C sources, module manifests and build templates use matching
topic folders. A topic folder contains multiple files; a capability does not
need its own folder simply to hold one file. U* layer roots such as `usec/` and
`uactuator/` may contain a single implementation.

The top-level trees retain their purposes:

| Tree | Contents |
|---|---|
| `include/` | Public headers and their layer guides |
| `src/` | Implementations and explicitly incomplete source scaffolds |
| `registry/` | Authoritative indexes, manifests and schemas |
| `templates/` | Reusable module routes and target project templates |
| `examples/` | Example source plus a README for each example |
| `docs/` | Architecture, designs, catalogues, audits and roadmap |
| `tests/` | C contract checks and CMake test support |
| `tools/` | Registry, template and documentation maintenance tools |
| `third_party/` | Dependencies kept in their upstream layouts |

The FreeRTOS test configuration lives at `tests/FreeRTOSConfig.h`, alongside
the test sources and CMake support, rather than in a folder containing one file.

## Topic groups

| Layer | Main topic folders |
|---|---|
| UAPP | `runtime/` for lifecycle, state machines, supervision and watchdog gating; `diagnostics/` for faults, fault history and diagnostics |
| UHAL | Common public headers and `stm32.h` at `include/uhal/`; CPU implementations under `src/uhal/arch/`, platform implementations under `src/uhal/platform/`, family records under `targets/`, and build composition in `src/uhal/sources.cmake` |
| UPAL | `analog/`, `can/`, `compute/`, `display/`, `network/`, `serial/`, `storage/`, `timing/`, `watchdog/` |
| UPROTO | `can/`, `rc/`, `network/`, `industrial/`; shared framing, MAVLink and USB device contracts remain at the layer root |
| UMID | `drivers/`, `messaging/`, `logging/`, `data/`, `ui/` |
| UMATH | `derivatives/`, `geometry/`, `lie/`, `matrix/`, `numerical/`, `polynomial/`, `probability/`, `signal/`, `vector/` |
| UCON | Algorithm families remain grouped; isolated support algorithms share their owning control or estimator folder. Electrical and infrastructure algorithms share their respective family folders. |
| UOS | Backend implementation files share `src/uos/`; a separate backend folder is unnecessary for one file. |
| UHAL | `arch/` contains CPU operations; `platform/` contains SoC/SDK implementations and host simulation. Physical assemblies and programming tools remain separate target metadata. See [the porting guide](design/UHAL_PORTING.md). |

`numerical/` collects integration, splines and least squares. `probability/`
collects distributions, random sampling and streaming statistics. UCON's
single-file support categories have been folded into their closest useful
parent. The initial QP active-set implementation remains directly in UCON
until a larger active optimization group warrants a folder.

## CAN family

CAN-related files stay within their owning layer and share a consistent
`can/` topic:

```text
include/upal/can/                 src/upal/can/
  can.h                            can.c
  fdcan_fd.h                       fdcan_fd.c

include/uproto/can/               src/uproto/can/
  canopen.h                        canopen.c
  uavcan.h                         uavcan.c
  future/                          future/
    canopen_fd.h                     canopen_fd.c
    j1939.h                          j1939.c
    medi_can.h                       medi_can.c
    uavcan_v1.h                      uavcan_v1.c
```

Matching manifest and template groups live under
`registry/modules/manifest/{upal,uproto}/can/` and
`templates/modules/{upal,uproto}/can/`. The CANopen FD and MEDI CAN design
documents share [`docs/design/can/`](design/can/).

UPAL owns controller/frame boundaries; UPROTO owns protocol behavior. The
folder grouping preserves those boundaries. CANopen FD and MEDI CAN remain
separate, fail-closed future scaffolds.

## Paths and public identity

Module IDs and C symbol names remain stable when files move. For example,
`upal/fdcan_fd` still identifies the module whose files now live under
`upal/can/`. Registry indexes provide the physical manifest paths. Maintenance
writers preserve those paths rather than rebuilding them from module IDs.

Each module route is named `<capability>.cmake.tmpl` and follows its manifest's
topic folder. For example, `templates/modules/upal/can/fdcan_fd.cmake.tmpl`
replaces a dedicated directory containing only `route.tmpl`. Consumers should
read `templates/modules/index.json` to find a route.

Later scope is recorded in module IDs and manifest metadata. Most later
protocol and algorithm files remain in family-specific `future/` folders.
The lone sparse CSR outline shares `umath/matrix/`; IEC 60870 and LIN share
`uproto/industrial/`. CMake explicitly excludes these later files from normal
builds and includes them only through the existing future-scaffold options.
Their relocation does not make them generation-ready.

Header include paths changed with this migration. Use the current header
guides or the [complete move list](audits/UETD_STRUCTURE_MOVES_2026-10-30.csv)
when updating downstream includes. Compatibility forwarding headers would
recreate the unwanted duplicate layout and are not supplied.

## Keeping the structure consistent

From the UETD root, run:

```powershell
python tools/generate_module_templates.py
python tools/audit_catalogue.py
python tools/audit_layout.py
```

The layout audit checks single-file folders, empty authored folders and the
relationship between manifest paths and generated routes. Its `--exclude`
option can omit a subtree maintained separately during concurrent work.
Generated IDE/build files, caches and bundled dependencies retain the layouts
required by their own tools and upstream projects.

Use a vendor folder in target manifests when it groups multiple records.
A vendor represented by one record can place that file directly in the
manifest parent. Target schemas accept both forms and reject traversal paths.

The [example index](EXAMPLES.md), [target template guide](TARGET_TEMPLATES.md),
[dependency notes](DEPENDENCIES.md) and
[target authoring guide](../registry/TARGETS.md) provide the supporting indexes
without adding a README as the only file in a container folder.
