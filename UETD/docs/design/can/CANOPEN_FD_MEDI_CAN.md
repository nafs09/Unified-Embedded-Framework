# CANopen, CAN FD, CANopen FD and MEDI CAN

State: future implementation plan. UETD has fail-closed CAN/CAN FD peripheral
outlines and separate CANopen CC and CANopen FD protocol outlines. MEDI CAN
is Myovena's separate protocol extending CANopen FD, with its own scaffold.
Its wire schema, reliable-delivery behavior and execution providers remain
unimplemented; see [MEDI CAN](MEDI_CAN.md).

## Protocol ownership

| Capability | UETD boundary | Intended standard and behavior |
|---|---|---|
| Classical CAN | `upal/can` | CAN frame transport; no object dictionary or CANopen services implied |
| CANopen CC | `uproto/canopen` | CiA 301; NMT/boot-up/heartbeat, PDO, SDO, SYNC/TIME and EMCY as required by the selected profile |
| CAN FD | `upal/fdcan_fd` | ISO CAN FD frame transport, up to 64 data bytes, explicit FD/BRS/ESI and identifier format |
| CANopen FD | `uproto/future/industrial/canopen_fd` | CiA 1301; FD communication, USDO, PDO, NMT/error control, SYNC/TIME and EMCY according to the selected revision/profile |
| MEDI CAN | `uproto/future/medi_can` | Myovena protocol extending CANopen FD; independent delivery, integrity/freshness, recovery and execution contract |

CANopen FD uses USDO rather than classic SDO and has FD-specific communication
rules. Do not implement it by enlarging the classic CANopen buffers. Its
protocols use FD frames, including short messages; BRS need not be enabled.
See [CiA's CANopen FD overview](https://www.can-cia.org/can-knowledge/canopen-fd-the-art-of-embedded-networking).

## CAN FD transport

Reuse `include/upal/can/fdcan_fd.h` and `src/upal/can/fdcan_fd.c`; another CAN FD driver
identity is unnecessary. Before hardware use, implement exact controller and
transceiver bindings, nominal/data bit timing, legal DLC lengths/padding,
message RAM, filters, queue capacities, IRQ ownership, receive timestamps,
bus-off recovery and bounded transmit completion reporting. ESI is a bus error
state indicator, not proof that application data passed MEDI validation.

Review every node on a shared segment before transmitting FD frames. An
FD-intolerant classic controller can generate error frames on FD traffic;
FD-tolerant does not mean it can decode FD. Separate buses with a gateway when
required. Application compatibility also requires a protocol-aware gateway;
classical CANopen peers cannot interpret CANopen FD USDO merely because
arbitration rates match. See [CiA's CAN terminology](https://www.can-cia.org/fileadmin/cia/documents/brochures/can_dictionary_v13_2026.pdf).

## CANopen FD implementation gates

1. Pin the CiA 1301 revision and device/application profile, stack version and
   license, node-ID/LSS policy and FD transport provider. CiA currently lists
   [CiA 1301 v1.1.0](https://www.can-cia.org/cia-groups/technical-documents).
   Define the provider transmit binding, callback ownership and maximum work
   per poll before implementing the adapter.
2. Implement required object types, access rights, NMT transitions, boot-up,
   heartbeat, USDO sessions/aborts/timeouts, PDO mappings and supported
   SYNC/TIME/EMCY services from that revision. Treat local enum values as
   semantic state, not wire encodings. Keep CANopen CC SDO state separate.
3. Apply bounded storage explicitly. The initial FD outline reserves 128 node
   slots (not 128 valid node IDs), eight events, four pending USDO transfers
   and 256 bytes per transfer. PDO application data is capped at 64 bytes.
   These are UETD budgets, not CiA limits. Reject unsupported sizes/modes
   explicitly; define streaming or another reviewed profile for larger USDOs.
   Specify timeout, segmentation, queue-loss, process-image quality and reset
   behavior. FD [PDO padding/mapping rules](https://www.can-cia.org/can-knowledge/pdo-protocol-1)
   must be handled without treating padding as extra mapped command fields.
4. Produce a CiA 1311-1 XDD device description and XDC configuration artifacts
   with version/hash provenance. Do not assume the classical EDS/DCF path is
   sufficient for FD. See [CiA 1311](https://www.can-cia.org/can-knowledge/cia-1311-canopen-fd-electronic-device-description).
5. Test the baseline against CiA 1310-1 and the selected device profile, then
   test MEDI independently and in combination. The
   [CiA conformance assessment](https://www.can-cia.org/services/cia-test-center/canopen-fd-device-conformance-testing)
   checks the device against CiA 1301 and its XDD; it does not establish MEDI
   security, command correctness or functional-safety certification.

## MEDI CAN protocol extension

Myovena's MEDI CAN has its own protocol scaffold, versioning, transactions,
acknowledgements, reliability state and execution contract. The
[MEDI CAN design](MEDI_CAN.md) is the authoritative extension plan. It defines
no silent loss of admitted logical messages, bounded retransmission/deadlines,
duplicate suppression, integrity/freshness, durable recovery and a trusted
execution gate. It distinguishes receipt, validation, execution and unknown
outcomes; it does not assert unconditional delivery under physical faults.

Specify which CANopen FD services MEDI inherits and which extension services
it defines. Preserve baseline behavior wherever standard CANopen FD
compatibility is claimed. Changes to standard service formats or semantics
belong to the MEDI protocol and require an explicit compatibility boundary;
unmodified CANopen FD nodes cannot be assumed to provide MEDI guarantees.

Proprietary application objects in `0x2000`-`0x5FFF` can expose extension
configuration and diagnostics. They do not reserve CAN-IDs or define MEDI's
entire protocol. USDO session matching, when used, remains distinct from
MEDI's end-to-end transaction identity. See
[CiA's CANopen FD dictionary description](https://www.can-cia.org/can-knowledge/canopen-fd-the-art-of-embedded-networking)
and [USDO description](https://www.can-cia.org/can-knowledge/usdo-protocol).

Freeze extension framing/dispatch, CAN-ID allocation, canonical encoding,
segmentation and bus-load bounds before implementing the codec. Test
inherited CANopen FD services for conformance and MEDI protocol behavior
against its separate fault, timing and execution requirements. Neither a
proprietary OD entry nor a base-stack conformance result establishes medical
safety or reliable execution of MEDI commands.
