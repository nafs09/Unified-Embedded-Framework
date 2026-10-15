# UPROTO roadmap and coverage review

This file maps Part XV.3 planned protocols to UETD-owned module outlines. It is
not a protocol implementation or selection list. The modules are registered
in `registry/modules/registry.json` as `scaffold_only`, their sources are excluded
from ordinary CMake builds, and no UPROTO future entry is present in
`protocol_provides`.

| Proposal | Function outline | Contract decisions before implementation |
|---|---|---|
| MSP (MultiWii) | profile validation, init, bounded byte feed, request encode, response extraction, reset | Choose MSP v1/v2, checksum rules, payload limits, command registry, transaction matching/timeouts. |
| ExpressLRS | profile validation, CRSF binding, link-state processing/snapshot, reset | Decide whether this is a CRSF profile plus link metadata or a separate protocol module; define hardware-specific receiver/telemetry fields and freshness. |
| UAVCAN v1 / Cyphal | init, bounded spin, publish, subscribe, reset | Select the compatible stack and DSDL generator, protocol naming/version, node-ID policy, fixed memory and CAN-FD callback contract. Keep separate from UAVCAN v0/DroneCAN. |
| LIN | schedule validation, init, header scheduling, response processing, reset | Define master/slave role, break generation/detection, schedule table, protected IDs, classic/enhanced checksum, timing, and UART capability requirements. |
| J1939 | init, frame process, address claim, PGN send/receive, reset | Define 29-bit CAN support, NAME/address policy, PGN registry, BAM and RTS/CTS transport, retry/timeout bounds. |
| IEC 60870-5 | profile validation, init, bounded process, ASDU send, event retrieval, reset | Select IEC 101 or 104 first; define link framing, ASDU types, address widths, time tagging, command select/execute, and security boundaries. |

## Additional coverage candidates

### Industrial protocols

The current spec mentions MODBUS as a companion to IEC 60870-5 but has no
separate UPROTO entry. The future catalogue now tracks three explicit
industrial candidates without advertising them through `protocol_provides`:

| Candidate | Required design decisions |
|---|---|
| Modbus RTU | Client/server role, supported function-code subset, address/register map, max frame/transaction count, RTU silent interval and inter-byte timeout, CRC behavior, retries, serial framing/half-duplex DE contract, and per-call work/memory bounds. |
| Modbus TCP | Client/server role, MBAP transaction-ID lifecycle, max concurrent requests, connection/reconnect policy, frame/timeout bounds, supported function subset, and mapping to the common PLC process image. It depends on a future Ethernet/network transport or a consumer network stack adapter. |
| CANopen | Exact device/profile subset, node-ID/configuration policy, object-dictionary source and size, PDO/SDO limits, NMT lifecycle, heartbeat/boot-up, EMCY handling, SYNC timing, EDS/DCF artifact identity, and external stack/license boundary. |

The future protocol contracts are linked from
`registry/future/registry.json`. Protocol modules remain scaffold-only until
these decisions become typed public records and the `umid/future` process-image
contract can bind them without duplicating PLC scan semantics. Existing
Modbus-compatible serial devices may use the same backend as RTU only after the
timing/framing details are explicit; TCP must not be inferred from an Ethernet
MAC contract. Protocols that require large vendor stacks remain explicit
consumer dependencies, with UETD owning only the adapter and bounded transport
hooks.

## Promotion gate

For each protocol, define a typed fixed-size configuration and instance state,
framing and checksum rules, inputs/outputs, ownership/lifetime, parser recovery,
maximum work per call, memory limits, status/freshness behavior, transport
requirements, external stack version/license, and manual target actions. Then
implement the direct UETD C API and add protocol-specific modules only after
their outputs match the reviewed API and target contract.
