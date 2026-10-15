# Persistent fault and crash history contract

**State:** future retained-storage service. The current recoverable fault queue
is implemented in RAM; current fatal reset metadata uses only a few backup
register words. See `fault_history.h` for the design-only typed outline
and `registry/future/crash_record.schema.json` for the canonical record
envelope. The C structure is never itself a persistent binary ABI.

A retained crash record is valid only when its magic, format/schema version,
sequence and integrity check pass. It records fault code/type, reset cause,
monotonic or RTC time with an explicit clock ID, exact artifact digest,
chip/board/core identity, task or interrupt identity, faulting PC/LR, architecture
fault status registers, and the stacked register/FPU context available from
the platform. Missing context fields are explicitly invalid; zero does not
mean “captured as zero.” A variable detail string is bounded, copied, and
marked truncated.

Commit is atomic across reset and brownout. Use a reserved backup-SRAM/flash
slot or journaled record; do not write a filesystem from HardFault context.
The design specifies what can be written with interrupts disabled and what
must be captured by the architecture fault entry before the C hook. A later
task exports the record to text logger/blackbox or persistent storage.

Each architecture backend publishes a versioned context schema and maximum
context length. Capture validity bits distinguish unavailable registers from
captured zero values. If emergency stack or retained storage cannot hold the
full context, preserve the fixed identity/fault header, record the truncation,
and never claim the omitted fields were captured. A partially written marker
is invalid on boot; the previous verified record remains readable.

Record lifecycle: unread, exported, acknowledged, cleared. Reading is
non-destructive; acknowledgement names the exact sequence; clear requires an
explicit API and cannot accidentally erase an unexported newer fault. Repeated
resets increment a bounded sequence/boot-loop counter and preserve the newest
valid and previous valid record where capacity permits. CRC detects corruption
only and is not cryptographic authentication.

Export and acknowledgement metadata must itself survive reset without
rewriting or losing the crash payload. Sequence comparison defines wraparound
rules, and clear targets one sequence after verifying the caller's expected
sequence. Startup recovery reports duplicate, corrupt, missing and overwritten
records through explicit status values.
