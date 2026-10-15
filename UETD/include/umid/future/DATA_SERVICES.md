# UMID persistence, acquisition logging and external process data

**State:** future service contracts. Device-specific sensors and actuators
remain consumer-authored drivers; this proposal standardizes storage,
calibration envelopes, log records and external process-image semantics.

## Common storage service

Provide a backend-neutral, fixed-buffer store with explicit backend capabilities
for internal flash, external NOR, SD/FatFS, or a host test backend. The caller
supplies bounded key, payload and workspace storage. Operations are
`begin/read/write/commit/abort`; a successful commit is the only point at which
the new generation becomes authoritative. A failed or power-interrupted write
must leave either the previous committed record or the new complete record
readable, never a partially accepted payload.

The first portable profile allows one serialized writer transaction per store
instance and concurrent readers only when the selected backend can guarantee
a stable committed-generation snapshot. Keys and all metadata have explicit
byte limits; APIs copy metadata and never retain caller string/payload pointers
after return. `begin` reserves a new generation, `abort` abandons it,
`commit` publishes it atomically, and recovery ignores uncommitted generations.
The backend reports its atomic commit unit, alignment, maximum record size and
worst write stall so callers can reject incompatible configurations before
writing. Every length, offset and size calculation must be checked for overflow.

Each record has a format magic, record kind, schema version, key, sequence,
payload length, target/product identity, SHA-256 and optional CRC32 integrity
checks, and a commit marker. The content hash binds payload identity; CRC32 is
only a low-cost corruption check and neither authenticates a publisher.
The store defines maximum payload size, alignment/erase constraints, atomic
unit, timeout, concurrency model, corruption recovery, previous-generation
fallback, and erase/wear policy. Schema migration is explicit and bounded;
unknown future versions are rejected without rewriting the record.

FatFS is a backend adapter, not the storage transaction contract. The common
API must expose that filesystem writes can stall and must not be called in an
ISR or hard real-time control loop.

## Calibration records

Calibration is still computed and interpreted by the user-owned device driver.
The common calibration envelope contains driver ID, driver calibration schema
version, board and sensor identity/serial where available, units,
calibrated-at clock/time, method/revision, payload length, integrity check,
sequence and commit state. Coefficients, valid input ranges and algorithm
parameters stay inside the driver-owned opaque payload with a length fixed by
that driver's contract; the store does not assume every sensor has
bias/scale coefficients or reinterpret a driver's range encoding.

The lookup key is a stable encoding of driver ID plus sensor identity, not a
human-readable sensor label. A board change, identity mismatch or schema
revision mismatch must never silently reuse a record belonging to different
hardware.

On load, the driver checks identity, schema, units, finite/range constraints
and integrity before activation. Mismatch or corrupt data yields an explicit
uncalibrated/degraded result and preserves the prior slot. Migration must be a
driver-supplied version-to-version function; UETD must not reinterpret device
coefficients.

## Binary blackbox

Use a record-oriented binary stream separate from human-readable `UMID_LOG`.
Every stream header identifies firmware/artifact, target and board revision,
configuration and calibration revisions, timebase, channel schema, and start
reason. Each record has sequence, timestamp or delta, channel/event ID,
payload length and CRC32 integrity check. Loss, time discontinuity, clock reset,
buffer overrun and storage failure appear as explicit records or counters.

The project declares sample/event rates and maximum encoded sizes. Compute the
required queue from measured workload:

`production_bytes_per_second = Σ(rate_millihz_i × maximum_encoded_bytes_i) / 1000 + framing_overhead`

`queue_bytes >= production_bytes_per_second × worst_storage_stall_seconds + burst_margin`

`retention_seconds = usable_capacity_bytes / production_bytes_per_second`

The validator uses checked wide arithmetic, includes record headers/alignment
and burst margin, and fails if any sum or multiplication overflows. Runtime
stats expose accepted, committed and dropped records separately, bytes written,
storage stalls and the last storage error. A record's payload is copied or
fully consumed before `append` returns; no logger backend may retain a pointer
to a control-loop sample buffer.

The selected backend must sustain average production throughput. A blackbox
contract chooses drop-newest, drop-oldest, decimate, or stop-on-full; the
choice is visible in metadata and counters. The storage worker owns erase and
filesystem activity. It may not block a control task or write flash from an
ISR. External NOR support defines sector-ring wrap, wear accounting, committed
page recovery and a reserved fault-record area.

## PLC process image

The generic external-peer adapter presents a typed process image, independent
of Modbus/CANopen/EtherCAT/other transport. Every field declares type, units,
direction, owner, update period, quality/status, source timestamp, stale
timeout, write authority, range, command semantics, acknowledgement policy,
and safe value. Inputs become unusable when
quality is bad or freshness expires. Outputs are staged then committed as one
image cycle; partial field updates are not silently presented as a coherent
scan. Transport adapters report diagnostics but cannot override local
interlocks or local safe-state authority.

The optional adapter also declares scan jitter/latency bounds, reconnect and
resynchronization policy, duplicate/out-of-order behavior, and whether commands
are edge-triggered or level-held. Validation rejects duplicate point IDs,
incompatible type and byte-size declarations, unbounded scans, missing safe
values for writable outputs, and peer authority that bypasses local interlocks.
A `PLC` label alone is insufficient to infer any of those semantics.

## Ownership

Concrete sensor protocol parsing, device-specific calibration math and
electrical behavior stay in handwritten UMID drivers. UETD supplies the bounded
store, envelope/commit semantics and common health/log integration only after
the future service contracts are promoted.
