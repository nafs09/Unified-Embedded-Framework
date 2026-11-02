# UPROTO roadmap and coverage review

This file maps planned protocols and connectivity services to UETD module
outlines. It is not a protocol implementation or target selection list. Future
module records are registered as `scaffold_only`; their sources are excluded
from ordinary CMake builds and they are not advertised through
`protocol_provides`. WiFi, Bluetooth, IP, external-radio transport, packet
netif, and Ethernet have active portable APIs and validation dispatch, but
their target providers and network/radio engines still require selection and
qualification.

| Proposal | Function outline | Contract decisions before implementation |
|---|---|---|
| MSP (MultiWii) | profile validation, init, bounded byte feed, request encode, response extraction, reset | Choose MSP v1/v2, checksum rules, payload limits, command registry, transaction matching/timeouts. |
| ExpressLRS | profile validation, CRSF binding, link-state processing/snapshot, reset | Decide whether this is a CRSF profile plus link metadata or a separate protocol module; define hardware-specific receiver/telemetry fields and freshness. |
| UAVCAN v1 / Cyphal | init, bounded spin, publish, subscribe, reset | Select the compatible stack and DSDL generator, protocol naming/version, node-ID policy, fixed memory and CAN-FD callback contract. Keep separate from UAVCAN v0/DroneCAN. |
| LIN | schedule validation, init, header scheduling, response processing, reset | Define master/slave role, break generation/detection, schedule table, protected IDs, classic/enhanced checksum, timing, and UART capability requirements. |
| J1939 | init, frame process, address claim, PGN send/receive, reset | Define 29-bit CAN support, NAME/address policy, PGN registry, BAM and RTS/CTS transport, retry/timeout bounds. |
| IEC 60870-5 | profile validation, init, bounded process, ASDU send, event retrieval, reset | Select IEC 101 or 104 first; define link framing, ASDU types, address widths, time tagging, command select/execute, and security boundaries. |

## Wireless and IP connectivity

`include/uproto/CONNECTIVITY.md` defines the provider split and the active
typed APIs and future outlines:

| Scaffold | Coverage | Provider still required |
|---|---|---|
| WiFi | Scan, station, AP, link state | External module provider over SPI/I2C/UART or integrated target SDK; regulatory and credential policy |
| Bluetooth | LE central/peripheral, GATT client/server, security hooks, Classic SPP | HCI/module provider or integrated stack, profile-specific target capability and coexistence evidence |
| IP | Address setup, DHCP, SLAAC hook, DNS, TCP/UDP sockets | One selected UETD-owned, target-native, or socket-offload stack with explicit memory/timeout bounds |
| PPP | PPPoS link state, MRU, IPv4/IPv6 negotiation request, opaque PAP/CHAP credential handle | Selected PPPoS/PPP stack and serial/modem or native SDK binding; PPP framing and negotiation stay provider-owned |
| TLS/DTLS | Opaque session and credential handles | Consumer/provider crypto library, entropy, trust store and secure key lifecycle |
| MQTT, CoAP, HTTP | Typed client request/publish and bounded output | Selected protocol implementation, retransmission/ACK policy, TLS as required |
| Thread/Zigbee | Commissioning/state and Zigbee endpoint/cluster command boundary | IEEE 802.15.4 radio plus selected external/vendor stack and key policy |

An external radio module uses `include/upal/network/radio_transport.h` for its
module-specific transaction or asynchronous byte stream over the selected bus. Integrated chips bind
directly to UPROTO through their SDK adapter. Raw 802.15.4 MAC frames use
`include/upal/network/ieee802154.h`. Packet-mode Ethernet/WiFi links and IP
stacks over 6LoWPAN use `include/upal/network/netif.h`; socket-offload-only modules bind
to the UPROTO IP/application adapter directly. No path infers an available
stack from an ESP32 chip capability string. Current provider contracts do not
make ESP-IDF selectable: `UETD_TARGET` currently implements HOST and CORTEX_M;
an ESP32 profile still needs a target backend and provider adapter.

## Additional coverage candidates

### Industrial protocols

The current spec mentions MODBUS as a companion to IEC 60870-5. Modbus RTU/TCP,
CANopen CC and CANopen FD have separate typed UPROTO headers and fail-closed source
definitions, registered without `protocol_provides` entries:

| Candidate | Required design decisions |
|---|---|
| Modbus RTU | Client/server role, supported function-code subset, address/register map, max frame/transaction count, RTU silent interval and inter-byte timeout, CRC behavior, retries, serial framing/half-duplex DE contract, and per-call work/memory bounds. |
| Modbus TCP | Client/server role, MBAP transaction-ID lifecycle, max concurrent requests, connection/reconnect policy, frame/timeout bounds, supported function subset, and mapping to the common PLC process image. It depends on the active UPAL Ethernet/netif and UPROTO IP contracts plus a selected network provider. |
| CANopen CC (CiA 301) | Exact device/profile subset, node-ID/configuration policy, object-dictionary source and size, PDO/SDO limits, NMT lifecycle, heartbeat/boot-up, EMCY handling, SYNC/TIME timing, EDS/DCF artifact identity, and external stack/license boundary. |
| CAN FD transport | Reuse the Phase 1 `upal/fdcan_fd` frame API; implement nominal/data timing, legal DLC lengths, message RAM, filters, queues, timestamps and bus-off recovery on the exact target. A transport does not supply CANopen FD. |
| CANopen FD (CiA 1301) | Separate `industrial/canopen_fd.h` outline with bounded USDO/PDO events over `upal/fdcan_fd`; select specification revision, device profile, standard session behavior, NMT/error control, mapping/padding, XDD/XDC artifacts and provider binding. |
| Myovena MEDI CAN protocol | Independent `can/future/medi_can.h` scaffold extending CANopen FD. Define the extension wire binding, acknowledged delivery, bounded retries/deadlines, integrity/freshness, duplicate suppression, durable recovery and trusted execution gate. No silent loss of admitted commands; unresolved outcomes require explicit fault/reconciliation. |

The future protocol contracts are linked from
`registry/future/registry.json`. Modbus, CANopen CC and CANopen FD have typed headers and
fail-closed C definitions; they remain scaffold-only until their wire behavior
and the `umid/future` process-image contract can bind them without duplicating
PLC scan semantics. Existing
Modbus-compatible serial devices may use the same backend as RTU only after the
timing/framing details are explicit; TCP must not be inferred from an Ethernet
MAC contract. Protocols that require large vendor stacks remain explicit
consumer dependencies, with UETD owning only the adapter and bounded transport
hooks.

The [CANopen FD foundation](../design/can/CANOPEN_FD_MEDI_CAN.md) specifies the
transport and standard-conformance boundary. The independent
[MEDI CAN protocol design](../design/can/MEDI_CAN.md) defines Myovena's reliability,
acknowledgement, replay/freshness, recovery and execution requirements. Its
typed operations remain fail-closed; the wire codec and guarantees are not implemented.

## Promotion gate

For each scaffold, select or implement its provider, define fixed memory/work
budgets, status/freshness behavior, task/ISR ownership, external stack
version/license, target capabilities and manual actions, then add a generated
route only after outputs match the reviewed API and exact target contract.
