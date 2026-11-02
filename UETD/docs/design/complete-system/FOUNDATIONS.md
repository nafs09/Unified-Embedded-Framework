# Foundations, hardware and runtime

This document defines proposed cross-layer contracts. Existing headers retain
their current ABI until an explicitly versioned migration implements them.
All new capabilities are design-only.

## Core values, errors and checked storage

UCORE owns fixed-width storage, status conversion, checked size arithmetic,
byte order, spans, and clock-independent duration/deadline values. It must not
include a vendor SDK or depend on UHAL to define a type or compiler attribute.

Introduce separate immutable-byte and mutable-byte spans with pointer and
`size_t` count. Empty spans may have a null pointer; nonempty spans may not.
Pointer arithmetic and `count * element_size` are checked before access.
Typed public C APIs retain their actual element type; byte spans are for
untyped storage, transport and codecs, not an excuse to erase algorithm types.

Use `uetd_i*` consistently for signed storage. Public enum layout is not a
wire/storage ABI. Serialization uses fixed-width fields with explicit order,
range and version. Checked read/write cursors contain the span, offset and a
sticky error; a failed field does not advance the cursor. Record parsing first
validates total bounds, discriminants and lengths, then publishes a complete
typed result. Generate codecs from project schemas, including endian and CRC
policy; never serialize C padding or pointers.

Retain the current UETD status domain and the UOS distinctions specified in
[UOS contracts](../UOS_CONTRACTS.md). New result-bearing APIs distinguish
invalid, not-initialized, unsupported, busy, empty/full, expired, cancelled,
overflow, transport, integrity and numeric failures as applicable. A conversion
table must state which distinctions are intentionally collapsed. A neutral
value such as zero samples is not the only way to report a failed read.

Scalar-returning legacy APIs remain available only under their documented
preconditions. New fallible operations use status plus output. Success means
the documented operation completed, rather than merely reaching an SDK call.
Async submission success means accepted ownership; terminal completion is a
different event.

Compiler attributes and required integer widths have compile-time checks.
Unsupported weak overrides are explicit: MSVC's empty weak macro does not
promise ELF-style function replacement. Overrideable assertions/fault hooks
need a build-selected implementation or callback on those toolchains.

Acceptance: boundary sizes near `SIZE_MAX`, null/empty spans, overlap policies,
unaligned endian reads, truncated/oversized records, unknown enum values,
incremental CRC known vectors, and C11/C++ inclusion. Fuzz codecs through a
host provider with a strict maximum input length.

## Monotonic time and deadlines

UCORE defines time; the selected clock provider supplies it. Each node has one
monotonic service clock with a named domain, epoch generation, resolution,
frequency/error bounds, and documented behavior during sleep and frequency
changes. UOS deadlines, freshness, logging and watchdog leases use that domain.
Wall time and remote timestamps are separate types/contracts.

A proposed deadline value is `{mode, at_us}`, where `mode` is poll, finite, or
forever. This avoids using a representable finite duration as an infinity
sentinel. `deadline_from_duration(now, duration, out)` checks addition and leaves
`out` unchanged on overflow. Finite waits carry the same absolute deadline
through all layers; bus acquisition and retries do not each restart a timeout.
Only APIs that support indefinite blocking accept forever.

The clock provider extends finite-width hardware counters with qualified wrap
handling. It must state how many unobserved wraps can be detected and how an
interrupt-disabled interval affects this. Converting raw ticks directly to a
64-bit return type does not extend the epoch. A clock reset increments a domain
generation so consumers invalidate old deadlines/samples; it cannot look like
a huge fresh time interval or a valid backward jump.

Periodic release uses checked arithmetic: find the first release after the
current observation in the configured sequence, report missed releases, and
commit the new deadline only if representable. Choose catch-up, skip, or fault
explicitly; an unbounded catch-up loop is disallowed. Tick conversion rounds a
positive finite duration up without converting it to an indefinite wait.

UTC/TAI-like wall-clock support is optional and reports synchronization state,
uncertainty and source. A wall-clock step does not change control-loop time.
Remote sample conversion requires an offset/rate estimate plus an uncertainty
budget. PTP, SNTP or GNSS time providers cannot silently claim a hardware
timestamping accuracy unsupported by the target.

Acceptance: hardware/tick wrap, zero-rate initialization, quantization,
`UINT64_MAX` arithmetic, late release, sleep/wake, DVFS, source failure and
multiple clock domains. The provider must have a physical timing check for
every accuracy/resolution guarantee requested by a project.

## Exact-target resource graph

Extend the existing family/chip/board records in place. Families organize
discovery. Chip/package records own silicon resources; boards add oscillator,
external devices, electrical constraints and reserved wiring. A board profile
references exact silicon/package/revision and a qualified build profile.

| Resource | Required facts and constraints |
|---|---|
| Pins/pinmux | Package ball/pad, alternate-function options, analog/digital mode, voltage domain, drive limits, pulls, reserved/debug/boot pins and board wiring |
| Clocks | Oscillator source/tolerance, PLL/divider graph, allowed ranges, consumers, shared roots, runtime-change permissions and start/lock deadlines |
| Interrupts | Vector, ownership, shared dispatch, priority width/grouping, nesting, RTOS-callable range, wake capability and bounded handler route |
| DMA | Controller/channel/request mux, direction, transfer width/burst, accessible memory, alignment, circular/double-buffer support and conflict groups |
| Timers/triggers | Counter width/rate, capture/compare channels, preload/deadtime/break inputs, trigger graph, synchronization cohorts and update points |
| Memory | Address/size, core visibility, executable/write attributes, cache/MPU properties, DMA reachability, retention, erase/program geometry and reserved regions |
| Reset/power | Reset and clock/power domains, shared reset effects, boot owner, wake sources, reset-cause capture and secondary-core release |
| Analog | ADC rank/trigger/sample-time constraints, reference range, calibration, comparator/DAC routing and hardware protection paths |
| Build | Startup/vector sources, linker inputs, ABI/FPU/float calling convention, SDK/config, compiler profile and programming method |

Resource requirements are typed predicates, not merely `UHAL_HAS_*` booleans.
For example, synchronized acquisition requires a legal timer-to-ADC trigger
edge, a DMA route to a reachable aligned buffer, and a sample/conversion time
that fits the trigger period. A timer that advertises PWM does not prove it
supports complementary outputs and a hardware break input.

The resolver reserves resources transactionally. It may propose alternatives
with an explanation, but cannot silently select a different pin, sample rate,
voltage reference or safety route. Shared buses require an explicit arbiter;
exclusive hardware requires one owner. Custom-board overlays live in project
inputs, identify their base record, and become part of the build fingerprint.
Imported SVD/CMSIS/SDK data is provenance-bearing draft material; it does not
override a datasheet/erratum or become verified board wiring automatically.

Acceptance: conflicting AF assignments, shared IRQ/DMA conflicts, unreachable
buffers, forbidden boot/debug pins, sample-time oversubscription, incompatible
clock roots, alignment/region overlap, and an explainable valid assignment on
each qualified board. Unsatisfied constraints identify both competing users.

## Startup, memory protection and CPU services

Startup is an explicit dependency graph: reset-cause capture; essential power
and clocks; memory initialization; vector/interrupt setup; monotonic time;
storage recovery and credential access where required; safe I/O preload;
drivers; services; application activation; watchdog confirmation. Interrupts
are enabled only after their destination state/buffers exist. Constructor
order or file-link order must not define device readiness.

Each memory region has a linker owner. Generate named partitions for code,
stacks, pools, DMA, retained records, configuration, update slots and optional
external memory. Check erase-block boundaries and bootloader reservations.
External RAM must be trained/initialized before use. RAM testing, ECC policy,
MPU/TrustZone isolation, stack guards and execute permissions are capabilities
with target-specific evidence. A security boundary cannot be inferred merely
from placing data in a named section.

UHAL provides CPU identity, reset request/cause, idle, cycle/timing diagnostics,
critical sections, atomic widths/order and optional fault-register capture.
Hardware unique IDs are identifiers, not secret keys or entropy. Reset/fault
paths report whether backup/retained storage is supported and commit only the
bytes that storage can actually retain.

Before reset, outputs use an application-declared local safe action if that
action is possible in the current context. Hard fault handlers capture only
bounded architecture-safe data. Formatting, general locks, network access and
filesystem writes do not run in fault context.

## Driver transaction and DMA ownership

Every asynchronous driver uses the same conceptual transaction lifecycle:

```text
IDLE -> QUEUED -> ACTIVE -> COMPLETE | FAILED | CANCELLED
                     -> CANCEL_REQUESTED -> terminal state
```

Each submission has an instance ID and monotonically changing generation/token.
A completion identifies that token, terminal status, transferred element count,
timestamp/domain and hardware error flags. There is exactly one terminal
completion for every accepted request, including abort/reset paths. A late
interrupt for an old generation is discarded and counted; it cannot complete
new work using reused storage.

The public submission contract states copy versus borrow, source/destination
lifetime, alignment, permitted overlap, cache policy and callback context.
Borrowed storage remains owned by the transaction until terminal completion or
an explicitly synchronous cancellation acknowledgement. Requesting cancellation
alone does not release a DMA buffer.

No provider may let DMA read/write arbitrary caller memory before validating
the route. Cached receive buffers use full cache-line ownership or a bounded
bounce buffer; invalidating a partial line shared with unrelated data is
forbidden. TX cache clean, RX invalidation, barriers and descriptor visibility
follow the exact target profile. Static arenas/pools have calculable capacity,
including the peak overlap of queued, active and completion-pending transfers.

Completion queues and ISRs have explicit maximum work. ISRs acknowledge owned
hardware flags, capture bounded results and enqueue/defer. Queue-full policy
must retain a terminal outcome through a sticky completion slot or another
bounded mechanism; silently losing completion leaks buffer ownership.

Acceptance: zero/oversized/unaligned requests, queue full, partial transfer,
timeout at every phase, cancel/complete races, stale IRQs, bus reset, buffer
reuse, cached DMA and a deliberately slow completion consumer.

## Existing peripheral families

The existing public APIs are retained as migration inputs. The following
policies complete their implementation design; they do not create another
copy of each API in this document.

| Family | Required implementation profile |
|---|---|
| GPIO/EXTI | Preload output before enabling drive; pin/AF validation; edge and debounce ownership; status-bearing configuration; bounded event ingress; safe level on release/reset |
| UART/RS-485 | Baud error, parity/stop/inversion/flow control; blocking and async deadlines; RX producer accounting and overrun; IDLE/error events; DE assertion/release after physical transmit-complete; turnaround/collision policy |
| SPI | Controller/peripheral role, mode/bit order/word width, CS lifetime across segments, half/full duplex, shared-bus atomic transaction, setup/hold/interword bounds, abort to idle and device-specific maximum rate |
| I2C/I3C | Address format and reserved addresses; repeated-start/combined transactions; arbitration/NACK/bus-stuck distinctions; bounded recovery; topology ownership. I3C is a separate optional provider with discovery/address/hot-join/IBI policy |
| Timer/PWM/capture/HRTIM | Frequency/quantization, compare limits, preload commit, complementary/deadtime/break rules; overcapture and rollover; capture timestamp domain; synchronized update cohorts; hardware-safe output disable |
| ADC/DAC/COMP/OPAMP | Reference/calibration and gain; units versus raw counts; scan ordering; trigger/settling/conversion deadlines; overrun; simultaneous-sample skew; saturation and independent comparator/break protection |
| CAN/FDCAN | Controller clock and validated bit timing, transceiver role, filters/FIFOs, canonical DLC/payload mapping, bus-off recovery, error counters, timestamping, and bounded TX arbitration/queue policy |
| Flash/QSPI/OSPI/EEPROM | Read/program/erase geometry, wear, alignment, XIP/cache/read-while-write restrictions, execute-from-RAM requirements, write protection, power-fail boundary and asynchronous completion |
| SDMMC/block storage | Sector geometry and LBA width, card insertion/removal/write protect, initialization state machine, DMA/cache buffers, exact block counts, sync/flush and media generation across replacement |
| USB | Device/controller and host role separation, endpoint allocation, bus reset/suspend/resume, class lifecycle and descriptor ownership; class functions are implemented above this boundary |
| RTC/watchdogs | Wall-time quality, alarm/wake/backup capacity, reset-cause semantics; watchdog oscillator tolerance, min/max feed window, reset behavior and exclusively routed refresh authorization |
| CORDIC/FMAC/CRC accelerators | Exact numeric format, scaling/rounding/saturation, fallback parity, exclusive instance ownership, DMA lifetime, and distinguishable unsupported versus failed operation |
| Ethernet/radio/netif | The active provider boundaries remain authoritative; actual MAC/PHY/HCI/radio providers declare MTU/frame limits, DMA/cache, events, controller firmware and target resources |

Additional peripheral families should enter as optional typed provider classes:
hardware RNG, I2S/PDM/SAI, display scanout, camera capture, touch/keypad, one-wire,
low-power timers, external memory, and selected programmable-I/O/FPGA bridges.
These are capabilities with pin/clock/DMA/IRQ budgets, not promises that every
target supports them. A generic byte bus is insufficient for their real-time
or framing constraints.

## Power and clock management

Create a power manager with named device/power-domain states, bounded transition
latencies, wake sources, retention and lost-context declarations. Services
acquire constraints/leases such as active-transfer, clock-minimum,
must-retain-memory, or wake-before deadline. The manager selects only a legal
state satisfying all active constraints and the project's energy policy.

Prepare/suspend/resume is transactional: stop new admissions, drain or cancel
work under a deadline, persist required state, set safe outputs, enter sleep,
restore clocks/memory/providers, reestablish time, then republish readiness.
Failure rolls back to a known active state or enters declared recovery. Never
sleep through an armed safety deadline or retain pointers into powered-off RAM.

Dynamic clock changes have a generation number. UART/timer/ADC configurations
either hold a clock lease or receive validated reconfiguration at a quiescent
boundary. DMA/comms may prohibit a clock change. Battery state, charger and
fuel-gauge providers publish measured quality and thresholds; the manager can
request reduced duty cycle but does not invent battery chemistry limits.

Acceptance: pending DMA veto, wake deadline, timer wrap across sleep, brownout,
lost peripheral state, stale sensor invalidation and measured energy/latency
on the selected target. No fixed universal current-consumption claim is made.

## UOS execution and synchronization

Use the status, storage, ISR and time contract in the existing UOS design.
Add backend capabilities for tasks, static queues, semaphores, mutexes,
notifications/events, timers and deferred work. Bare-metal implements only
features it can actually execute; a cooperative run loop is a real backend
with explicit scheduling, not a set of task-creation success stubs.

Task configuration owns stack bytes/alignment, priority, affinity, release
policy and blocking budget. All kernel static-object sizes are checked against
the pinned port at build time. Object initialization, use, quiescence and
destruction are explicit; copying an initialized opaque object is forbidden.
Mutex ownership/priority inheritance and ISR-callable priority ranges belong
to the backend profile. An ISR API always has zero wait and a documented yield
handoff. Timers enqueue work or run only bounded callbacks in a declared context.

Host simulation supplies actual host synchronization for claims involving
multiple host threads. Its current no-op PRIMASK simulation is suitable for
serialized tests and cannot validate the logger/health/fault queues under
concurrent host producers. Target interrupt masking likewise does not provide
cross-core exclusion.

Deadlock prevention uses a declared lock order and checks blocking while
holding incompatible resources. The generator rejects a hard real-time route
through indefinite locks, allocation, filesystem writes, or network waits
unless the application changes its requested guarantee.

## Intercore and distributed boundaries

AMP is the default design for heterogeneous cores. Each image owns its startup,
runtime, interrupts, memory and peripherals. Shared-memory channels declare
single/multi-producer topology, cache-line placement, atomic widths and memory
ordering, fixed record/pool capacity, core-reset generation and notification.
Use offsets/handles validated against a shared region; ordinary virtual
pointers are not an intercore or wire ABI. A reset peer invalidates outstanding
handles and forces an explicit rebind/recovery sequence.

SMP is selectable only for an exact RTOS/port/target combination with qualified
locks and scheduler semantics. Per-core interrupt masking is never a global
lock. Multi-node messages additionally specify clock-domain/freshness limits,
loss/retry/deduplication policy, and local output behavior on link loss.

Generation emits separate images when the target startup model requires them,
plus one system bundle describing compatibility and deployment order. Partial
node success cannot be labelled a successful whole-system deployment.
