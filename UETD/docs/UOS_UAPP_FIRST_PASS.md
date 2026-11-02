# UOS/UAPP portable first pass and manual project assembly

Updated 2026-10-30. This pass implements portable runtime behavior and its
template path. A manually assembled host application can run now; no additional
whole U* layer must be completed first. Hardware I/O, authentication, retained
storage and boot management still need the concrete providers used by that
application. They cannot be inferred from a portable interface.

## What is implemented

| Area | Behavior now supplied |
|---|---|
| UOS common | Explicit monotonic microsecond clock binding, checked unit conversions and absolute periodic deadlines with missed-release skipping. |
| UOS bare metal | Checked time/delay/yield; tasking and synchronization explicitly return `UOS_NOT_SUPPORTED`. The application owns the cooperative loop and IRQ ingress. |
| UOS FreeRTOS | Static tasks, queues, mutexes, counting semaphores and event flags; explicit errors, storage/alignment checks, scheduler and ownership checks, finite timeout conversion, ISR result/yield handling. |
| UOS deferred | Caller-backed bounded slots, clock-domain/generation checks, cancellation tokens, FIFO among ready work, poll budgets and protection against recursive dispatch and immediate self-resubmission. |
| UAPP state machine | Copied bounded FIFO, validated ordered guards, exit/action/entry ordering, external self transitions, latched failures, reentrancy rejection and explicit reset. Reset discards events from the old activation, including its exit hook. |
| UAPP lifecycle | Guarded init/start/stop states; a callback's explicitly reported fault remains latched. |
| UAPP supervision | Timestamp-zero-safe cadence, persistence thresholds, stale checks and recursive tick protection. |
| UAPP watchdog gate | Explicit startup grace, actual-report/freshness/status checks, recursive tick rejection and a first-cause latch. Registration alone never authorizes a post-grace feed. |
| UAPP faults | Bounded copied RAM reports, overflow accounting and bounded nonrecursive recovery dispatch. Reports remain queued without a handler. Fatal handling crosses the platform reset boundary. |
| UAPP diagnostics | Bounded copied records and maintenance requests, schema/authority checks, expiry checks after authorization, terminal result delivery and no implicit execution retry. |
| UAPP product session | Copied commands, increasing IDs, bounded outstanding work, authority/expiry checks, evidence-based outcomes and no replay across invalidation/reconnection. A successful send is not physical completion. |
| UAPP crash history | Canonical little-endian format, CRC, bounded architecture context, atomic retained-provider calls and sequence-checked unread/exported/acknowledged/clear lifecycle. |
| UAPP secure update | Policy/capability admission, contiguous staging, atomic checkpoints, authenticated resume, complete-image verification, actual boot/health checks, rollback and security-index advancement after confirmation. |
| UCORE dependencies | Checked span arithmetic/copying and domain-aware deadlines used by these services. The independent codec module remains a scaffold. |

`uos/safertos` remains an explicitly unselected future scaffold: it requires
the licensed kernel and a separate port. No SafeRTOS implementation is claimed.
Bare-metal tasking is intentionally unsupported; inventing a mutex or task
scheduler inside that backend would change its execution model.

## Start with the manual example

The [example](../examples/manual_runtime/README.md) contains
[`runtime.json`](../examples/manual_runtime/runtime.json), rendered C and
[`host_main.c`](../examples/manual_runtime/host_main.c). It exercises an
idle/running controller with application-owned hooks and a guarded transition.
It uses the working host UHAL implementation and needs no peripheral driver.

From the UETD directory, with Python/Jinja2 and a C11 CMake toolchain:

```powershell
python tools/render_manual_runtime.py examples/manual_runtime/runtime.json examples/manual_runtime/generated
cmake -S examples/manual_runtime/generated -B ../build/manual-runtime -DUETD_ROOT="$PWD" -DMANUAL_APPLICATION_SOURCE="$PWD/examples/manual_runtime/host_main.c"
cmake --build ../build/manual-runtime --config Debug
../build/manual-runtime/Debug/manual_application.exe
```

The final executable path above is for a Windows multi-configuration generator.
With Ninja/Make, run `../build/manual-runtime/manual_application` instead.

For the separate project, copy the JSON, provide explicit names/values and
render into a fresh output directory there. Implement the declarations in
`callbacks.h` in application-owned source files and link them with
`manual_runtime`. The optional `MANUAL_APPLICATION_SOURCE` is convenient for
one-file examples; a larger parent CMake project can add its executable and
link that library directly. Configure generated code before adding UETD so
the same capacity definitions shape every translation unit.

The renderer checks identifiers, duplicate/unknown fields, output-name
collisions, references, limits, callback signatures and unreachable transition
rows before writing anything. `"from": "any"` expands to exact-source rows in
declaration order, including self transitions. Generated files are tracked by
hash; edited files are not overwritten. Callback files are never generated
over application work. You can also fill the templates yourself while keeping
these validation rules.

The templates produce executable event-driven FSM wrappers. They do not yet
implement the full UEF-PL compiler's snapshot routing, state publication or
system assembly. Existing module selection templates for every UOS/UAPP
module resolve to current source/header paths. Newly implemented service
manifests are generation-selectable; target deployment evidence is a separate
gate. This is suitable for the requested manual workflow.

## FreeRTOS assembly

Set `"backend": "FREERTOS"` and list static tasks with an operation callback,
priority, stack bytes and microsecond period. See the example README. A task
operation runs immediately once, then follows absolute periodic releases.
Overruns skip missed releases. Callback or OS failures call the application's
required nonreturning failure handlers. Do not start the scheduler after any
task-creation failure; partial creation is intentionally not retried.

The render includes `FreeRTOSConfig.h` and selects its include directory in
CMake. Supply `uos_port_config.h`, the appropriate kernel port/toolchain and
platform. The checked-in integration manifest currently exposes
`arm_cm4f_gcc`; the adapter itself contains no board-specific code. Other ports
need an explicit kernel build/profile with that port's settings. For direct
module-template assembly, define `UETD_FREERTOS_KERNEL_TARGET` as an existing
target carrying its kernel/config/port includes and link requirements.

The port header must provide `configASSERT`, applicable clock/IRQ settings and
`UOS_FREERTOS_YIELD_FROM_ISR(woken)`. The yield hook must return normally to the
UOS wrapper. A port macro that returns from the enclosing ISR cannot be used
directly inside this wrapper; adapt it at the interrupt boundary. The Windows
simulator tests demonstrate this by collecting the yield flag and returning it
from the outer simulated IRQ handler. Only use the `_from_isr` APIs from an
interrupt priority at which the selected kernel permits its APIs.

Static control-block storage and alignment are checked against native kernel
types at compile time. The objects contain actual native static control blocks;
stacks use `uos_stack_word_t`. Generated stacks round the requested minimum byte
size up to complete native elements. CMake publishes
`UOS_FREERTOS_NATIVE_STORAGE=1` and kernel/config includes to every consumer;
apply those consistently yourself when assembling compiler commands manually.
Consumers of an installed FreeRTOS archive must supply the same kernel, port
and configuration headers used to build it; an installed archive cannot choose
a compatible port/configuration on their behalf.
Set `UOS_*_STORAGE_BYTES`/`UOS_STORAGE_ALIGN` consistently
for the library and all consumers if a port needs larger values. Stack sizes
must satisfy the native stack element/alignment rules and the actual callback
stack budget. Finite waits round upward with a tick-phase allowance and reject
overflow rather than becoming an infinite wait. Host tests cover 16-, 32- and
64-bit ticks; actual IRQ priority and target timing remain port validation.

## API changes to apply to manually copied code

UOS now returns `uos_status_t`, with **`UOS_OK == 0`**. Replace old boolean
success tests with explicit comparisons; `if (uos_...)` now means an error.
Time, queue-count and event-wait queries use output parameters. Queue creation
also receives the backing buffer's byte count. Initialize objects with
`UOS_OBJECT_INITIALIZER`, create once, keep their storage alive and never copy
live kernel objects. See [the UOS API](../include/uos/uos.h) and
[contracts](design/UOS_CONTRACTS.md) for exact signatures/context rules.

Bind UOS time once before task creation, using `uos_clock_bind` or
`uos_clock_use_platform` after platform/UCORE time initialization. Until then
time returns `UOS_NOT_READY`. The clock provider reports monotonic microseconds;
bare-metal delay also requires its delay callback. There is no assumption that
CPU clock frequency equals the monotonic timer frequency. If UOS timestamps
and UAPP health/fault timestamps are compared, bind them to the same UCORE
epoch or convert explicitly. A change of clock domain/generation invalidates
deferred deadlines and command expiry rather than silently reinterpreting them.

State machines and provider service objects use their documented zero
initializers and one task/main-loop owner. Configurations, context, static
slots and borrowed tables must outlive their use. Guards must be side-effect
free. Callbacks are synchronous; work budgets limit invocation counts, not
callback duration. A failed callback may already have changed external state.
Fault handling must make that state safe before a reset is requested.

Other tasks/ISRs do not call FSM APIs directly. A FreeRTOS ingress queue can
carry copied events to the owner; bare metal needs a qualified latch/ring or
another serialized ingress mechanism. Ingress overflow and FSM overflow are
different failures. An emergency inhibit must not depend solely on a lossy
ordinary event FIFO. Lifecycle stop callbacks now return `uetd_status_t`.

## Provider responsibilities

These are implemented portable services with explicit providers, not default
success stubs. Host test doubles exercise failure behavior; they must not be
copied as production authentication, crypto or retained-storage providers.

| Service | Application/platform must supply |
|---|---|
| Diagnostics | Record schema validation, authorization, bounded synchronous maintenance, log delivery and terminal-result handling. Execution failure is never automatically retried. |
| Product session | Authenticated stable peer identity, current authority/output policy, bounded send and trustworthy matching completion evidence. `BUSY` from send means no bytes were accepted; other send errors produce an unknown outcome. |
| Crash history | Atomic retained read/commit/clear, compare-and-swap sequences, durable sequence high-water across clear and architecture capture. Sequence exhaustion does not wrap. Ordinary API use is owner-only; HardFault capture needs a separately qualified path. |
| Secure update | SHA-256, signature verification using protected trust anchors, exact canonical-manifest/descriptor binding, target/load compatibility, complete artifact/segment verification, power-loss-safe staging/boot selection, real health evidence and protected monotonic security index when required. |
| Watchdog | The actual UPAL/platform refresh function and an application policy deciding which health reports justify feeding. All feeds for that watchdog must pass through its gate. |

The update orchestrator requires staging, trial boot and rollback even for
explicit unsigned development. Authentication is implied by recovery,
anti-rollback and encryption requirements. Descriptor data remains immutable
and alive for every call on a session, including rollback after failure.
Only one owner uses a backend; a stale checkpoint check does not replace
serialization. Session time limits govern receiving/finalizing; boot health
and its confirmation window are reported by the boot provider. A rebooted
session authenticates and validates persisted state again before continuing.

## Which other U* work should come next?

Choose the rows needed by the first application's data path; there is no need
to finish the entire catalogue before assembling a project.

| First-project feature | Work still needed |
|---|---|
| Portable state-machine/lifecycle demonstration | None beyond application callbacks. Use the runnable example now. UCORE base services, host UHAL, UMID health/log/ring buffer and the implemented UOS/UAPP paths are available. |
| Shared latest values, fan-out or pooled payload ownership | Finish the selected **UMID mailbox, event bus and buffer pool** APIs. They still return `UETD_NOT_SUPPORTED`. A simple FreeRTOS FIFO ingress can use the implemented UOS queue without these modules. |
| Physical peripheral access | Implement/qualify the chosen **UHAL platform/time/IRQ** and **UPAL** devices. Host UHAL is implemented; an architecture or target record alone does not make a real peripheral work. See the UHAL porting guide. |
| Controlled physical output | Finish the required **UACTUATOR actuator** authority, expiry, feedback and safe-output behavior plus the device's UPAL/provider path. Its shared actuator API remains a scaffold. |
| Peer commands or telemetry | Implement the selected **UPROTO** transport/protocol and its authenticated identity/evidence adapter as needed. Product-session bookkeeping does not implement the wire protocol. |
| Persistent settings, logs or crash/update recovery | Supply the selected **UMID storage/settings** or another real atomic storage provider and the relevant UPAL media driver. Crash/update tests use injected in-memory providers. |
| Secure communication or field update | Finish **USEC/provider** or integrate an independently vetted provider directly, plus key/entropy/boot/storage management. USEC currently remains a scaffold. |
| Generic serialization through UCORE codec | Finish **UCORE codec**. UAPP crash/progress encoding does not depend on that unfinished API. |
| Numerical/control processing | Verify and implement the particular **UMATH/UCON** operations selected by the application. A catalogue entry or compilable outline is not sufficient. Unused algorithms are not blockers. |

For a broader portable application, UMID messaging is the most useful next
shared piece. Add UACTUATOR before relying on its API for physical outputs, and
the selected protocol/storage/security modules when those features enter the
project. The source/registry state may advance independently; the manifest's
availability and actual function implementation should agree before selection.

## Validation and its limits

Completed host validation on 2026-10-30: **27/27 CTest cases in both Debug and
Release**, **63/63 uef-gen Python tests**, and the **6 manual-template tests**
(included in CTest). The generated host application built and ran. Public
UOS/UAPP headers compiled as C++17 for bare-metal and native FreeRTOS storage.
The module-route audit found zero integrity errors or missing source/header
references. Release compilation still reports an unrelated integer-to-scalar
conversion warning in `src/umath/matrix/core.c`; that numerical module is
outside this implementation pass.

The host suite covers FSM order, guard priority, copied payloads, FIFO wrapping,
capacity boundaries, fault/reset behavior, lifecycle fault preservation,
deferred cancellation/reentrancy, checked arithmetic, health cadence and actual
watchdog reports. Service tests inject expired/revoked commands, corrupt crash
records, stale sequences, failed atomic commits, failed signatures/images,
recovery after confirmation and anti-rollback failures.

FreeRTOS integration tests execute the vendored kernel using its Windows port
with 16-, 32- and 64-bit tick types, including simulated ISR queue/semaphore/
event operations. Generated periodic tasks are also compiled and exercised for
startup failure, first-release cadence and callback failure behavior. Template
tests render every UOS/UAPP source route, check the committed manual example
and reject invalid configuration. The example is compiled and run separately.

```powershell
cmake -S . -B ../build/uos-uapp-first-pass -DUETD_BUILD_TESTS=ON
cmake --build ../build/uos-uapp-first-pass --config Debug
ctest --test-dir ../build/uos-uapp-first-pass -C Debug --output-on-failure
```

These checks establish portable behavior and build integration. They do not
establish interrupt latency, worst-case callback time, flash power-loss
behavior, cryptographic soundness or electrical safe-state behavior for an
unselected target. Those checks belong with the concrete providers and port.
