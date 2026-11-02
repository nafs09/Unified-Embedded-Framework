# Industrial UPROTO profiles: Modbus, CANopen CC and CANopen FD

**State:** first-pass Modbus/CANopen CC contracts with generation blocked. CANopen FD and MEDI CAN retain their separate later-scope records.
These profiles are not exposed in `protocol_provides` and cannot be selected
for generated firmware. `industrial_contracts.h` sketches caller-owned bounded
state/configuration types and typed feed/event operations; all tables referenced
by a live instance must remain immutable and alive for that instance's lifetime.
`can/future/canopen_fd.h` adds a separate FD outline using the shared OD/NMT
semantic types and the UPAL CAN FD frame contract.

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

This section covers CANopen CC (CiA 301) over classical CAN using `upal/can`.
The separate CANopen FD adapter uses `upal/fdcan_fd` and USDO instead of SDO.

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

## CANopen FD

`can/future/canopen_fd.h` and its fail-closed source outline cover a separate
CiA 1301 adapter. Initial storage budgets are 128 tracked-node slots, eight
events, four pending USDO transfers, 256 bytes per complete USDO transfer and
64 bytes of PDO application data. These are UETD budgets, not CiA limits.
Frame ingress retains FD, identifier format, BRS, ESI and receive timestamp.
Standard USDO session/segmentation state and provider TX binding remain to be
specified. All eight operations return `UETD_NOT_SUPPORTED` without changing
caller storage. The module is not exposed through `uproto.h`.

Freeze specification revision/profile, object access and mapping, FD padding,
NMT/error control, optional services, USDO deadlines/aborts and XDD/XDC
provenance before implementation. The existing Phase 0 CAN FD peripheral
outline does not make the protocol or hardware usable.

## MEDI CAN

Myovena's MEDI CAN is a separate protocol extending CANopen FD, with its own
typed scaffold at `can/future/medi_can.h`, source, module descriptor and future
catalogue entry. It owns planned acknowledged delivery, bounded retry/deadline
handling, integrity/freshness, duplicate suppression, recovery and trusted
execution semantics. Its eleven operations currently fail closed.

The [MEDI CAN protocol design](../../docs/design/can/MEDI_CAN.md) defines the
no-silent-loss objective and separates receipt, validation, execution and
unknown outcomes. Wire binding, opcode/CAN-ID allocation and provider behavior
remain unresolved. Proprietary OD objects may expose configuration and
diagnostics; they are not the protocol's entire definition. Inherited standard
services retain their own conformance boundary.

## Shared PLC image boundary

The industrial adapters bind to the first-pass UMID PLC process-image contract. That model
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
