# Connectivity stack and binary artifact plan

**Status:** implementation plan; no radio, network, or security provider is
currently qualified by this document.

This plan consolidates the recent UPAL, UPROTO, and UMID connectivity work. It
distinguishes source code that belongs in UETD from target firmware, build
tools, and product data that a working hardware profile must supply.

## Recommended architecture

Use one UETD-owned Bluetooth LE host above a controller boundary where the
controller exposes standard HCI. This gives external controller modules and
some integrated SoCs a common host implementation. Keep HCI transport and
controller firmware target-specific. The Bluetooth Core specification defines
HCI as the uniform Host-to-Controller command/event interface; it does not
make the radio, controller firmware, or every physical transport identical.

Use the same UPROTO service surface for WiFi, but do not assume WiFi has a
Bluetooth-like standard HCI. Commodity WiFi parts commonly expose a vendor
driver, full-MAC interface, or socket offload. A UETD-owned WiFi facade and
network stack can be portable while the radio provider remains specific to a
module or chip SDK. Replacing those providers with a UETD-owned 802.11 MAC,
security supplicant, and PHY interface is a separate, much larger project.

Application
  └─ UPROTO service APIs
      ├─ Bluetooth → UETD Bluetooth LE host → HCI controller interface
      │    → UPAL bus or integrated virtual-HCI adapter
      │    → controller firmware and radio
      └─ WiFi/IP → selected network provider → UPAL packet netif or socket
           offload → WiFi module/target SDK or Ethernet MAC/PHY
UMID logging and data services sit beside UPROTO and use UPAL storage or
device interfaces where required.

“Host” here means the higher protocol layers on the main MCU. On Bluetooth,
the Host/Controller boundary is standardized by HCI. On WiFi, an SDK adapter
or module protocol will often remain part of the target profile. An integrated
ESP32 provider may call ESP-IDF directly; where the chip exposes a supported
controller-only HCI interface, it may instead connect the common UETD host to
that controller. These are different provider options, not a promise that
every ESP32 variant has the same radio features. ESP-IDF currently is not a
selectable UETD target: CMake selects HOST or CORTEX_M, while ESP-IDF target
work is still scaffolded.

## What the recent changes provide

| Layer | Added or expanded | Current boundary |
|---|---|---|
| UPAL links | SPI/I2C/UART-backed radio transport, asynchronous byte stream and bounded command exchange, generic packet netif, Ethernet link-status facade, and IEEE 802.15.4 frame facade | Transport and dispatch contracts; board bus drivers and actual radio/MAC providers still need implementation and qualification |
| UPROTO connectivity | Bluetooth LE/selected SPP operations, WiFi station/AP/scan, and IP configuration/socket APIs | Typed, validated service/provider interfaces; no built-in Bluetooth host, WiFi driver, IP engine, or cryptography |
| UPROTO future connectivity | PPP, TLS/DTLS, MQTT, CoAP, HTTP, and Thread/Zigbee outlines | Fail-closed scaffolds, excluded from normal builds unless the future-scaffold option is enabled |
| UPROTO other protocol gaps | MSP, ExpressLRS, UAVCAN v1/Cyphal, LIN, J1939, IEC 60870-5, Modbus RTU/TCP, and CANopen outlines | Registered source/API scaffolds; protocol state machines, generated data descriptions, timing, and target bindings remain to be done |
| UMID | Transport-neutral logger with an optional UPAL UART sink; split storage, calibration, blackbox, and process-image contracts | Logger has a provider callback path; UART sink and data-service implementations still need target/backend behavior. Future services fail closed and are excluded by default |

The existing Bluetooth API binds a complete provider. It does not yet expose a
separate reusable HCI Host implementation. Add that lower layer as a distinct
implementation/provider beneath the existing service API so applications can
keep the same calls while provider selection changes.

## Runtime binary and data inventory

Do not check in generated firmware outputs as source dependencies. Build them
from pinned source/toolchain inputs where possible, then retain the exact
release artifacts and their provenance in the build/release system.

| Item | Needed when | Where it belongs and what to record |
|---|---|---|
| UETD static library and final firmware image | Every target profile | Build output, not a hand-maintained binary in the source tree. Record UETD revision, configuration, compiler/linker versions, target profile, ELF/map, image hash, and reproducible build inputs |
| Startup, linker, bootloader, and flash-layout artifacts | MCU target | Target/board package. Pin the startup and linker sources, memory map, boot protocol, flash offsets, and any vendor bootloader images. These are not part of a portable UPROTO provider |
| Bluetooth controller firmware, patch, or ROM-load image | Controller silicon or module requires it | Prefer a reproducible build from redistributable source. If only a vendor binary exists, stage it as an external firmware artifact (proposed layout: UETD/third_party/firmware/<vendor>/<part>/<version>/) or fetch it into the build workspace. Record exact silicon/module revision, upstream URL, version, SHA-256, license/redistribution terms, load address, transport, and compatibility range |
| WiFi firmware, calibration image, regulatory data | Selected radio/SDK requires them | Same external-artifact policy as controller firmware. Record hardware revision, region/source, initialization method, format/version, checksum, and whether the product or vendor supplies calibration. Never infer regulatory approval from a generic UETD profile |
| Thread/Zigbee or proprietary coprocessor image | A selected radio module runs its own stack | Version and hash must match the module protocol and API. Record stack license, security/key provisioning policy, commissioning compatibility, and upgrade/rollback rules |
| Crypto and secure-element implementation | Bluetooth pairing, WPA security, TLS/DTLS, secure boot, or signed updates are enabled | Prefer target hardware acceleration or one selected audited source implementation. Record algorithms, enabled suites, entropy source, key-storage provider, version, and license. Do not put product private keys or passwords in UETD or firmware manifests |
| Trust anchors and client credentials | TLS, enterprise WiFi, or authenticated device links are enabled | Product provisioning inputs or secure-element references, not shared framework blobs. Record rotation/update owner and production/development separation. Keep credentials out of logs and ordinary source control |
| Persisted bonds and UMID records | Bonding, calibration, storage, or blackbox features are selected | Product data partition/backend with schema version, atomic-update and recovery behavior, integrity policy, capacity, retention, and erase/reset rules. The UMID service outlines do not yet implement those guarantees |
| Protocol description inputs | A protocol uses generated or product-specific descriptions | Keep versioned inputs with the product profile: CANopen EDS/DCF, UAVCAN/Cyphal DSDL and generated types, MAVLink dialect XML, Modbus register map, LIN schedule/LDF, J1939 PGN/NAME subset, and IEC 60870 ASDU/address profile as applicable. Record generator versions and output hashes |
| Board and module profile | Every provider binding | Versioned data for bus instance, pins, reset/ready/IRQ/wake, flow control, baud/clock, DMA/cache constraints, voltage, antenna, region, packet sizes, memory budgets, concurrency, and supported capabilities |

The firmware directory above is a proposed location, not a requirement to add
vendor blobs. A checksum and license manifest can be kept in Git while a
restricted or non-redistributable binary is fetched from the approved vendor
source during packaging. Never silently substitute a newer controller image.

## Build and engineering tools

These are build-time tools, not firmware dependencies. Pin versions in the
build environment or lock files; do not copy host executables into the runtime
library.

| Tool binary/package | When needed |
|---|---|
| Native C11 compiler and linker, CMake, and Ninja (or the selected generator) | Host builds, provider adapters, and ordinary UETD builds |
| Cross compiler, assembler, linker, objcopy/binutils, and debugger | Each selected MCU architecture; for example an ARM embedded toolchain for Cortex-M |
| CMSIS/device headers, startup files, and vendor linker inputs | Cortex-M device profile; record which are source inputs versus prebuilt images |
| ESP-IDF, its Python environment, target compiler, and flashing utility | Only after an ESP-IDF target/provider is implemented and selected. Pin the ESP-IDF release, chip target, components, sdkconfig, and binary components |
| Python interpreter and UETD/uef-gen registry-generation utilities | Code/catalogue generation and project assembly; generated sources must record generator version and input hashes |
| Controller firmware loader or vendor packaging utility | Only for radios requiring a patch/blob upload; keep vendor utilities out of the runtime image |
| Protocol generation tools | Only for selected DSDL, MAVLink, CANopen, or other generated descriptions; pin exact versions and preserve source inputs |
| Hardware debug and qualification tools | JTAG/SWD, Bluetooth HCI snoop or protocol analyzer, WiFi/Ethernet packet capture, RF/antenna equipment, and vendor qualification tools; engineering-only, not shipped in the product |

For external code, preserve the current policy of keeping vendor headers out of
public UETD headers. A provider's C file and CMake target should contain the SDK
includes and translate into UETD-owned records. Select only one implementation
per service in a product profile. This keeps public APIs stable and limits the
third-party surface even when an early target uses its SDK's radio or IP stack.

## Work still required

### 1. Freeze the service and controller boundaries

- Add a Bluetooth controller/HCI-provider contract below the existing
  UPROTO Bluetooth operations. It needs bounded transmit/receive, event
  delivery, controller reset/initialization, supported feature discovery,
  buffer limits, and ownership/context rules.
- Define a common HCI packet engine with command credits, asynchronous event
  dispatch, ACL buffer flow control, version/feature probing, reset recovery,
  and bounded poll/work behavior.
- Keep transport framing separate: standard HCI UART, USB, or Secure Digital
  where supported;
  module-specific SPI/I2C framing and integrated virtual-HCI remain adapters.
- Extend UPAL transport only for selected profiles. Likely additions are
  USB/SDIO or DMA/IRQ/wake integration; do not claim generic SPI is a
  standardized HCI transport.
- Keep IP packet mode (upal/netif) and socket-offload mode distinct. An
  offload module must not advertise raw packet access unless it can supply
  packets through netif.

### 2. Implement Bluetooth LE host support

Implement an initially bounded LE-only host: HCI core; L2CAP fixed channels;
ATT client/server; GATT discovery, database and operations; GAP scan,
advertising, central/peripheral connection management; and finite event,
attribute, connection, and packet pools. Add SMP pairing, Secure Connections,
privacy, bond lifecycle, and secure storage only with a reviewed crypto,
entropy, and key-store provider. The existing SPP calls are a separate Classic
BR/EDR feature: they require a BR/EDR-capable controller and host layers such
as SDP/RFCOMM, so do not count them as delivered by the LE host milestone.

Start with one HCI-compatible external LE controller over UART H4. Then add a
second transport/module only when a real product needs it. For an integrated
SoC, add either a controller-only HCI adapter or an SDK stack provider, based
on what that exact chip and SDK expose.

### 3. Implement the IP and WiFi provider strategy

- Select one packet IP implementation and bind it to upal/netif for
  Ethernet and packet-mode radios. The UPROTO IP contract already describes
  DHCP, IPv6/SLAAC, DNS, and TCP/UDP sockets, but no packet engine implements
  them. A UETD-owned bounded source stack best meets the ownership goal but
  carries a substantial IPv4/IPv6, TCP, DHCP, DNS, fragmentation, timeout, and
  security maintenance burden. Start with a single private SDK/module adapter
  if it is needed to qualify the first hardware, while keeping the adapter
  replaceable.
- Add one WiFi provider for an external full-MAC module and one integrated
  target SDK provider when hardware is selected. Capture radio reset, event,
  country/channel, station/AP, credential, coexistence, and power behavior.
- Treat 802.11 PHY/MAC, WPA2/WPA3 supplicant/authenticator, regulatory
  implementation, RF calibration, and certification as provider or vendor
  responsibilities for the first release. A fully UETD-owned WiFi
  implementation is a separate radio-driver and security program.
- Define one credential/secret interface and one entropy/key-store interface
  before enabling WPA3, Bluetooth pairing, or TLS. Never copy raw secrets into
  logs, persistent generic configs, or generated project manifests.
- Add TLS/DTLS, PPP, MQTT, CoAP, and HTTP only after the selected IP/socket
  provider, resource bounds, version profile, and crypto policy are explicit.
  Thread/Zigbee additionally require a qualified 802.15.4 radio and selected
  stack. The current future scaffolds do not provide those engines.

### 4. Complete target and module build integration

- Make the ESP-IDF backend a real UETD_TARGET option only after UHAL/UOS
  target contracts, compiler configuration, component selection, and
  reproducible build inputs are implemented. Add ESP-IDF-specific Bluetooth,
  WiFi, Ethernet/netif, and IP adapters as private source files.
- Define project-profile fields for radio/module part and revision, firmware
  artifact identity, provider choice, bus and GPIO resources, feature bitmap,
  security modes, memory pools, and capability conflicts.
- Ensure generated projects select required provider sources and artifacts,
  emit clear errors when a required binary/license/input is missing, and
  exclude unused protocols and stacks.
- Keep all future UPROTO/UMID fail-closed scaffolds off by default until a
  real implementation is chosen. Future protocol source files being present
  is not a reason to ship their dependencies.

### 5. Finish UMID binding and persistence

- Implement the UART logger sink over UPAL UART; specify buffering, ISR/task
  ownership, backpressure, truncation, and drop counters.
- Connect UMID storage to selected flash/QSPI/SD/MMC backends with atomic
  commit, integrity, recovery, version migration, wear, and capacity bounds.
- Define calibration identity/version and record migration; keep calibration
  algorithms in sensor/driver/application policy.
- Implement blackbox streaming with a bounded producer queue, timestamp
  source, loss accounting, file/partition rollover, and explicit retention.
- Decide whether the process image is the common typed data record for Modbus,
  CANopen, and telemetry rather than allowing each protocol to invent a
  separate PLC scan model.

### 6. Qualify each exact profile

For every shipping combination of MCU, board, radio/module, controller
firmware, provider, and stack version, retain:

- source/toolchain/configuration lock data and hashes for all external blobs;
- flash/RAM/stack/queue/packet-pool budgets and task/ISR ownership;
- bus timing, reset/recovery, link-loss, reconnect, and power-cycle results;
- Bluetooth packet/HCI traces and required interoperability/security evidence;
- WiFi association, security-mode, country/channel, disconnect/reconnect, and
  packet tests; Ethernet link/DHCP/IP tests where selected;
- protocol conformance, malformed-input, timeout, overflow, and bounded-work
  results; and
- release image, map file, SBOM/license inventory, and qualification record.

Host compilation validates API and provider glue only. It does not qualify
radio firmware, RF performance, protocol interoperability, product regulatory
approval, Bluetooth qualification, or secure key handling.

## Suggested delivery order

1. Select the first MCU/board and one external LE HCI controller/module; record
   the exact controller firmware and board resources.
2. Implement the HCI provider contract and UART H4 transport, then the
   bounded LE host core and basic GAP/GATT flows.
3. Add bonding only after secure storage, entropy, and cryptographic provider
   decisions are resolved.
4. Bring up one Ethernet or WiFi profile through netif/IP; implement private
   SDK/module providers first if needed, then replace selected network code
   with UETD-owned source only where the maintenance and security cost is
   justified.
5. Implement the ESP-IDF target and integrated providers for one exact ESP32
   variant. Do not treat the family name as a complete capability profile.
6. Complete UMID UART logging and persistent data services against selected
   board storage.
7. Promote PPP, TLS/DTLS, MQTT, CoAP, HTTP, Thread/Zigbee, and industrial
   protocols in dependency order, with a separate profile and qualification
   record for each.

## Primary references

- [Bluetooth Core Specification, Host Controller Interface functional specification](https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Core_v6.3/out/en/host-controller-interface/host-controller-interface-functional-specification.html)
- [Bluetooth SIG Core configurations and qualification overview](https://qualification.support.bluetooth.com/hc/en-us/articles/26704878828813-Understanding-Core-Configurations)
- [ESP-IDF Bluetooth API and supported host-stack options](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/index.html)
- [ESP-IDF Controller and HCI API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/bluetooth/controller_vhci.html)
- [ESP-IDF WiFi driver guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html)
- [ESP-IDF network interface and lwIP binding](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_netif.html)
