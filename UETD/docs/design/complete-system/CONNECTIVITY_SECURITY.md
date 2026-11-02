# Connectivity, security and maintenance

First-pass scope includes usable provider-based networking and secure product
maintenance. Optional protocol/stack breadth is later scope. A design or facade
does not imply that a radio, controller firmware or stack is integrated.

## Provider layering

Keep the active UPAL `radio_transport`, `netif`, and `ethernet` boundaries, and
UPROTO `wifi`, `bluetooth`, and `ip` service APIs. Complete them through selected
providers instead of embedding vendor dependencies in portable callers.

| Layer | Owns | Must not imply |
|---|---|---|
| Physical/bus | Electrical pins, clocks, DMA, HCI/AT/SPI/UART transfer, MAC/PHY/radio/controller firmware | A usable IP or application protocol stack |
| Link service | WiFi association, BLE link/GATT, Ethernet link, radio framing and quality events | Internet reachability, authentication of application peers or durable delivery |
| Network | Addressing, routes, DHCP/DNS and bounded TCP/UDP socket operations | TLS identity or application message acceptance |
| Security session | Trust policy, key use, authenticated encrypted connection, peer verification | Application authorization or exactly-once action |
| Application protocol | MQTT/HTTP/Modbus/CANopen/custom wire profile and acknowledgements | Physical execution or permanent storage unless the profile says so |
| Product policy | Connection/recovery, commissioning, authority, safe actions and update activation | A provider-specific guarantee that was never declared |

A provider may collapse several layers through socket/application offload, but
must declare the resulting limits, trust boundary, error mapping and evidence.
The generator chooses one compatible route; it must not link two network
stacks accidentally or assume that an IP stack is needed for every BLE use.

Every provider pins its source/library revision, generated SDK configuration,
external firmware/blobs, license files, toolchain/runtime ABI and target
resources. Configuration, initialization, event pump, shutdown and recovery
are part of its contract. A provider that requires heap or a background task
declares them in the budget rather than hiding them in `bind()`.

## Network and connection manager

First-pass profiles support explicitly selected IPv4 and/or IPv6, TCP/UDP,
bounded DNS, static addressing and supported DHCP modes. An address-family
choice is a project/provider capability, not inferred from an address string.
DNS result count, hostname length, socket count, receive/transmit buffers,
backlog and in-flight operations are bounded.

Use generation-tagged socket/connection handles. State transitions distinguish
link down, address unavailable, resolving, connecting, secured, ready, closing
and failed. Partial TCP transfers are normal and carry exact byte counts;
datagrams preserve packet boundaries and reject or explicitly report truncation.
Deadline expiry/cancel closes or drains outstanding operations according to the
provider contract before caller buffers are released.

The connection manager owns retry/backoff, address reacquisition, optional
multi-interface selection and service readiness. Each reconnect has a maximum
attempt/work budget and jitter source. An outage invalidates dependent session
handles and product commands. Reconnection does not replay unsafe actuation
requests or reactivate outputs automatically.

Routing between interfaces and Internet gateway/NAT behavior are separately
selected capabilities. Basic first-pass interface management does not claim
full router or firewall functionality. Packet MTU, fragmentation/reassembly,
checksum offload, timestamp quality and maximum retransmission memory belong
to the network provider's profile.

Acceptance: cable/radio loss, address change, partial reads/writes, DNS failure,
full socket/pbuf pools, connection storm, malformed lengths, cancel/close races,
resource leakage and a sustained-load interoperability test.

## WiFi and Bluetooth

WiFi configuration identifies supported station/AP roles, authentication
profiles, credential handle, regulatory region, scan result bound, connection
timeout and coexistence/power policy. Scans and provisioning cannot overwrite
an active connection silently. AP mode specifies its own addressing and access
policy; it does not automatically expose the maintenance service.

Bluetooth first-pass provider contracts cover the selected LE roles and GATT
client/server subset. Bound service/characteristic counts, value lengths,
connection count, ATT MTU, notifications/indications and transaction queues.
Discovery results and server request/response ownership are explicit. A server
write request does not become accepted application configuration until its
validation/authorization completes. Bonding/security level and credential
storage belong to a security/provider policy.

Selected Classic SPP support remains provider-specific. A UETD-owned portable
LE host over standardized HCI is useful later stack ownership work, distinct
from first-pass integration of a qualified SDK/controller provider. WiFi has
no assumed universal HCI-equivalent. Controller firmware version and its
transport initialization are locked build artifacts.

Acceptance: authentication failure, wrong region/profile, bond reset,
disconnect during GATT transaction, MTU/payload mismatch, repeated scan events,
notification backpressure, concurrent roles and lost controller state.

## Framing, codecs and project protocols

First-pass common utilities include bounded binary cursor codecs, explicit
CRC/checksum selection, length/delimiter framing and selected CBOR/JSON adapters.
COBS/SLIP-like framing is an explicit profile rather than a wrapper imposed on
standard protocols. Define maximum encoded/decoded size, resynchronization
work per call, invalid escape/length policy, and partial-frame timeout.

JSON/CBOR profiles declare allowed types/depth/string/array/map sizes, numeric
range, duplicate-key/unknown-field behavior and canonicalization requirements
where signatures use them. Parsing never borrows input beyond its declared
lifetime. A general-purpose provider is accepted only with bounded allocation
or an explicit capped arena and failure tests.

Project protocol schemas are authored once. They declare message IDs, field
width/order/units, encoding, payload bounds, version evolution, sequence and
freshness rules, integrity/authentication, fragmentation/reassembly and retry
semantics. Both nodes receive generated codecs and test vectors from that
source. Source-language enums/records are not automatically C wire layouts.

Reception publishes a complete validated record. Unsupported message/version,
duplicate, stale, corrupt, unauthorized and incomplete are distinct outcomes.
A protocol defines whether retries preserve the original request ID, what an
acknowledgement acknowledges, and whether an action is idempotent. Do not claim
exactly-once execution from a reliable byte stream or an ACK alone; durable
deduplication/outcome storage is required where that property is requested.

## First-pass application protocols

| Protocol | Minimum bounded profile | Failure and validation gate |
|---|---|---|
| MQTT | Explicit 3.1.1 or 5 profile; topic/payload/count bounds, selected QoS, session policy, keepalive, retained-message policy and TLS/credential route | ACK/in-flight queues, reconnect/session recovery, duplicate delivery, broker interoperability and no implicit actuation from retained telemetry |
| HTTP client | Explicit supported version, methods, bounded headers/body, streaming/chunking subset, TLS, redirect host/credential policy and request deadline | Oversized/chunked response, short read, retry safety, invalid status/header and connection reuse |
| Modbus RTU/TCP | Client/server roles, selected function codes, typed register map, endian/word policy, writable authority, RTU serial/turnaround or TCP transaction limits | CRC/framing/timing, exception mapping, stale/atomic process image, concurrent requests and actual instrument interoperability |
| CANopen CC | Pinned stack/profile, bounded object dictionary, NMT/heartbeat, configured PDO/SDO subset, timing/COB-ID policy and fault reporting | Dictionary range checks, heartbeat loss, SDO timeout, PDO freshness/atomicity, bus-off and stack conformance/interoperability |
| USB device | CDC ACM and HID profiles, project-owned descriptors/identity, endpoint/RAM budget, enumeration and suspend/resume | Host interoperability, malformed setup requests, endpoint overflow, disconnected console and safe reenumeration |
| Time synchronization | Explicit SNTP/PTP/GNSS/provider subset; clock domain mapping, source/authentication policy, uncertainty, step/slew and holdover | Source change, asymmetric delay, missing hardware timestamps, backward wall-clock jump and stale synchronized data |
| Existing control links | Complete PPM plus bounded CRSF/SBUS/DSHOT/MAVLink/DroneCAN profiles required by selected products | Golden frames, ranges/CRC, timeout/failsafe, target timing/transport and physical peer tests |

MQTT, HTTP, TLS, Modbus and CANopen CC are first-pass contracts even if their
current implementation is an outline. A selected product can omit them; their
main-catalogue presence is independent of per-board support. Protocol feature
subsets must be explicit rather than promising every optional standard feature.

The process-image contract stages a complete scan, validates quality/range/
authority/generation, and publishes atomically. Reads observe one generation;
writes do not expose a half-updated control image. Interlocks/local safety
remain with the actuator owner.

## Security service boundary

Add an optional `usec` namespace for shared security provider contracts. It is
a first-pass dependency for security-requiring products, not a mandatory linked
library for an offline device. UPROTO TLS, commissioning and UAPP update reuse
these services rather than each inventing key storage or entropy rules.

| Service | Proposed contract |
|---|---|
| Entropy/DRBG | Provider initialization/health, entropy-ready status, bounded random-byte request, reseed policy and catastrophic-failure latch; simulation seed is never a production entropy source |
| Credentials/keys | Opaque handle, algorithm/use policy, identity, lifecycle/generation and storage provider; key material is non-exportable where the selected hardware supports it |
| Crypto operations | Selected vetted provider, algorithm/profile, input/output/scratch bounds, context, cancel behavior and exact error mapping; hash, MAC, signature verify/sign and AEAD only as requested |
| Trust policy | Trust anchors, peer/name validation, allowed algorithms, validity-time policy, revocation/update route and fail behavior when trusted time is unavailable |
| Authorization | Principal/role/capability plus operation/target/context; separate observation, configuration, actuation, enrollment, update and factory-reset permissions |
| Audit/redaction | Typed event IDs and metadata; secrets, tokens, plaintext keys and credential-bearing URLs are never ordinary log fields |

No new cryptographic primitive is implemented from scratch as a prerequisite
for the generator. Select established providers and qualify their build/config,
memory and operation subset. Hardware acceleration is an alternate provider
with the same output/security contract, not a reason to change algorithm or
silently use a weaker fallback.

Credential provisioning is `unenrolled -> enrollment_authorized -> receiving ->
verifying -> committed -> active`, with bounded timeout/cancel and erasure of
temporary secret buffers. Define factory identity, initial trust, physical
presence or authenticated authorization, rotation/revocation and ownership
transfer. A device serial number is not proof of possession of a secret.

Acceptance: unavailable entropy, invalid signature/tag, incorrect peer name,
expired/untrusted certificate, no trusted clock, secret-store failure, stale
key handle, rotation interrupted by power loss, privilege denial and redaction
of every failure path. Security assurance claims remain tied to the selected
provider and its configured use.

## TLS and authenticated sessions

TLS/DTLS adapters expose bounded session objects, explicit client/server roles,
supported versions/profiles, credential/trust handles, hostname/identity
verification, handshake deadline and receive/transmit limits. A transport
offload provider must report whether it validates the peer itself and what
trust/key material crosses the module boundary.

Session states distinguish transport connection, handshake, peer verified,
established, closing and failed. Application data cannot be admitted as
authenticated before peer verification. Nonblocking provider operations return
required readiness/work rather than spinning without a bound. Handshake
scratch, certificate chain depth/size, entropy calls and concurrent sessions
are included in static/capped memory planning.

A security failure does not downgrade to plaintext or disable peer checks
implicitly. Resumption and ticket/key persistence are separately selected and
bounded. Protocol adapters reuse the authenticated session and authorization
policy; encrypted transport alone does not authorize an actuator command.

## Boot, update and rollback

Authenticated boot/update, persistent crash history and configuration recovery
are first-pass product-maintenance contracts. Reuse the existing signed
artifact, trust-policy and update-progress schemas and the
[deployment update design](../../../../uef-gen/src/uef_gen/deploy/SECURE_UPDATE.md).

The exact target/bootloader profile declares image format, verification
algorithm/key access, partition geometry, trial/confirm/rollback, anti-rollback
counter and recovery behavior. Support for authenticated programming does not
automatically imply dual-slot power-fail rollback. A single-bank device must
either have a qualified recovery route or reject the stronger guarantee.

Update state is `admitted -> receiving -> staged -> integrity_verified ->
signature_verified -> compatibility_checked -> programmed -> trial -> confirmed`,
with explicit rejection, failure, cancellation and rollback branches. Reverify
the exact bytes/manifest before activation. Persist only a bounded progress
record that recovery can validate. Output inhibit and maintenance authorization
are part of admission, not a late transport decision.

The signed manifest binds exact hardware compatibility, bootloader/profile,
security version, images/roles/ranges/hashes and required peer-image
compatibility. Signing occurs after final segment hashes exist. Private signing
keys stay in an external authorized signer/HSM/key store and outside generated
firmware/source snapshots. Hashes used for reproducibility are not signatures.

Trial confirmation requires declared health conditions within a deadline.
Power loss during programming, activation or confirmation has a tested outcome.
Schema/data migration is compatible with rollback or explicitly makes rollback
unavailable under an approved product policy. Multi-image/node updates have a
compatibility and partial-failure plan, not an implied distributed transaction.

MCUboot's separation of boot utilities, target boot application and flash areas
is a useful provider reference; UETD still requires an exact compatible port
and layout. [MCUboot design](https://docs.mcuboot.com/design.html).

Acceptance: corrupt/unsigned/wrong-board image, downgrade, revoked key, out-of-
range segments, low power, interruption at every durable transition, failed
trial, incompatible peers, and recovery with retained diagnostics.

## Genuinely later connectivity scope

| Pack | Design boundary and additional obligations |
|---|---|
| PPP/cellular/modem | AT command/parser state, URC ownership, SIM/credential handles, registration/APN, PPP or socket offload, reconnect/power budget, modem firmware and operator/regional profile |
| LoRa/LoRaWAN/other LPWAN | Radio/regional plan, airtime/duty-cycle/payload constraints, network/session key lifecycle, downlink windows and certified/provider interoperability |
| Thread/Zigbee/mesh | Selected stack/profile, bounded routing/neighbor tables, commissioning/key lifecycle, radio resource coexistence and mesh-specific certification scope |
| CoAP/LwM2M | Selected role/RFC/object profile, bounded retransmission/blockwise/observe resources, DTLS/OSCORE provider and management authorization |
| CANopen FD, J1939, LIN, IEC60870 | Separate standard/profile and wire contract; they do not inherit implementation readiness from CANopen CC, CAN-FD or UART |
| MEDI CAN | Keep the existing separate proprietary design and durable execution/acknowledgement obligations. No generic safety or lossless-delivery claim is inferred |
| EtherCAT/PROFINET/OPC UA/TSN | Licensed/pinned stack and exact real-time hardware profile, schedule/clock/queue constraints, application mapping and interoperability evidence |
| Advanced USB | Host role, hubs, mass storage, audio/video and composite class combinations with explicit endpoint/bandwidth/resource budgets |
| UETD-owned protocol stacks | Portable BLE host or full packet-stack ownership only when justified; maintain the provider boundary so first-pass SDK adapters remain valid |

These packs use the same bounded provider and product-policy model. They are
kept in the later catalogue because their absence does not block the first-pass
acceptance products, rather than because their implementation is difficult.
