# UETD structure reorganisation — 2026-10-30

The reorganisation groups related capabilities across headers, sources,
manifests and templates while retaining U* layer boundaries and existing
module IDs. The [structure guide](../STRUCTURE.md) describes the layout;
the [move inventory](UETD_STRUCTURE_MOVES_2026-10-30.csv) lists every old and
new file path.

## Scan and moves

The initial complete filesystem scan found 3,230 files, including generated
IDE/build state and bundled dependencies. A pre-edit snapshot retained 1,867
authored UETD files. There were 494 authored folders containing one direct
file outside the allowed U* layer roots.

| Original section | Files moved |
|---|---:|
| `include/` | 143 |
| `src/` | 137 |
| `registry/` | 137 |
| `templates/` | 367 |
| `docs/` | 4 |
| `examples/` | 1 |
| `third_party/` documentation index | 1 |
| Newly added test configuration | 1 |
| UHAL CMake selector | 1 |
| Total | 792 |

All 792 destination files were verified present. Existing edited and untracked
files were included. The pre-edit snapshot and checked move map remain in
`C:\NEXUS\uetd-structure-work\`; the later FreeRTOS configuration move has a
separate snapshot there. Bundled FreeRTOS sources, build products and IDE
caches retain their upstream/tool-owned files. The final cleanup removed empty
branches from those trees without deleting any source, dependency or build files.

The initial pass deferred UHAL while another chat changed it. The user later
authorised completing UHAL and removing unused folders. The final layout audit
now covers all authored UETD folders without exclusions. Earlier target-profile
and route moves and the separately authorised FreeRTOS test-header move remain
in the inventory.

## Main changes

- Classic CAN and CAN FD share UPAL's `can/` family. CANopen, UAVCAN and the
  separate later CANopen FD, MEDI CAN, J1939 and UAVCAN v1 outlines share
  UPROTO's `can/` family. CAN design documents share `docs/design/can/`.
- Peripheral, protocol, middleware, application and numerical capabilities
  have shared topic folders. Single-file algorithm categories were folded
  into useful parent groups.
- The original 362 module routes became capability-named `.cmake.tmpl` files
  beside related routes, replacing one folder per `route.tmpl`.
- Registry writers retain indexed physical paths. Route generation follows
  those manifest paths while preserving public module IDs.
- Target schemas accept both flat records and vendor groups. Type discovery
  uses manifest scope for colocated later headers. Internal math helpers
  remain excluded from public operation discovery.
- Header includes, CMake paths, indexes, examples and documentation references
  were updated. Layer guides contain current inventories and file trees.
- `tests/FreeRTOSConfig.h` shares the test root; its CMake include directory
  was updated for all three tick-width targets.

## Initial validation

| Check | Result |
|---|---|
| Initial folder audit outside then-continuing UHAL work | Zero errors |
| Catalogue audit | 364 modules, zero integrity errors |
| Local Markdown/HTML documentation links | Zero missing links |
| Public headers compiled separately as C11 and C++17 | 395 headers, 790 successful compilations |
| Dedicated layout/generator regression tests | Seven passed |
| UMATH type scope and operation-index comparison | No semantic differences |
| Original default host configuration | Build and all 15 then-existing C tests passed |
| Later-scaffold configuration with f64 scalar ABI | Build and all 18 then-existing C tests passed |
| Later default configuration after concurrent test additions | Build passed; 19 of 22 tests passed; three new FreeRTOS integration tests timed out |
| Relocated FreeRTOS configuration | All 16/32/64-bit tick-width targets built |
| Patch whitespace check | Passed |

Compiler source inventories confirm sparse CSR, IEC 60870, LIN, CANopen FD,
MEDI CAN, pole placement and the anti-windup observer are excluded from the
default library and included with their existing future-scaffold options.

The initial full Python suite was run against the live checkout. That run had
57 tests and 22 failures while other chats were changing implementation
readiness and language dispatch. Observed failures include an unknown
`physical_target_profile` lowering rule and tests still expecting newly
implemented services to be scaffolds. The seven reorganisation tests pass.
The broader concurrent changes remain with their owning work.

## UHAL completion and empty-folder cleanup

The remaining STM32-specific header moved from a one-file `platform/` folder
to `include/uhal/stm32.h`. Build composition moved to
`src/uhal/sources.cmake`, alongside the implementation map it controls.
The source globs were rebased, and CMake, API metadata and guides were updated.
Public C symbols and module identities remain unchanged by these moves.

The completion removed **215 empty folders**: 206 generated build-output
branches, four obsolete UHAL backend folders, the former CMake and public
platform containers, two empty optional upstream-port placeholders, and the
new one-file guide container after moving its guide into `docs/`.
Every deletion checked that its resolved path stayed inside UETD and that the
folder was still empty. The authored tree has no empty folders. Active build
tools subsequently recreated empty branches under `out/`; those are generated
working directories rather than obsolete source folders.

The complete layout audit and the catalogue audit both pass without exclusions.
All 396 current public headers compile independently as C11 and C++17
(792 compilations). The full Python suite passes all 63 tests, including the
new whole-tree layout regression.

A complete C-suite build exposed a separate existing SafeRTOS scaffold-test
generator problem: it emits an invalid call for `uos_clock_use_platform(void)`.
The core library and portable example build, and all six focused UHAL contract
tests pass. These are validated separately;
the malformed generated SafeRTOS test remains outside this layout change.

To reproduce the final folder audit from the UETD root:

```powershell
python tools/audit_layout.py
```

The catalogue and layout commands are documented in the
[structure guide](../STRUCTURE.md).
