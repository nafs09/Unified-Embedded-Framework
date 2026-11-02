# First-pass scope and placement

29 October 2026. This is the scope decision for the complete-system design.
An implementation may be missing and still belong to the first pass. Readiness
is separate: no row becomes generation-ready until its implementation,
contract, provider and target evidence exist.

The first-pass display driver is implemented as the generic `upal/display`
provider facade in the main source/catalogue tree. The other missing families
below have detailed design contracts; they are not claimed as implemented.
The [final scaffold review](../../audits/UETD_FINAL_SCAFFOLD_REVIEW_2026-10-29.md)
records 26 additional first-pass typed source scaffolds and eleven host pipeline
obligations. Their interfaces are drafts; every new source operation remains
unsupported and generation-blocked.

## First-pass runtime requirements

| Capability | Placement / implementation route | Owning design |
|---|---|---|
| Checked sizes, spans, endian cursors and codecs | UCORE portable primitives; static storage and transactional cursor errors | [Core](FOUNDATIONS.md#core-values-errors-and-checked-storage) |
| Common monotonic clock and checked deadlines | UCORE value contract, UHAL clock provider, UOS waits in the same domain | [Time](FOUNDATIONS.md#monotonic-time-and-deadlines) |
| Pin/clock/IRQ/DMA/trigger/memory graph | Exact chip/package/board records with typed resource predicates | [Resources](FOUNDATIONS.md#exact-target-resource-graph) |
| Startup/reset/fault capture and memory layout | UHAL target profiles plus generated initialization/linker plan | [Startup](FOUNDATIONS.md#startup-memory-protection-and-cpu-services) |
| Bounded asynchronous transfers | UPAL transaction tokens, exact completion and pixel/DMA buffer ownership | [Transactions](FOUNDATIONS.md#driver-transaction-and-dma-ownership) |
| GPIO events, UART/RS-485, SPI, I2C, timers/capture | Qualified peripheral implementations with status/timeout and shared-bus rules | [Peripherals](FOUNDATIONS.md#existing-peripheral-families) |
| ADC/DAC/comparator, synchronized acquisition/PWM | Trigger cohorts, skew and analog timing plus hardware output inhibit | [Peripherals](FOUNDATIONS.md#existing-peripheral-families) |
| CAN and CAN-FD transport | Exact timing/transceiver/FIFO/filter and bus-off profile; separate from CANopen FD | [Peripherals](FOUNDATIONS.md#existing-peripheral-families) |
| Flash/QSPI/SDMMC and common block device | Geometry, cache/DMA, durability and removable-media generation | [Storage](PRODUCT_SERVICES.md#storage-partitions-and-filesystems) |
| Watchdogs/RTC/low-power integration | Window/clock/retention/wake facts and centralized feed authorization | [Power](FOUNDATIONS.md#power-and-clock-management) |
| Cooperative and static RTOS execution | Real supported primitives, deadline/status/ISR/static-storage contract | [UOS](FOUNDATIONS.md#uos-execution-and-synchronization) |
| Deferred work and timers | Fixed queues, bounded callbacks, explicit execution context | [UOS](FOUNDATIONS.md#uos-execution-and-synchronization) |
| Pools, queues, mailboxes and event distribution | Distinct ownership/overflow/lifetime contracts and coherent values | [Dataflow](PRODUCT_SERVICES.md#buffer-pools-events-and-dataflow) |
| Sensor lifecycle, sample quality/time and calibration | Common envelopes above provider/project-owned device drivers | [Sensors](PRODUCT_SERVICES.md#sensor-and-acquisition-services) |
| Digital/PWM/servo/stepper/motor/protocol actuators | Typed command admission, feedback, arming/inhibit and local safe output | [Actuators](PRODUCT_SERVICES.md#actuator-services-and-local-safety) |
| Transactional records, settings and binary logs | Bounded recovery, integrity/commit, wear, schema migration and host decoding | [Storage](PRODUCT_SERVICES.md#storage-partitions-and-filesystems) |
| Typed configuration transactions | Cross-field/resource validation, generations and safe activation/rollback | [Settings](PRODUCT_SERVICES.md#settings-and-configuration-transactions) |
| Industrial process image | Atomic scan publication, quality/freshness, authority and safe values | [Protocols](CONNECTIVITY_SECURITY.md#first-pass-application-protocols) |
| Lifecycle graph, recovery, fault history | Ordered start/cleanup, bounded retry, retained crash records and export | [Recovery](PRODUCT_SERVICES.md#lifecycle-supervision-and-recovery) |
| Structured diagnostics, telemetry and maintenance shell | Bounded typed records/commands, redaction and access policy | [Diagnostics](PRODUCT_SERVICES.md#diagnostics-telemetry-and-maintenance) |
| Ethernet/WiFi/BLE/IP provider integration | Exact target/stack/controller firmware, bounded sessions and real peer tests | [Connectivity](CONNECTIVITY_SECURITY.md#provider-layering) |
| Network connection management | Address/session generations, reconnect/backoff and dependent-service invalidation | [Networking](CONNECTIVITY_SECURITY.md#network-and-connection-manager) |
| TLS, credentials, entropy and authorization | Shared security-provider contract; selected vetted implementations | [Security](CONNECTIVITY_SECURITY.md#security-service-boundary) |
| MQTT/HTTP, Modbus RTU/TCP and CANopen CC | Named bounded protocol subsets and selected providers | [Protocols](CONNECTIVITY_SECURITY.md#first-pass-application-protocols) |
| USB CDC/HID and selected device update path | Descriptor/endpoint/lifecycle budget and interoperability | [Protocols](CONNECTIVITY_SECURITY.md#first-pass-application-protocols) |
| Existing PPM/CRSF/SBUS/DSHOT/MAVLink/DroneCAN scope | Complete the selected portable framing and target binding | [Protocols](CONNECTIVITY_SECURITY.md#first-pass-application-protocols) |
| Provisioning, authenticated boot/update and rollback profiles | Exact key/bootloader/partition/maintenance capability and interruption tests | [Update](CONNECTIVITY_SECURITY.md#boot-update-and-rollback) |
| Display, touch/input and graphics/UI | Main `upal/display` facade, exact OLED/provider and bounded GUI adapter | [Handheld specifics](MYOVENA_HANDHELD.md) |
| MIPI display-host and panel integration | DSI/DSI-2, DCS/panel profiles and exact hardware-backed D-PHY/C-PHY compatibility; optional per target, still first-pass | [MIPI display link](MIPI_DISPLAY_LINK.md) |
| BLE/WiFi product-control session | Identity, command expiry/authority, observable outcome and stale-state handling | [Handheld controls](MYOVENA_HANDHELD.md#blewifi-control-semantics) |
| Shared numeric foundation and common filters | All required first-pass UMATH work, reviewed bounded bindings | [Numerics](MATH_CONTROL.md#umath-module-design) |
| Existing FIRST_PASS controllers/estimators | Typed family contracts and plant/profile obligations; remain first pass when blocked | [Algorithms](MATH_CONTROL.md#typed-algorithm-instance-contract) |
| Dense bounded QP foundation | First-pass active-set reference for existing MPC scope | [Optimization](MATH_CONTROL.md#optimization-and-mpc) |
| AMP and typed multi-node links | Explicit core/image ownership, bounded channels, freshness and local safety | [Systems](FOUNDATIONS.md#intercore-and-distributed-boundaries) |

Optional per-product selection does not mean later priority. A tiny controller
does not link display/TLS/networking merely because those are first-pass
capabilities for another product.

## First-pass generator and delivery requirements

The [generation design](GENERATION_QUALIFICATION.md) specifies registry-backed
typed IR and full C emission, source maps, provider/dependency locking, exact
resource assignment, initialization and scheduling, static memory/workspace
planning, codec generation, SDK/startup/linker/build files, standalone package
consumption, artifact verification/signing, read-only device discovery,
qualified programming/update, decoded diagnostics, manufacturing records,
generated/direct equivalence and target evidence. These belong in the first
usable generator. They are not placed in later scope because they are unfinished.

## Genuinely later requirements

Later scope includes audio, camera/imaging, inference, general removable-device
discovery, advanced robotics planning, cellular/PPP/LPWAN, mesh/Thread/Zigbee,
CoAP/LwM2M, CANopen FD and proprietary MEDI CAN, J1939/LIN/IEC60870 and specialized
industrial-Ethernet profiles, advanced USB host/audio/video, a UETD-owned BLE
host, sparse/alternative optimization methods, selected advanced control
variants, dynamic graph loading, fleet/graphical orchestration and general Linux
image generation. A concrete first-pass product can deliberately promote a
particular later capability; that requires the same contract/evidence rules.

Deferred plant-specific methods remain blocked on their actual model and
validation. Difficulty, incomplete source, or lack of a board alone is not a
reason to relabel a foundation as later work.

## Proposed migration of older first-pass entries

Status: **executed after explicit approval**. All 51 approved files moved, 10 canonical module IDs changed, and all 20 planning entries now live in `registry/first_pass/registry.json`. Six later entries remain in `registry/future/registry.json`. Two blocked UAPP design records were added for secure update and fault history. Implementation and generation remain blocked.

The pre-move content and hashes of 1,865 source/document/template files are retained in `C:/NEXUS/uetd-audit-work/first-pass-migration-snapshot`. The table below preserves the approved old/new mapping; old paths in this historical table are not active references.

The approved move comprised **51 files**, **10 canonical module-ID changes**,
and reclassification of **20 planning entries**. Each move remains within the
current repository and is a file rename with updated references. No third-party
tree, generated user project, external application or physical device is changed.
The current uncommitted content must be preserved, with a pre-move snapshot and
a reviewable diff. Destination collisions or changed inputs stop the migration.

| Original location | Executed destination | Files / canonical module changes |
|---|---|---|
| `include/umid/future/{storage,calibration,blackbox,process_image}.h`, matching `src/umid/future/*.c`, matching module manifests | Corresponding `include/umid/`, `src/umid/`, and `manifest/umid/` files | 12 files; `umid/future/<name>` becomes `umid/<name>` for four IDs |
| `include/umid/future/data_services.h`, `DATA_SERVICES.md` | `include/umid/data/data_services.h`, `DATA_SERVICES.md` | 2 files |
| `include/uapp/future/{secure_update.h,fault_history.h,SECURITY_AND_UPDATE.md,FAULT_HISTORY.md}` | Corresponding `include/uapp/` files | 4 files; main blocked records for the two services |
| `include/uproto/future/connectivity/{tls,mqtt,http}.h`, matching sources and module manifests | Corresponding `include/uproto/`, `src/uproto/`, and `manifest/uproto/` files | 9 files; three IDs become `uproto/{tls,mqtt,http}` |
| `include/uproto/future/industrial/{modbus,canopen}.h`, matching sources and module manifests | Corresponding `include/uproto/`, `src/uproto/`, and `manifest/uproto/` files | 6 files; two IDs become `uproto/{modbus,canopen}` |
| `include/uproto/future/{industrial_contracts.h,INDUSTRIAL.md}` | Corresponding `include/uproto/` files | 2 shared contract/document files |
| `include/upal/future/capability_contracts.h` | `include/upal/capability_contracts.h` | 1 shared UART/capture/synchronized-I/O/USB outline |
| `include/ucon/future/optimization/qp/qp_active_set.h`, matching source and manifest | `include/ucon/qp_active_set.h`, matching source and manifest | 3 files; one canonical module ID loses `/future/` and becomes FIRST_PASS |
| Host `deploy/future/SECURE_UPDATE.md`, system `future/EXTERNAL_ENDPOINTS_AND_EVIDENCE.md` | The same owning directories without `/future/` | 2 documents |
| Ten shared record schemas currently in `registry/future/` | `registry/contracts/` with refreshed schema IDs/references | 10 schemas: persistent record, process image, external artifact, calibration, blackbox, qualification evidence, signed artifact, crash, update progress, trust policy |

Reclassified planning entries are `upal.can_fd`, `upal.uart_modes`,
`upal.timer_capture_events`, `upal.synchronized_io`, `upal.usb_profiles`,
`umid.storage_transactions`, `umid.calibration_records`, `umid.blackbox`,
`umid.plc_process_image`, `uapp.secure_update`, `uapp.fault_history`,
`uproto.modbus`, `uproto.canopen`, `uproto.tls`, `uproto.mqtt`, `uproto.http`,
`project_language.type_interchange`, `system_language.external_evidence`,
`deploy.signed_update`, and `evidence.qualification`.

After the move, the runtime records remain explicitly generation-blocked
and declare Phase 0 placement. Public-header references, owning docs, scope
indexes and generated routes/type references were updated. Schema, closure,
C/C++ inclusion, host build/test and changed-source consumer checks passed.
The [migration result](../../audits/UETD_FIRST_PASS_MIGRATION_2026-10-29.md)
records the exact file mapping and verification. The migration changes
placement and metadata; it does not implement the missing services.
