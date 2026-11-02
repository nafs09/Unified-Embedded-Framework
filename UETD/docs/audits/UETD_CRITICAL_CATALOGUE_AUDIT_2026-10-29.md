# UETD critical catalogue and implementation audit

Audit date: 29 October 2026. Scope: the live canonical UETD checkout and its
uef-gen consumers, excluding vendored third-party implementation internals and
the separate copy under NEXUS/Code/third_party. Existing uncommitted work was
preserved. Counts describe the reviewed working tree, not a tagged release.

## Decision

UETD has extensive control/estimation coverage on paper and a useful portable
foundation. It is not yet a complete embedded systems generator: the dominant
gaps are usable target/provider integrations, typed operation contracts,
generation, product services, resource/timing guarantees and qualification.
Adding more algorithm names is less valuable than completing these vertical
paths. Existing first-pass obligations should remain in the first pass even
when their implementations are missing.

The [complete-system design](../design/complete-system/README.md) specifies the
missing architecture, family contracts, resource/failure rules, generation and
acceptance projects. [First-pass scope](../design/complete-system/FIRST_PASS_SCOPE.md)
separates required first-pass work from genuine later expansion. The
[Myovena handheld specifics](../design/complete-system/MYOVENA_HANDHELD.md)
bring its approximately 7-inch OLED/touch graphics path into the first pass.

## Inventory and evidence

The reviewed baseline had 333 modules. The display facade, two blocked UAPP
design records and 26 final-pass scaffold records bring the current inventory
to **362 module IDs**. The complete
[module inventory](UETD_MODULE_INVENTORY_2026-10-29.csv) lists every ID, source,
header, dependency, authoring/readiness state, source evidence and blocker.

| Layer | Module IDs | Assessment |
|---|---:|---|
| UCORE | 4 | Implemented primitive/status/assert/CRC utilities; shared checked sizes/spans/deadline contracts and cross-toolchain override policy need completion |
| UHAL | 4 | Host simulation and generic Cortex-M source boundary; target core/clock/GPIO behavior remains partly stubbed, and there is no qualified physical board |
| UPAL | 33 | Extensive peripheral contracts; most physical operations are fail-closed source outlines. Network/radio/display facades validate dispatch but need real providers |
| UOS | 5 | Bare-metal and pinned FreeRTOS adapter; common time/status/lifetime/ISR semantics and port qualification remain incomplete; SafeRTOS is later/unavailable |
| UMID | 26 | Ring, health, bounded text log and FPGA endpoint bookkeeping are useful; nine sensor/measurement drivers are stubs. Durable data services remain unimplemented |
| UPROTO | 29 | PPM is implemented; radio/IP/TLS/MQTT/HTTP facades need providers. Other protocol outlines remain blocked; Modbus and CANopen CC are first-pass requirements |
| UAPP | 9 | Lifecycle, flat FSM, supervisor, fault queue and watchdog gate are implemented; dependency cleanup, durable crash context and secure maintenance have first-pass design records and need implementations |
| UCON | 213 | Two implemented algorithms, one core module and 210 blocked algorithm identities. Breadth is ample; typed/numerical/model contracts and reusable foundations are the bottleneck |
| UMATH | 35 | 331 public declarations: 258 bodies without a not-implemented return and 73 fail-closed operations. Eleven module identities are eligible by metadata; 24 remain blocked, including direct-C dual/vector implementations awaiting binding review |
| UACTUATOR | 2 | H-bridge authoring identity; generic lifecycle/authority/safe-output scaffold is present but unimplemented |
| USEC | 1 | Shared entropy/key/crypto/authorization provider scaffold; no actual cryptographic implementation |
| UPHYSICS | 1 | Later design identity; no complete physical-model library |

Thirty modules pass module-level flags and dependency closure; two additional
unblocked records are stopped by blocked dependencies; 330 are explicitly
blocked. Passing module selection is not equivalent to a ready language
instance, a target build, or hardware support. Only four manifests have
authoring contracts: three draft and one reviewed. Twelve UMATH functions have
ready language bindings. The standalone firmware-emission path remains incomplete.

Target catalogues contain 68 family records, 52 chip records, 16 development
boards and 31 toolchain records. One chip record is the nonphysical UETD
reference target; all 51 physical part records and all 16 board records are
generation-unavailable. All toolchain records are scaffold-only. Seven C family
profile outlines are not seven qualified targets. This audit checked structural
integrity/readiness and references; it did not independently revalidate every
silicon/package fact against its reference manual and errata.

The type index now contains 472 declarations. Regeneration exposed substantial
staleness in the previous type index and module-route catalogue. Counts are
inventory measurements, not an estimate of implementation percentage.

## Confirmed implementation defects still requiring correction

### Cholesky solve can report success with infinity

[`src/umath/matrix/cholesky.c`](../../src/umath/matrix/cholesky.c) checks the
pre-division accumulator but does not validate each quotient/narrowed scalar
before storing it. A one-element solve with `L=1e-20f`, `b=1` returns
`UMATH_OK` with `x=inf`. The f64 equivalent `L=1e-200`, `b=1` reproduces it.

Validate division results and scalar narrowing in both passes, define whether
output is scratch on failure or transactionally preserved, and add underflow/
overflow/nonfinite factor regressions. A successful checked numerical operation
must not publish an unrepresentable result.

### Software CORDIC maps positive unity outside Q31

[`include/upal/compute/cordic.h`](../../include/upal/compute/cordic.h) multiplies a floating
cosine by 2^31 and casts directly to signed Q31. For angle zero the host probe
returns sine `0`, cosine `-2147483648`. Positive unity needs saturation to
`INT32_MAX`; the current conversion is outside the representable signed range.

Define rounding/saturation and the angle convention, use checked conversion,
and compare software/hardware results at zero, quadrants, endpoints and values
next to saturation. The compile-time header/source conflict was repaired;
this numerical behavior is a separate unresolved defect.

### Time and concurrency assumptions are incomplete

The FreeRTOS time adapter converts the kernel tick count without extending
its wrap epoch. Its origin/resolution also differ from UCORE time. Finite wait
overflow can map to `portMAX_DELAY`, while periodic deadlines have unchecked
updates. Existing UOS contract documentation correctly identifies this work.
Implement a common clock/deadline/status model before long-running or hard
deadline claims.

Host PRIMASK/critical-section intrinsics are no-ops. That host mode cannot
prove multithreaded logger, health or fault-queue serialization. Cortex-M
interrupt masking is single-core and not an intercore lock. Add a real host
synchronization provider for concurrency tests and qualified core/channel
contracts for multicore operation.

## Quality assessment and improvement decisions

| Area | What is sound | What should improve |
|---|---|---|
| PID/lead-lag | Explicit configuration, finite checks, candidate state and bounded recurrence; PID includes conditional/back-calculation policies | Independent recurrence/anti-windup tests, reset/retuning/bumpless contracts and generated equivalence. Decide whether deadband/rate limiting remain explicit graph blocks |
| Dense math/geometry | Allocation-conscious APIs, configured scalar/accumulator, many checked operations and workspace queries | Correct quotient/narrowing failures, conditioning/rank policy, alias/partial-output contracts, bounded convergence and numerical error envelopes |
| Math scaffolds | Honest not-implemented returns and named operation outlines | Implement QR/LU/LDLT and the required derivatives/integration/statistics first; do not promote whole modules solely because one helper has a body |
| Ring/health/log | Bounded storage, coherent snapshots, explicit overflow and single-owner/SPSC assumptions | Real concurrency qualification, runtime drop/watermark diagnostics, latency budget for critical-section copies and configuration ABI consistency |
| FPGA endpoint | Useful link/freshness bookkeeping with explicit local time | It is not the FPGA transport, codec, identity handshake or RTL; those belong to the declared project protocol/provider |
| Lifecycle/FSM | Guarded transitions and bounded run-to-completion engine | Reverse-order cleanup, callback-failure safe actions, critical ingress behavior and stable configuration lifetime; hierarchy can follow actual product need |
| Watchdog/fault | Lease-based feed gate, bounded recoverable queue and deferred dispatch | Qualify hardware window/timing, avoid bypass feeds, capture real fatal context, retain/check/export records and test reset interruption |
| Peripheral drivers | Detailed public surfaces and operation TODOs | Finish resource/status/timeout/DMA/cache/cancel ownership, then exact target implementations and physical tests; neutral value/no-op stubs must remain blocked |
| Radio/network/display | Portable typed provider dispatch and bounded interface validation | Pin actual providers/firmware/SDKs, bound memory/work, qualify coexistence, disconnect/recovery and real peers; facades are not full stacks |
| Storage/protocol outlines | Useful typed envelopes and honest missing behavior | First-pass implementation for core persistence, TLS/MQTT/HTTP/Modbus/CANopen CC; explicit durability/security/interoperability gates |
| Generator | Registry ownership, dependency selection, resource/frontend analysis, scaffold assembly and provenance | Complete emission/build/deployment and end-to-end acceptance; make guarantees a resolved operation/provider/target/evidence conjunction |

## Catalogue coverage decisions

The control catalogue already covers classical/state-space/nonlinear/adaptive/
predictive control, observers/Kalman/attitude/frequency/identification,
filtering/signal processing, trajectories, allocation/safety and domain
profiles. Do not add another broad collection of named methods as the main
completeness strategy. Implement shared math, a bounded QP reference and common
typed filters/estimators before advanced variants.

The major missing or insufficiently integrated first-pass families are checked
time/storage/ownership, exact resource graphs, startup/linker/partition planning,
power management, real I/O/runtime providers, sample quality/calibration,
actuator command/safe-state services, pools/events/mailboxes, transactional
settings/logging, common network/application protocols, security/provisioning/
update, maintenance/observability, handheld graphics/touch/UI, generated codecs,
typed emission and qualification. Their full decisions and contracts are in
the [first-pass coverage table](../design/complete-system/FIRST_PASS_SCOPE.md).

Audio/imaging/inference, cellular/LPWAN/mesh, specialized industrial profiles,
advanced USB and selected research algorithms are reasonable later packs.
Linux/rootfs, arbitrary FPGA RTL, RF PHYs and certification are external scope
rather than implied capabilities of a firmware generator. The provider model
allows broad coverage without making UETD implement every external stack.

## Stale data and integration breaks corrected during this audit

- Marked source-only peripheral/sensor/protocol scaffolds unavailable to module
  selection. Baseline selection incorrectly accepted examples such as UART,
  IMU and CRSF even though their source only returned unsupported/no-op values.
- Separated external/provider/optional prose requirements from canonical module
  dependencies. Previously active network facades failed on fictitious module
  IDs such as “target-specific packet link provider”. They now fail explicitly
  on readiness until a provider is integrated.
- Corrected stale vector/dual source descriptions, header closure omissions,
  the moved UMATH catalogue path, the future-index schema locator field and
  22 broken peripheral-source links in the equation checklist.
- Refreshed type/function/module-route references. Build routes had lagged at
  311 entries; all 362 current module identities now have consistent routes.
- Repaired UCORE weak-attribute ownership, missing watchdog boot-grace state
  and initialization, missing fault-reset include, CORDIC/FMAC capability guards,
  the CORDIC math include and the matrix-core math-dispatch include. The latter
  could hide unresolved symbols inside an archive until a consumer linked it.
- Repaired scaffold-test source selection so provider implementations are not
  parsed as unsupported stubs, while preserving concurrently added protocol work.
- Added a real portable runtime/link test and display-provider tests. Existing
  scaffold tests primarily established honest failure, not correct algorithms.
- Added the generic first-pass display facade and separate OLED handheld
  specification. No panel/MCU or hardware qualification was fabricated.

The approved [first-pass migration](UETD_FIRST_PASS_MIGRATION_2026-10-29.md)
is complete: 51 files moved, 10 canonical module IDs changed and 20 planning
entries reclassified. Six genuinely later planning entries remain in the
future index. Promoted contracts remain generation-blocked; the migration
does not supply their missing runtime implementations.

## Verification and practical limits

The [final scaffold review](UETD_FINAL_SCAFFOLD_REVIEW_2026-10-29.md) closes additional
interface/source gaps and records the remaining implementation limits. The
MSVC host/bare-metal build and tests pass. Portable numerical probes made
1,201 deterministic independent NumPy/reference comparisons in each f32 and
f64 configuration (2,402 total), covering representative matrix solves/
exponential, Cholesky reconstruction/solve, vectors, complex division,
quaternion rotation, polynomials, scalar/dual sine and a Q31 saturation boundary.
Those normal-domain passes coexist with the two explicit edge-case failures
above; they do not establish numerical correctness over every supported domain.

All 395 current public headers compiled independently as C11 and C++ after
the stale signed-type reference was fixed. The host suite passes 15 tests in
both f32/Debug and f64/Release configurations, including the generic display
provider contract tests. Registry/schema/path/DAG/header-closure and
generated-route checks are reproducible with `tools/audit_catalogue.py` and the
normal generation tools. Target numerical/real-time, DMA/cache, radio, display,
storage power-fail and physical interoperability qualification remain outstanding.

## Recommended execution order

1. Correct the reproduced numeric defects and stabilize common time/status,
   ownership, provider and resource contracts.
2. Complete generated portable behavior and multiple exact-board vertical paths:
   startup/time/I/O/runtime/watchdog/build/program/diagnostics.
3. Complete sensor/actuator/data services and shared numerical/filter/estimator/
   QP foundations required by those products.
4. Integrate connected products, secure maintenance and the Myovena OLED/BLE/
   WiFi handheld as first-pass acceptance compositions.
5. Expand industrial, multicore/multi-node and later packs through the same
   contracts and evidence gates, rather than widening readiness claims by name.
