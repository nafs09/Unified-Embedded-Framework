# Future `.uefs` external hardware and qualification contract

**State:** design-only extension to the system-composition model. External
devices remain independently authored and built unless a separate toolchain
adapter is explicitly selected.

## External artifact identity

An external endpoint record identifies endpoint kind (`fpga`, `plc`, or a
project-defined typed peer), owning project/system, logical interface schema
and version, transport binding, role/direction, and whether UETD builds the
artifact. For externally built RTL, evidence may include source repository and
revision, source-tree digest, FPGA part, HDL tool and version, constraints and
IP package digests, generated bitstream path/hash/size, and build owner. These
facts are provenance, not proof of timing or electrical correctness.

An FPGA contract additionally records clock sources/frequencies, clock-domain
crossing assumptions, reset sequencing, pin/voltage standard ownership,
constraints, maximum protocol rate/latency, and the exact MCU-side protocol
contract. The MCU project continues to own the link parser/codec. UETD does not
claim to generate the HDL or bitstream.

## PLC process-image binding

An optional PLC endpoint maps a cyclic process image to an explicit fixed
record. Every point declares type, units, direction, producer/consumer
authority, cycle, source timestamp domain, quality flags, stale limit,
valid range and safe value. A scan is committed atomically: consumers observe
one complete image generation and its sequence/time/quality metadata. Partial
or stale scans cannot appear valid by retaining old field values.

Writable points declare command authorization, range validation,
edge-triggered versus level-held semantics, acknowledgement and timeout. The
local MCU remains authoritative for interlocks and safe output state. Protocol
adapters (Modbus RTU/TCP, CANopen, or later vendor fieldbus modules) bind the
typed image without changing its safety or freshness semantics.

## Evidence records

The evidence schema links a claim to exact bytes and exact conditions. At
minimum it identifies:

- artifact/source/UETD fingerprint, module IDs and configuration hash;
- chip, board, hardware revision, runtime, toolchain, compiler flags and
  external artifact hash where applicable;
- evidence class (`schema-reviewed`, `host-tested`, `target-built`,
  `target-exercised`, `system-HIL`, or separately assessed safety/security
  claim), test/harness revision, date and owner;
- environment such as supply range, temperature, clock profile, connected
  peripherals and external peer revision;
- test identity, outcome, logs/raw-data digests and measured values including
  WCET, interrupt latency, jitter, stack/RAM/flash, throughput, packet loss,
  recovery time and fault-injection result where relevant;
- limitations, exclusions and expiry/review trigger.

Evidence attaches to an exact capability and profile. A system-level claim is
valid only when each selected component/link capability has compatible
evidence, and the aggregation policy names interaction tests. “Generated,”
“builds,” and “qualified” are distinct statuses. No record implies a
certification unless an external certification process and scope are named.

## Generator behavior

System resolution checks evidence references against artifact and configuration
hashes. Missing evidence never blocks ordinary development generation unless
the project selects a policy requiring it; it does block that policy's
release/qualification claim. The system report lists each node, link, external
artifact and evidence status separately, including partial or stale evidence.
