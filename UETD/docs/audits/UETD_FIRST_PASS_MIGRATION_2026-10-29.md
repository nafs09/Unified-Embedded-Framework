# Completed first-pass catalogue migration

29 October 2026. The user approved the exact scope in
[FIRST_PASS_SCOPE.md](../design/complete-system/FIRST_PASS_SCOPE.md#proposed-migration-of-older-first-pass-entries).
The migration is complete.

This file preserves the inventory and validation snapshot immediately after
that migration. The subsequent [final scaffold review](UETD_FINAL_SCAFFOLD_REVIEW_2026-10-29.md)
records the expanded current catalogue and checks.

## Result

- Moved all 51 approved files into their first-pass owning directories.
- Renamed 10 canonical module IDs, preserving their DSL descriptors and C API
  symbols. Includes, manifests, dependencies, documentation and generated
  routes now use the new paths.
- Reclassified 20 design entries into `registry/first_pass/registry.json`.
  Each declares FIRST_PASS, Phase 0, design-only and generation-blocked.
- Added blocked, header-only UAPP design records for secure update and fault
  history. There are now 336 module IDs, with 304 explicitly blocked and two
  further records blocked by dependencies. Thirty pass module-level metadata
  and dependency checks; this does not establish complete generation readiness.
- Kept the six later planning entries in `registry/future/registry.json`:
  IEEE 802.15.4, CANopen FD, MEDI CAN, PPP, CoAP and mesh networking. The broader
  module catalogue retains its other later algorithms and protocol outlines.
- Moved ten shared record schemas into `registry/contracts/`. Added a
  first-pass design loader and scope-separation checks without granting
  generation readiness.

Missing service implementations remain missing and blocked. The promoted
storage, calibration, blackbox, process-image, Modbus and CANopen CC sources
still return unsupported without changing caller storage. TLS/MQTT/HTTP still
require integrated, qualified providers. QP_ACTIVE_SET still needs its typed
solver implementation and numerical qualification.

The generic display facade remains first-pass. The
[Myovena handheld specification](../design/complete-system/MYOVENA_HANDHELD.md)
contains the approximately 7-inch OLED touchscreen, BLE/WiFi, memory,
bandwidth, power, touch/UI and acceptance requirements. MCU, panel and touch
controller selection remain open.

## Preservation and verification

The current uncommitted inputs were captured before moving files. A byte-for-
byte snapshot of 1,865 source/document/template files, SHA-256 hashes, the
original Git status and the exact 51-file map are retained under
`C:/NEXUS/uetd-audit-work/first-pass-migration-snapshot/`. All stored bytes and
hashes were verified. Existing unrelated work was preserved.

- All 51 original file paths are absent and all destinations exist. The module
  key set equals the original set after exactly ten approved renames plus the
  two new UAPP design records. Moved C API callable symbols are preserved.
- All 12 promoted runtime/design module records remain Phase 0 and rejected
  by module selection. Scope indexes are disjoint and preserve all 26 original
  planning identities.
- All 28 JSON Schema definitions and ten registry indexes validate. The 99
  chip/board/toolchain records with record schemas also validate; 68 family
  records pass index/path/identity checks.
- All 370 public headers compile independently as C11 and C++.
- MSVC HOST/BAREMETAL passes all 14 CTests in both f32 Debug and f64 Release.
  The new first-pass service consumer checks 32 unsupported operations and
  preservation of caller storage.
- Python regressions: 23 tests run, 18 passed and five optional editor tests
  skipped because the language-server extra is absent. All three new catalogue
  placement tests pass.
- Catalogue paths, dependency/header closure, generated routes, active moved
  references and local Markdown file targets pass their checks.

Earlier independent numerical comparisons remain documented in the
[whole-catalogue audit](UETD_CRITICAL_CATALOGUE_AUDIT_2026-10-29.md).
The reproduced Cholesky overflow and software Q31 CORDIC endpoint defects
remain open. Host verification does not qualify physical targets or providers.

The inventory can be refreshed with `python tools/audit_catalogue.py --inventory
docs/audits/UETD_MODULE_INVENTORY_2026-10-29.csv` from the UETD directory.
The migration regressions are in `uef-gen/tests/test_catalogue_placement.py`;
run them with the normal `uef-gen` dependencies and source path configured.
