# UPROTO roadmap and coverage review

This file maps Part XV.3 planned protocols to UEF-owned module outlines. It is
not a protocol implementation or selection list. The modules are registered
in `registry/modules/registry.json` as `scaffold_only`, their sources are excluded from
ordinary CMake builds, and no UPROTO future entry is present in
`protocol_provides` or the `registry/control_ir/registry.json` index and its per-node manifests.

| Proposal | Function outline | Contract decisions before implementation |
|---|---|---|
| MSP (MultiWii) | profile validation, init, bounded byte feed, request encode, response extraction, reset | Choose MSP v1/v2, checksum rules, payload limits, command registry, transaction matching/timeouts. |
| ExpressLRS | profile validation, CRSF binding, link-state processing/snapshot, reset | Decide whether this is a CRSF profile plus link metadata or a separate protocol module; define hardware-specific receiver/telemetry fields and freshness. |
| UAVCAN v1 / Cyphal | init, bounded spin, publish, subscribe, reset | Select the compatible stack and DSDL generator, protocol naming/version, node-ID policy, fixed memory and CAN-FD callback contract. Keep separate from UAVCAN v0/DroneCAN. |
| LIN | schedule validation, init, header scheduling, response processing, reset | Define master/slave role, break generation/detection, schedule table, protected IDs, classic/enhanced checksum, timing, and UART capability requirements. |
| J1939 | init, frame process, address claim, PGN send/receive, reset | Define 29-bit CAN support, NAME/address policy, PGN registry, BAM and RTS/CTS transport, retry/timeout bounds. |
| IEC 60870-5 | profile validation, init, bounded process, ASDU send, event retrieval, reset | Select IEC 101 or 104 first; define link framing, ASDU types, address widths, time tagging, command select/execute, and security boundaries. |

## Additional coverage candidates

The current spec also mentions MODBUS as a companion to IEC 60870-5 without a
separate UPROTO entry. Modbus RTU and CANopen are common embedded/industrial
protocol candidates worth considering for a later specification revision.
They are not added to the code catalogue because there is no approved profile,
transport boundary, or message/state contract yet. Protocols that depend on
large external stacks should remain explicit consumer dependencies, with UEF
owning only the adapter and bounded transport hooks.

## Promotion gate

For each protocol, define a typed fixed-size configuration and instance state,
framing and checksum rules, inputs/outputs, ownership/lifetime, parser recovery,
maximum work per call, memory limits, status/freshness behavior, transport
requirements, external stack version/license, and manual target actions. Then
implement the direct UEF C API, add protocol-specific templates only when
ControlIR contains the structural choices, and add `protocol_provides` plus a
shared-manifest entry only after those outputs match the reviewed API.
