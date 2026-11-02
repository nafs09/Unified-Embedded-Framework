# Product and data services

These are first-pass design requirements unless explicitly listed as later
scope. Missing implementations remain generation-blocked in the main
catalogue. The shared execution, deadline, buffer and target rules in
[Foundations](FOUNDATIONS.md) apply to every service below.

## Buffer pools, events and dataflow

Provide a fixed-block pool, copied bounded queue, latest-value mailbox and
typed event bus. They have different semantics and cannot be substituted by
one generic `publish()` call.

| Primitive | Contract |
|---|---|
| Pool | Static arena, block size/alignment/count; generation-tagged handle; allocate/free status; invalid/double-free detection; no allocation while in a forbidden context |
| Queue | Fixed record size/depth, producer/consumer topology, FIFO order per producer, overflow policy, bounded wait and exact accepted count |
| Mailbox | One complete latest value plus sequence/quality/time; a publish may replace unread data; readers can detect skipped versions |
| Stream | Ordered byte/sample segments, framing independent of transport, producer/consumer positions, overrun, partial transfer and release protocol |
| Event bus | Registered typed topics and fixed subscriber list; explicit queue/mailbox delivery for each subscriber; source context, work budget and drop accounting |

Default event delivery copies small records; large data uses a pool handle with
explicit ownership/reference-release rules. No pointer into a stack or DMA
buffer escapes its lifetime. Broadcast allocation reserves all required
references before publication or reports the documented partial-delivery
result. Critical conditions also use a sticky cause/inhibit path and are not
dependent on the availability of a normal event queue.

The generator derives capacity from maximum burst and service latency, using
`capacity >= burst + ceil(maximum_rate * maximum_unserviced_time)` as a necessary
bound when those assumptions hold. It reports the assumptions and adds any
required active/in-flight ownership. This formula alone is not a general
queueing proof or a substitute for a protocol-specific backlog analysis.

Acceptance: wrap, full/empty, reset generation, subscriber removal at quiescence,
slow/failed consumer, nested producer contexts, pool exhaustion and refcount
release. Host concurrency tests use real synchronization rather than no-op
interrupt masks.

## Sensor and acquisition services

Keep device-specific register drivers project/provider-owned. Add reusable
typed sensor instance and acquisition-group contracts above those drivers.
Configuration includes device identity/revision, bus transaction limits,
calibration reference, sample rate/mode, timestamp source, maximum age and
published frame/units. Per-device register details remain outside the common
sensor API.

A sample envelope carries a typed value, validity/quality flags, source
sequence, capture timestamp and clock domain/generation, and calibration
generation. A last-valid sample is distinct from a current-valid sample.
Queries specify acceptable age and quality; callers never infer validity from
a nonzero measurement or a previous successful initialization.

Sensor state is `unconfigured -> configured -> starting -> running`, with
`stopping`, `faulted`, and bounded recovery branches. An acquisition operation
may return a sample, no-new-data, stale, overrun, disconnected, invalid, or a
provider failure. Retry budgets and warm-up/settling intervals are explicit.
An invalid read cannot publish partially updated axes/channels.

Acquisition groups declare trigger cohort, clock domain, sample order, skew
budget, DMA frame count, decimation/anti-alias policy and consumer deadline.
The driver publishes the time the data was captured, rather than the time a
task eventually parsed it. If only a later timestamp is available, the envelope
reports its uncertainty instead of pretending it is a hardware capture time.

Core driver categories include IMU, magnetometer, encoder, GNSS, pressure,
temperature, range, voltage and current. Add digital/debounced inputs, switch
and limit inputs, load/strain bridges, optical/light channels and battery/power
status through the same sample contract when required. New physical devices
do not require a new generator grammar rule.

Calibration records bind device/serial identity, calibration schema/version,
units/frame, generation, integrity and optional provenance/temperature range.
Drivers own calibration mathematics. Application of an incompatible record
fails visibly and does not rewrite the record as if it were valid. Zero/span,
bias/scale/misalignment and temperature models have separate typed payloads.
Calibrations commit atomically and activation occurs at a declared sample
boundary. Rollback and factory defaults remain distinct operations.

Acceptance: units/frame conversions, late/missing/out-of-order samples,
multiaxis snapshot coherence, calibration incompatibility, startup settling,
rate/trigger mismatch, disconnect and a target-measured skew/timestamp check.

## Actuator services and local safety

Expand the current H-bridge authoring identity into a small reusable actuator
family. Providers own electrical implementations; UETD owns admission,
command-state, timeout and safe-output semantics.

| Class | Typed command/configuration | Required checks |
|---|---|---|
| Digital/relay | Desired state, polarity, minimum on/off time, power-on/off state | Interlock, switching rate, stale command and bounded release |
| PWM/analog | Duty/voltage/current demand, range, update period, slew bound | Saturation reporting, preload/update point and disabled-state level |
| RC servo | Position/angle or calibrated pulse width, range and rate | Explicit calibration, clipping, command age, disable/hold policy |
| Step/direction | Position/velocity/acceleration, pulse timing and direction setup/hold | Timer/DMA capacity, missed-step observability, limits and stop distance |
| H-bridge/motor | Enable/mode, signed demand, current/velocity/position feedback | Deadtime, reversal rules, overcurrent, feedback age and hardware inhibit |
| Protocol actuator | Typed demand and peer mode/profile | Wire encoding, heartbeat/ack semantics, bus budget and local loss-of-link policy |

Every actuator has `disabled`, `arming`, `enabled`, `stopping` and `faulted`
states. Arming requires declared permissives, valid calibration/configuration,
fresh required inputs, and target readiness. A command includes sequence,
receiver-domain expiry and authority. A successful admission is not a claim
that physical motion occurred; feedback and execution completion are separate.

The command path reports requested, limited and applied values. Saturation,
rate limiting and actuator tracking feed back to the controller where its
anti-windup contract requires it. A control mode change has a declared bumpless
transfer/reset policy; assigning another algorithm's output is insufficient.

Safe action is selected per device: de-energize, brake, coast, zero demand,
controlled stop or hold only when the hardware and hazard model permit it.
Emergency/hardware trip paths act independently of the ordinary task loop.
Restart requires the declared acknowledgement and fresh permissives; clearing
a diagnostic record alone cannot re-enable outputs.

Acceptance: stale command, failed permissive, limit switch, unexpected feedback,
trip during DMA update, watchdog reset, communication loss and safe startup
pin levels. High-energy/medical mechanics require the application's hazard
and actuator profile; the generic service cannot choose those limits.

## Storage, partitions and filesystems

First-pass persistence includes a block/device contract, named partitions,
transactional records, settings/calibration and bounded acquisition logs.
The existing storage/calibration/blackbox/process-image design contracts are
first-pass contracts, even while their operations remain unimplemented.

The block provider reports geometry, read/program/erase units, total capacity,
erase count/wear information where available, removable-media generation,
write-protect state, durability/flush behavior and power-fail atomic unit.
Filesystem adapters use this contract or an explicitly pinned native SDK
backend. FatFS and a small flash-filesystem adapter are optional providers;
UETD does not infer transactional record safety from `write()` returning a
byte count.

Partitions have unique roles and nonoverlapping aligned ranges. Bootloader,
application, staging, settings, calibration, retained logs and user files are
separate owners. A read-only firmware region must not overlap a writable
configuration arena. External/QSPI flash advertises its execute-in-place and
read-while-program restrictions to scheduling and update planning.

Transactional records contain magic, schema/version, identity, length,
generation, payload integrity and a commit marker protected against torn writes.
Use the existing persistent-record schema as the envelope authority. A minimal
implementation uses alternating slots or an append journal with a bounded
index. Recovery scans only the configured arena, validates a full record before
accepting it, and picks the newest **valid committed** generation using a
specified wrap policy. A partially programmed marker never establishes a
valid record. Multi-record transactions are a separate optional capability.

Compaction has an independent reserve region and power-loss recovery. Report
full, wear-limited, media-changed, read-only, corrupt and incompatible-schema
states separately. Compute worst-case storage growth, reserve and write rate
from the product profile. No claim of unlimited endurance or lossless logging
is made when sustained input exceeds durable throughput.

Binary blackbox records carry stream/schema ID, sample sequence, quality,
capture time/domain and bounded payload. Batching and queue depth are explicit;
drop-newest/drop-oldest/stop-acquisition policies are chosen by the application.
Critical fault metadata uses a separately reserved path. A flush timeout
reports which durability boundary was actually reached. Provide a host decoder
bound to the exact schema/build bundle.

Acceptance: power interruption at every program/erase/commit/compaction step,
corrupt length/integrity, generation wrap, read-only media, full arena, card
replacement, schema upgrade/rollback, alignment/cache failures and recovery
time bounded by the configured storage size.

## Settings and configuration transactions

Settings are typed, range/unit-checked values with defaults, mutability class,
authorization requirement and activation policy. Distinguish immutable build
configuration, boot-only settings, quiescence-required settings, and live tuning.
Credentials are opaque references owned by security providers.

An update follows `read generation -> stage candidate -> validate cross-field
constraints -> reserve dependent resources -> persist if required -> activate
at the declared boundary -> publish new generation`. Failure leaves the last
valid configuration active or follows an explicitly documented recovery state.
A compare-generation guard prevents lost updates by two maintenance clients.
Validation covers dependent sample rates, saturation bounds, memory limits,
bus timing and actuator permissives, not only each scalar's minimum/maximum.

Schema migration is a bounded, versioned transform. Unsupported versions reject
or enter a declared recovery/default policy. Factory reset has a precise scope:
user settings, calibration, identity and keys are separate categories, with
authorization and irreversible-operation handling defined by the product.

Acceptance: interrupted commit, conflict, invalid coupled fields, rejected
resource reservation, rollback, power-cycle activation and secret redaction.

## Lifecycle, supervision and recovery

Extend the implemented component lifecycle with a generated dependency graph,
reverse-order stop/rollback, idempotent cleanup and explicit fault transitions.
An init/start failure records which resources were acquired and invokes their
bounded cleanup; it cannot merely set a state enum while leaving an actuator
armed. Reentrant lifecycle calls are rejected. Configuration-table lifetimes
remain static or otherwise explicitly owned.

The flat run-to-completion FSM remains a useful first-pass primitive. Inputs
arrive through the separate ingress mechanism in the UOS contract. Timers and
state snapshots are typed and bounded. First-pass generation must implement
guard/action/entry/exit failure policy, deterministic transition order, explicit
queue-full behavior, and safe-state callbacks. Hierarchical/orthogonal state
regions are later work unless a concrete first-pass product requires them.

Recovery policies specify trigger, debounce/persistence, maximum retry count,
backoff ceiling, cooldown, dependent-service invalidation, output inhibit and
terminal escalation. An automatic retry never bypasses arming/authorization.
The watchdog gate's lease model remains the common feed policy; a hardware
watchdog profile adds minimum/maximum window and oscillator tolerances.

Fault history is first-pass maintenance infrastructure. Architecture capture,
retained-record format, atomic commit/integrity, boot collection, export and
acknowledge/clear are distinct stages. The current zero PC/LR placeholders
must not be presented as captured context. Prefer a small fixed fault record
over invoking a general storage/network stack during a fatal exception.

Acceptance: failure at each startup/cleanup phase, repeated fault, saturated
event/fault queues, callback failure, watchdog race, safe restart and retained
record corruption or reset during commit.

## Diagnostics, telemetry and maintenance

Retain the bounded text logger and add structured event/metric records with
stable IDs, build/schema identity, severity, sequence and monotonic timestamp.
Each sink has maximum record size, work/deadline, acceptance semantics and
backpressure policy. Text formatting is not ISR-safe. ISR tracing uses fixed
binary records drained later. Log truncation/drop counters are visible.

Runtime observability includes task stack high-water marks, release/deadline
misses, queue/pool watermarks, bus/driver errors, sensor age, reset reason,
storage wear/fullness, clock quality and selected provider versions. Sampling
diagnostics consumes a declared budget and cannot invalidate the timing claim
it is measuring. Host symbolization uses the exact image/debug files.

A diagnostics shell/service exposes typed read/query commands and explicitly
authorized mutation/maintenance operations. Parsing is bounded; commands have
role, context, timeout and audit behavior. Raw arbitrary-memory write, arbitrary
function invocation, unbounded filesystem access and unauthenticated actuation
are not default features. The same command model can use serial, USB or an
authenticated network adapter without embedding transport-specific policy.

Commissioning/maintenance mode gates factory tests, calibration, enrollment,
configuration and firmware update. Enter/exit policy defines output inhibit,
physical presence where required, timeout and reboot behavior. Production
builds explicitly select which debug operations remain available.

## First-pass compositions

The design must support a battery data logger, a deterministic sensor/actuator
controller, an industrial serial/CAN node, a connected telemetry device, and a
multi-node system. Each composition uses only selected services and reports
every missing provider or unsupported guarantee. The detailed test projects
are specified in [Generation and qualification](GENERATION_QUALIFICATION.md#acceptance-projects).

## Genuinely later product packs

The following are useful expansion work rather than prerequisites for the
first usable general generator. Their public integration contracts are fixed
here so later additions fit the same architecture.

| Pack | Contract, resources and lifecycle | Qualification gate |
|---|---|---|
| Audio | I2S/PDM/SAI or codec provider; channel/sample format, rate, frame clock, DMA ping-pong buffers, sample-count clock domain and underrun/overrun recovery | Clock drift, buffer exhaustion, format conversion, sustained throughput and bounded DSP load; no implied voice/telephony stack |
| Imaging | Camera/frame-source provider; pixel format, ROI/stride, frame pool, capture time, bandwidth, DMA/cache and consumer lease | Frame loss, mismatched stride/length, inaccessible memory, disconnect and buffer lifetime; ISP/GPU stacks stay external |
| Inference | Pinned inference engine/model artifact, tensor shapes/layout/quantization, bounded arena, supported operations, execution budget and cancellation/output validity | Reference vectors, operator subset, arena/stack peak, deadline miss and model compatibility; no automatic control/safety decision from an inference result |
| Removable-device discovery | Bus/provider-specific discovery with fixed maximum instances; identity, compatible driver binding, generation-tagged handles and removal quiescence | Surprise removal, identity collision, stale handles, capacity exhaustion and reconnect without stale data |
| Advanced motion/robotics | Kinematics/frame tree, trajectory constraints, collision/environment provider, feedback quality and bounded planning horizon | Independent geometric references, singularity/limit handling, deadline/fallback and actual mechanism model |

Later packs depend on the first-pass resource, ownership, provider, security and
data contracts. Their absence must not prevent a first-pass firmware build.

Display/graphics, touch/input and the UI adapter are first-pass requirements for
the [Myovena handheld](MYOVENA_HANDHELD.md). They remain optional per project;
they are not later-catalogue capabilities.

The [MIPI display-link design](MIPI_DISPLAY_LINK.md) specifies the first-pass
DSI/DSI-2 host, DCS/panel adapter and D-PHY/C-PHY compatibility route beneath
the generic display facade. Actual link signalling requires a compatible
host/bridge and panel; provider qualification remains outstanding.
