# Final first-pass scaffold and coverage review

29 October 2026. This review follows the approved placement migration and the
MIPI display discussion. Existing uncommitted work and the migration snapshot
were preserved. Counts describe the current working tree.

## Added and repaired

Added **26 main-catalogue first-pass module scaffolds**. The 25 new public
headers use typed configurations, values, bounded spans/handles and operation
outlines. The additional peripheral-extension record supplies definitions for
ten existing declarations. All new records are FIRST_PASS, Phase 0, explicitly
scaffold-only and unavailable to generation. Their draft interfaces are not a
stable ABI and are not working drivers.

| Module | Missing boundary now scaffolded | Operations |
|---|---|---:|
| [`ucore/span`](../../include/ucore/span.h) | Checked byte spans and size arithmetic | 4 |
| [`ucore/deadline`](../../include/ucore/deadline.h) | Clock-domain stamps and checked finite deadlines | 4 |
| [`ucore/codec`](../../include/ucore/codec.h) | Bounded binary cursors and wire values | 4 |
| [`upal/transaction`](../../include/upal/transaction.h) | Common asynchronous I/O token and ownership contract | 4 |
| [`upal/dsi_host`](../../include/upal/display/dsi_host.h) | MIPI DSI host with explicit D-PHY/C-PHY profile | 7 |
| [`upal/display_panel`](../../include/upal/display/display_panel.h) | Bounded panel profile and initialization sequencing | 6 |
| [`upal/touch`](../../include/upal/display/touch.h) | Coherent bounded touchscreen contact snapshots | 4 |
| [`upal/power`](../../include/upal/power.h) | Power-state admission, wake and battery facts | 5 |
| [`upal/block_device`](../../include/upal/storage/block_device.h) | Generation-tagged block storage and durability | 5 |
| [`umid/buffer_pool`](../../include/umid/messaging/buffer_pool.h) | Fixed aligned blocks with generation-tagged ownership | 4 |
| [`umid/mailbox`](../../include/umid/messaging/mailbox.h) | Coherent copied latest-value publication | 3 |
| [`umid/event_bus`](../../include/umid/messaging/event_bus.h) | Bounded typed topic and subscriber delivery | 4 |
| [`umid/sensor`](../../include/umid/sensor.h) | Shared sample quality, clock and calibration envelopes | 4 |
| [`umid/settings`](../../include/umid/data/settings.h) | Generation-checked staged configuration transactions | 5 |
| [`umid/input`](../../include/umid/ui/input.h) | Bounded touch, button and encoder events | 5 |
| [`umid/graphics`](../../include/umid/ui/graphics.h) | GUI integration, assets and display flush ownership | 5 |
| [`uactuator/actuator`](../../include/uactuator/actuator.h) | Typed actuator authority, expiry and safe output | 5 |
| [`usec/provider`](../../include/usec/provider.h) | Shared entropy, key and crypto provider boundary | 5 |
| [`uapp/product_session`](../../include/uapp/product_session.h) | Generic peer identity and command outcome coordination | 5 |
| [`uapp/diagnostics`](../../include/uapp/diagnostics/diagnostics.h) | Structured diagnostics and authorized maintenance commands | 4 |
| [`uproto/connection_manager`](../../include/uproto/network/connection_manager.h) | Bounded interface reconnection and session invalidation | 4 |
| [`uproto/usb_device`](../../include/uproto/usb_device.h) | Bounded CDC/HID device-class profile | 6 |
| [`uproto/time_sync`](../../include/uproto/network/time_sync.h) | Explicit synchronized-clock mapping and uncertainty | 3 |
| [`uproto/framing`](../../include/uproto/framing.h) | Bounded optional COBS/SLIP framing profiles | 4 |
| [`uos/deferred`](../../include/uos/deferred.h) | Static deferred work and finite timer release | 4 |
| [`upal/capability_contracts`](../../include/upal/capability_contracts.h) | Advanced peripheral compatibility contracts | 10 |

Existing secure-update and fault-history headers also now have source
definitions for their eleven operations, and C++ linkage guards. The final
archive consumer covers **134 unsupported operations across 28 source files**.
Every operation returns `UETD_NOT_SUPPORTED` without reading or altering caller
storage, invoking callbacks or pretending to touch hardware. They never return
success as a substitute for implementation.

The declaration-to-source scan found ten missing advanced UART, timer capture,
synchronized-I/O and USB profile definitions in addition to the eleven UAPP
definitions repaired earlier in this pass. They are now present. A parser
false positive for function-pointer return typedefs was excluded; this scan is
an inventory aid, not a C ABI proof.

## Coverage and ownership decisions

- MIPI is represented by the DSI host and panel/profile adapter. D-PHY/C-PHY
  are explicit hardware compatibility facts; there is no pretend software PHY.
  The generic display facade keeps pixel ownership independently of transport.
- Touch, button/encoder ingress, GUI flush/asset bounds, power/wake leases and
  product identity/command outcomes now have separate generic scaffolds. They
  are not specific to Myovena part numbers or a selected GUI library.
- Common checked spans, time domains/deadlines, wire cursors, asynchronous
  tokens, block storage, fixed pools, latest mailboxes and event delivery now
  have explicit first-pass boundaries rather than being implied by a ring
  buffer or a display buffer alone.
- Sensor quality/calibration, staged settings, actuator authority/safe output,
  diagnostics and shared security-provider ownership now have source outlines.
  Device drivers and vetted crypto implementations still come from providers.
- Connection management, selected USB device classes, synchronized time and
  optional COBS/SLIP framing are separate bounded profiles. The existing UPAL
  USB profile helpers are compatibility development outlines; implement one
  class-provider route, with UPROTO owning CDC/HID class policy and UPAL owning
  the controller. Do not build duplicate class stacks.
- The [host generation catalogue](../../registry/generation/README.md) now
  records eleven first-pass pipeline obligations: typed emission, provider
  resolution, exact resources, memory/linker planning, codecs, builds, signing,
  deployment, decoding, manufacturing and qualification. It is distinct from
  embedded C modules; every host record is design-only and generation-blocked.
- Existing target/startup profiles, UOS primitives, sensors, math/control,
  protocols and persistence keep their established owners. No duplicate
  algorithm names or target records were added to inflate coverage. Later
  audio/imaging/inference/mesh and specialized algorithms remain later scope.

This closes the identified first-pass *scaffold/catalogue* gaps against
[FIRST_PASS_SCOPE.md](../design/complete-system/FIRST_PASS_SCOPE.md). It does
not establish that every source operation, provider, emitter or target works.
The [complete design](../design/complete-system/README.md) and
[362-row design matrix](UETD_IMPLEMENTATION_DESIGN_MATRIX_2026-10-29.csv)
retain the implementation and acceptance obligations for every module.

## Validation

Current inventory: **362 module IDs**, **472 type declarations**, **395 public
headers**, **331 UMATH function declarations**, and **362 generated module
routes**. Module selection blocks 330 records explicitly and two through
dependencies; 30 pass module-level metadata and dependency checks. This is not
a measure of complete generation support.

- HOST/BAREMETAL builds and all **15 CTests** pass for f32 Debug and f64 Release.
- The new consumer links through the ordinary UETD archive, checking 134
  unsupported operations and unchanged caller storage, exposing omitted source
  routing rather than only compiling stub sources directly.
- All **395 headers** compile independently as C11 and C++17. A C++ executable
  also links and calls representative UAPP, DSI and security stubs.
- Python: **28 tests run, 23 passed, five optional editor tests skipped** because
  the language-server extra is absent. New tests check source discovery,
  blocked selection, design references and separation of host obligations.
- The catalogue audit now detects orphan public entry points. Regression tests
  check missing definitions and avoid treating function-pointer typedefs or
  inline definitions as missing external functions.
- All **29 JSON Schema definitions**, **11 registry indexes** and 99 target
  records with record schemas validate. Catalogue closure, routes, design
  references, matrix coverage and local Markdown file targets pass.

## Remaining implementation limits

The two reproduced numerical defects remain open: Cholesky quotient/narrowing
overflow can report success with infinity, and software Q31 CORDIC maps positive
unity out of signed range. See the [critical audit](UETD_CRITICAL_CATALOGUE_AUDIT_2026-10-29.md).
Common long-running clock/timeout semantics and real host synchronization also
need implementation and qualification. Scaffold additions do not fix these.

No exact panel/MCU/touch/radio configuration is selected or hardware-qualified.
Provider integration, typed generation bindings, full emission/build/deployment,
numerical edge qualification and target timing/power/interoperability evidence
remain necessary before claiming a complete usable embedded-systems generator.
