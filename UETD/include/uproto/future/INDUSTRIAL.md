# Industrial UPROTO profiles: Modbus and CANopen

**State:** design-only protocol outlines, indexed by the UETD future catalogue.
These profiles are not exposed in `protocol_provides` and cannot be selected
for generated firmware. `industrial_contracts.h` sketches caller-owned bounded
state/configuration types and typed feed/event operations; all tables referenced
by a live instance must remain immutable and alive for that instance's lifetime.

## Modbus RTU

Freeze a supported function-code subset and client/server or mixed-role
behavior. The project contract supplies unit ID, address/register map, permitted
read/write ranges, maximum PDU/ADU size, request rate and outstanding
transaction bound. The implementation supplies CRC16, exception responses,
malformed-length behavior and bounded parser work.

The current proposed outline bounds each RX ADU at 260 bytes, parsed events at
253 bytes, queued events at eight, and outstanding transactions at eight.
Profile validation must enforce those compiled capacities, reject illegal
function codes/ranges and report event-queue loss. RTU gap timing comes from
the configured character format; TCP feeds may split any MBAP/PDU field across
arbitrary input chunks. Configuration maps are immutable for the lifetime of
an instance.

RTU framing requires explicit serial data bits/parity/stop bits, silent-interval
calculation from the active character format and baud, inter-byte timeout,
turnaround time, retry count and RS-485 DE control. A timeout or truncated
frame cannot leave a half-written process-image scan marked good. Server
register updates commit atomically at a declared boundary.

## Modbus TCP

Use the MBAP transaction identifier and protocol identifier explicitly. Bound
connection count, outstanding request table, PDU/ADU size, request lifetime,
socket buffers and retry/reconnect work. Define whether a server is single or
multi-client and how duplicate IDs, out-of-order replies, disconnects and
partial TCP segments are handled. TCP stream segmentation is not frame
segmentation; parser state must tolerate arbitrary chunk boundaries.

Modbus TCP depends on a selected network stack/transport adapter and optional
UPAL Ethernet MAC. A MAC contract alone does not supply IP, sockets, DHCP,
addressing or cybersecurity behavior.

## CANopen

Select a device/profile subset before defining the API. Freeze node-ID and
network-management ownership, boot-up/NMT transitions, heartbeat producer and
consumer timing, SYNC cadence, PDO mapping and event policy, bounded SDO
segmentation/retry, EMCY publication, and emergency/timeout behavior. Object
dictionary entries have fixed types, access rights, units, defaults and
nonvolatile policy. EDS/DCF source/version/hash belongs in artifact provenance.

An external CANopen stack is an explicit versioned dependency when used; UETD's
adapter owns bounded buffers, callbacks, status mapping and process-image
binding, not a duplicate upstream stack. CANopen does not make the PLC's
process-image scan atomic unless the selected mapping adapter commits it as one
generation.

The type-level outline currently caps local node tracking at 128, queued events
at 32, concurrent SDO transfers at four and the per-transfer buffer at 256
bytes. Validation rejects larger configured limits until caller-owned storage
or a different bounded profile is designed. Heartbeat expiry, NMT changes,
SDO aborts, EMCY and PDO quality changes are explicit events; queue overflow
must be counted and must mark affected process points stale or bad.

## Shared PLC image boundary

Both protocols bind to the future UMID PLC process-image contract. That model
owns point direction, data quality, freshness, atomic scan generation,
write authority, safe value and local interlock ownership. Protocol adapters
may not weaken those rules or turn a stale remote command into a valid output.

## Promotion gate

Before adding a module descriptor or `protocol_provides` entry, specify typed
fixed-size config/state, transport dependencies, exact function/profile set,
maximum memory and work, all timers/retries, failure/freshness semantics,
external-stack license/version if applicable, target support, and test vectors.
Then implement parser/client/server behavior and UETD-owned generated
configuration from the same reviewed module contract.
