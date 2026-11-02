# UPROTO connectivity contracts

**State:** active, buildable provider contracts for WiFi, Bluetooth, and IP;
TLS/DTLS, MQTT and HTTP are first-pass provider contracts with generation blocked; CoAP, PPP and Thread/Zigbee remain later modules. These
contracts do not include vendor radio drivers, a network stack, or
cryptography. An unbound provider is not a usable link, and unsupported
operations return `UETD_NOT_SUPPORTED`.

## Provider and transport split

The radio and network implementation is selected per target/profile:

1. An external radio/network coprocessor binds its module-specific command
   provider to `upal/radio_transport`. That UPAL boundary supports
   bounded command/response exchanges and asynchronous byte streams over an
   already configured SPI, I2C, or UART resource. The provider owns AT/HCI/
   vendor framing, wake/ready timing, unsolicited event parsing, and reset
   sequencing.
2. An integrated radio or network stack binds directly to the corresponding
   UPROTO operations through its SDK adapter. ESP-IDF is an example of this
   provider shape; it does not need a fake SPI/UART transport.
3. Packet-mode Ethernet/WiFi links and IPv6 stacks over 6LoWPAN may expose a
   packet interface through `upal/netif`. A module that exposes only IP sockets or HTTP/MQTT
   offload binds those UPROTO providers directly and does not claim raw packet
   access.

Raw IEEE 802.15.4 MAC frames use `upal/future/ieee802154`; its fixed frame
buffer excludes the FCS, which the provider generates or verifies. Thread and
Zigbee consume a selected MAC/radio provider rather than duplicating the radio
driver.

The UETD facade owns fixed-size public records, argument checks, capability
gates, and status dispatch. Provider-owned memory, task/ISR context, queue
capacity, timing, retry behavior, security properties, and implementation
version must be recorded in the selected project profile. Callback tables and
their contexts must outlive their bound UETD handle; providers must not retain
borrowed call buffers unless their contract explicitly copies them into bounded
storage.

## WiFi

The first configuration profile covers station and access-point roles, scan,
link state, WPA2-PSK/WPA3-SAE security labels, a selected channel, and optional
IPv6 request. Enterprise authentication, regulatory database ownership, mesh
WiFi, roaming policy, and power-save behavior require explicit provider
extensions and must not be inferred. Credentials are supplied by the caller,
are not logged, and are not persisted by the UETD facade.

`uproto_wifi_bind` accepts a native SDK adapter or a provider that drives an
external module through the UPAL host transport. IP configuration and socket
service are a separate `uproto_ip` service. Only a module advertising socket
offload should bypass the raw packet `upal_netif` path.

## Bluetooth

The active API covers LE central/peripheral operations, scanning,
advertising, asynchronous GATT service/characteristic/descriptor discovery,
client reads/writes/subscriptions, and server service registration, request
responses, notifications, and indications. Registration returns the service
handle and each characteristic value handle, and services can be unregistered.
Pairing/security hooks and the Classic Serial Port Profile (SPP) are included;
other Bluetooth Classic profiles are not claimed. UUIDs, characteristic
properties, maximum value sizes, security level, bond policy, transaction IDs,
and event payload capacity are explicit. The host stack owns ATT/GATT protocol
state and BLE controller scheduling.

Classic SPP and BLE have different controller/profile requirements. A provider
advertises only the capabilities it actually supports. In particular,
`BLUETOOTH` in a chip record must not be treated as proof of Classic SPP, GATT
roles, security mode, or coexistence behavior.

## IP network services

The active IP API covers static interface configuration, DHCPv4 hooks, IPv6
autoconfiguration/SLAAC, DNS resolution, and caller-owned TCP/UDP socket
handles with bounded send/receive buffers. Ethernet needs this layer (or a
socket-offload provider) above the UPAL MAC/netif. The current UETD code owns
the portable API and validation; packet processing, TCP/IP, DHCP/DNS, socket
memory, and timeout behavior still require one selected implementation. The
provider can be a later UETD-owned stack, the target SDK stack, or module
socket offload. Only the chosen target profile should pull that implementation
into a build.

## PPP over serial

The PPP facade describes a bounded-MRU link, requested IPv4/IPv6 families,
authentication mode, opaque credentials, and link state. A provider may bind a
PPP-over-UART/modem stack or a native SDK PPPoS implementation. The provider
owns HDLC-like framing, LCP, PAP/CHAP exchange, IPCP/IPv6CP negotiation,
retransmission and binding into the selected IP service; this API does not
implement those protocols or expose credentials.

## TLS and application protocols

TLS/DTLS accepts only opaque credential/trust-store handles. Cryptographic
code, certificate parsing, peer verification, entropy, secure-key access, and
version/cipher-suite policy come from the selected provider. An application
must not set `use_tls` unless the provider verifies the requested policy.

MQTT 3.1.1/5.0, CoAP, and caller-buffered HTTP are provider facades above the
selected IP/offload service. Their providers own framing, ACK/transaction
state, retransmission, redirects, reconnects, backpressure and protocol
interoperability. Fixed event and payload limits are exposed in the headers;
oversized output is reported rather than silently treated as complete.

## Thread and Zigbee

The mesh adapter covers commissioning/state and bounded Zigbee endpoint/cluster
command exchange. Thread application traffic uses the IP/socket facade; Zigbee
traffic uses the declared endpoint and cluster. Both depend on a selected
IEEE 802.15.4 radio provider and an external/vendor Thread or Zigbee stack.
Network keys must be provisioned by a secure product policy; this scaffold does
not persist key material or replace the stack's secure commissioning behavior.
Network credentials are referenced through an opaque provider/secure-store
handle rather than carried as raw key material in the configuration record.

## Promotion gates

Before any module becomes selectable, pin the provider and its license/version,
record target/package/board radio capabilities and resource conflicts, define
bounded storage/work and task/ISR ownership, add generated configuration and
provider binding, and qualify protocol/security behavior on the exact target.
ESP32 capability strings and a successful host build do not establish that
qualification.
