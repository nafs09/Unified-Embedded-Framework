# MEDI CAN: Myovena protocol extending CANopen FD

State: a separate future UPROTO protocol scaffold for Myovena medical-device
communication. MEDI CAN extends CANopen FD with its own protocol identity,
delivery state machine, integrity/freshness contract and execution semantics.
It is not merely a CANopen FD application-object allocation or validator hook.
The transport binding and wire specification still require design and review.

UETD owns `include/uproto/can/future/medi_can.h`,
`src/uproto/can/future/medi_can.c` and an independent `medi_can` module
descriptor. All eleven operations return `UETD_NOT_SUPPORTED` without changing
caller storage. The module remains `scaffold_only`, outside `uproto.h` and
`protocol_provides`. No delivery, execution or medical safety guarantee exists
in this scaffold.

## Relationship to CANopen FD

```text
Myovena device control and device-specific fault response
                         |
MEDI CAN protocol: sessions, acknowledged delivery, integrity, recovery
                         |
MEDI execution gate: exact command, freshness, deduplication, interlocks
                         |
Explicit extension binding to CANopen FD services and CAN FD transport
                         |
CANopen FD base stack / UPAL CAN FD controller and transceiver
```

Pin the CANopen FD revision/profile and specify which services MEDI reuses,
which it adds, its capability negotiation and the dispatch boundary. MEDI may
define extension messages and protocol fields under its own identity. Its wire
codec, opcodes, CAN-IDs, segmentation and mapping are not fixed by the current
C structs. A 256-byte logical payload may require multiple FD frames.

Keep inherited services conformant wherever standard CANopen FD compatibility
is claimed. A changed standard service must be identified as MEDI-specific,
with an explicit compatibility boundary. Unmodified nodes must not be assumed
to understand or satisfy MEDI delivery/execution guarantees. Protected command
paths require an established compatible MEDI session; no silent fallback to
ordinary PDO/USDO command admission is permitted.

Proprietary OD objects can expose MEDI capabilities, configuration and
diagnostics. They do not define the whole protocol or reserve CAN-IDs. USDO's
session-ID is transport state when USDO is used; MEDI owns its separate
end-to-end session/transaction identity. See the
[CANopen FD foundation and conformance boundary](CANOPEN_FD_MEDI_CAN.md).

## Reliability objective: no silent loss

Translate the intended "never drop packets" behavior into a testable contract:
**every admitted logical message remains tracked until a confirmed outcome or
an explicit fault/unknown outcome; no queue or timeout silently discards it.**
Physical disconnection, persistent bus faults, power loss and finite storage
prevent an unconditional delivery guarantee. The design must preserve evidence
and invoke the defined device fault response when delivery cannot complete.
These are proposed requirements, not implemented behavior.

| Requirement | MEDI protocol responsibility |
|---|---|
| Admission and backpressure | Reserve bounded transaction/reassembly/result capacity before accepting work; report busy/full before admission, never overwrite admitted work |
| End-to-end receipt | Address the intended peer; confirm complete logical-message receipt, independent of a CAN frame ACK or an underlying transport response |
| Loss and ordering | Session-bound sequence numbers, bounded reassembly, gap detection, missing-fragment recovery and explicit ordering rules per stream |
| Retransmission | Retry the same identity/content with bounded attempts, backoff, lifetime and bus-load budget; never retry an expired command as fresh |
| Execution acknowledgement | Distinguish receipt, validation, execution in progress, completed execution, rejection and outcome unknown |
| Duplicate handling | Match source/destination, epoch and transaction ID plus exact command binding; return retained progress/result rather than repeat the physical action |
| Integrity and origin | Protect canonical header, endpoint/session context, sequence, operation, freshness evidence and payload end to end; choose integrity/authentication measures from the fault/threat model |
| Recovery | Retain required replay/outcome evidence across reset and reconcile before admitting affected commands |
| Fault reporting | Latch resource/link/validation faults independently of a possibly full event queue; inhibit new execution and invoke the device-owned response |

Configuration must bound maximum peers, inflight commands, payload/reassembly,
ACK/delivery/validation/execution deadlines, attempts, result retention and
work per poll. ACK/result traffic also needs reserved capacity and bandwidth
so control traffic cannot deadlock behind data. Counters must saturate or
report overflow rather than hide evidence by wrapping silently.

The initial planning budgets are four peers, eight tracked transactions,
sixteen retained results, sixteen events and 256 payload bytes per command.
They are not reviewed Myovena operating limits. RAM result storage is not a
durable journal. Define result eviction and epoch retirement so an old
duplicate cannot become a new command after its cache entry expires.

## Safe execution contract

Use distinct semantic progress states:

```text
admitted -> received -> validated -> executing -> executed
                |           |           |
             rejected    rejected    outcome unknown
```

Delivery failure is terminal only when non-execution is established. A lost
execution result or a restart between physical actuation and recording its
outcome creates ambiguity: mark `OUTCOME_UNKNOWN`, preserve identity/evidence
and reconcile. A missing ACK is not proof of non-execution. Never allocate a
fresh transaction ID and blindly repeat an unresolved non-idempotent command.

The trusted execution boundary must:

1. Stage the complete immutable command and validate version, length, source,
   destination, operation, authorization and canonical integrity metadata.
2. Validate session epoch, sequence/replay rules and freshness evidence. A
   receive timestamp bounds local queue residence only. Sender-to-execution
   age requires authenticated timing with bounded clock error or a
   receiver-issued expiring lease/challenge. Check expiry at execution as well
   as reception; include reassembly, retries and validation time.
3. Check deduplication, execution authority and device interlocks against the
   exact command. Bind any hardware verdict to the command digest, identity,
   policy generation and expiry; prevent buffer substitution and direct-output
   bypass. A boolean "valid" result is insufficient.
4. Coordinate persistent intent/result records with the physical action. An
   exactly-once effect cannot be inferred from retry logic or a RAM cache.
   Document unavoidable crash windows and recovery evidence. If the target
   cannot establish whether an action occurred, inhibit that action pending
   reconciliation instead of claiming an executed/failed result.
5. Retain and retransmit the authenticated execution outcome until the reviewed
   acknowledgement/retention rule permits retirement. Separate successful
   receipt from actual execution and from device-level clinical effectiveness.

`uproto_medi_can_execute_ready` is a future privileged execution-gate boundary;
`uproto_medi_can_record_result` is a future trusted executor report boundary.
Neither currently authorizes an action. Provider identity, callback ownership,
atomic commit and journal APIs must be designed before implementation.
`reset` must retain unresolved outcomes and replay protection, then establish a
new permitted session or reconcile. It must not simply clear all state.

Select a device-specific response for stale commands, validator timeout,
unavailable hardware, clock faults, bus-off, queue saturation, loss of journal
integrity and unresolved execution. The protocol inhibits unsafe new command
admission; Myovena's reviewed risk controls determine the physical response.
A generic transport cannot assume stopping every output is always safe.

## Verification and promotion gates

Freeze the fault model, threat model, residual error/detection targets, timing
budget, wire format, provider boundaries, device fault responses and recovery
contract before implementing or claiming robustness. Trace requirements to
state transitions and evidence on the exact hardware and software versions.

Inject deletion, corruption, repetition, insertion, misaddressing, reordering,
partial frames, delayed/stale traffic and lost receipt/execution ACKs. Exercise
arbitration starvation, retry storms, full queues, result-cache exhaustion,
bus-off/disconnect, counter/epoch rollover, clock faults, resets at every
execution/journal boundary, validator failure and output bypass attempts.
Demonstrate deadline detection, explicit outcomes and prevention of duplicate
or stale execution under the declared assumptions. Test baseline CANopen FD
conformance and MEDI compatibility/behavior separately.

For design review, the
[IEC 61784-3 black-channel principles](https://webstore.iec.ch/en/publication/92494)
are a reference for end-to-end safety communication over an underlying network;
their applicability to Myovena must be assessed. They do not make this new
protocol certified. [CiA identifies CANopen Safety separately](https://www.can-cia.org/cia-groups/cia-glossary-of-terms)
from CANopen FD; do not label MEDI CAN as an existing CANopen Safety profile.
The medical-device safety case, quantitative analysis, independent review and
product qualification remain separate from scaffold compilation and ordinary
[CANopen FD conformance testing](https://www.can-cia.org/services/cia-test-center/canopen-fd-device-conformance-testing).
