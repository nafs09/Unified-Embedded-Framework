# Build, artifact, probe, flash, and monitor source map

This package owns the host-side pipeline after UETD has supplied target facts.
All physical write operations are scaffolds until their target identity,
artifact range, protection behavior, erase geometry, verification, timeout, and
cleanup contracts are implemented. A `NotImplementedError` is intentional and
must not be replaced with a success-shaped placeholder.

## End-to-end order

1. `build_manager.BuildManager.build(config)` validates the generated project
   and orchestrates compile, link, image conversion, size report, and memory
   budget checks. Its helper headings split list parsing, path validation,
   compile, link, objcopy, and size parsing so each operation can be reviewed.
2. `models.FirmwareArtifact.from_build(...)` turns a successful build plus the
   generated manifest into artifact provenance. It must use linker-derived
   absolute load addresses, not only the chip's flash capacity.
3. `artifact_store.py` persists and reloads artifact metadata. It must verify
   hashes when an artifact is reopened and reject paths that escape its
   project/artifact root.
4. `target_discovery.TargetDiscovery.enumerate_probes()` and
   `identify_target()` find endpoints and read identity without erase, reset,
   unlock, or mode changes.
5. `backend_factory.DeploymentBackendFactory.create()` selects the backend
   named by the UETD board `flash.backend` profile. Probe type and backend must
   agree; the factory itself must never connect.
6. `orchestrator.DeploymentOrchestrator.deploy()` validates the artifact and
   selected probe, then owns the write sequence: connect, identify, exact-match,
   confirm, halt, inspect protection, validate range/geometry, erase, program,
   verify, reset/run, disconnect. Any failure reports the phase and still
   disconnects.
7. `monitor.SerialMonitor` attaches to a user-selected wired console profile
   from the UETD development-board record. Monitoring is read-only: no reset,
   erase, mode switch, or implicit flash. Wireless transports are outside V1.

## Files and implementation headings

| File | What to implement there |
| --- | --- |
| `../schemas/artifact.schema.json` | Versioned sidecar shape for target/UETD/project/build identity, primary output files, and every independently addressed image segment. |
| `build_manager.py` | Parse canonical list files as structured data; validate project-local paths/flags; resolve one coherent toolchain; execute bounded argv processes; compile deterministic object paths; link ELF/map; convert HEX/BIN; parse sections and compare verified budgets. |
| `models.py` | Define immutable build inputs/results, artifact provenance and staleness, target identity, probe identity, and structured progress/result values. |
| `artifact_store.py` | Write/read versioned artifact sidecars, hash each output, bind UETD/chip/project/source fingerprints, and reject stale/tampered output. |
| `target_discovery.py` | Optional USB/serial/DFU enumeration, stable endpoint IDs, read-only identity, and exact artifact compatibility. |
| `ideployment.py` | Backend contract for connect, identity, halt, protection inspection, exact erase, program, verify, reset, disconnect, and temporary read-only identity. |
| `orchestrator.py` | State transitions, phase validation, explicit confirmation payload, progress callbacks, cancellation policy, and unconditional cleanup. |
| `backends/openocd.py` | OpenOCD config selection, bounded process/RPC, ID read, halt, geometry-aware erase, program, verification, reset, and process cleanup. |
| `backends/stlink.py` | ST-LINK selection and its configured CLI/session lifecycle; exact ID and memory checks precede writes. |
| `backends/pyocd.py` | CMSIS-DAP session lifecycle, device-pack resolution, identity, page geometry, programming, readback, reset, and cleanup. |
| `backends/jlink.py` | SEGGER CLI/session lifecycle, serial selection, device identity, exact ranges, verification, and cleanup. |
| `backends/dfu.py` | DFU interface/alternate-setting selection and explicit mode/profile; keep discovery read-only and use image format required by the target DFU implementation. |
| `backends/avrdude.py` | ATmega/Arduino programmer profile, exact AVR signature, application-only programming, bootloader preservation, and no implicit fuse/EEPROM/chip-erase operation. |
| `backends/esptool.py` | Classic ESP32 ROM bootloader over an explicitly selected serial port; preserve reset-line policy and program/verify each ESP-IDF image segment at its manifest offset. |
| `backends/uart_boot.py` | Protocol-specific bootloader negotiation, packet checksums/retries, address validation, verification, reset, and serial cleanup. |
| `backends/ti_uniflash.py` | Vendor CLI selection and part-specific commands; require exact identity and do not infer option-byte/security actions. |
| `monitor.py` | Validate `debug_console`; enumerate stable serial port choices; connect with board framing; bounded incremental decode/line handling; cancellation and close diagnostics. |
| `cli.py` | `build`, `probe`, `deploy`, `flash`, and `monitor` arguments, human/JSON output, and stable error codes without putting diagnostic text on JSON stdout. |

Each backend file already has method-level headings and a TODO describing what
to put in that method. The shared interface in `ideployment.py` is the API
checklist. When one backend cannot support a method, return an explicit
unsupported result and keep that capability unavailable in its UETD flash
profile.

## UETD records this package consumes

- Exact chip: `registry/targets/chips/registry.json` plus the chip record's
  verified `resources.memory_regions` and device identity facts.
- Physical board and transport: `registry/targets/development_boards/registry.json`
  plus `build` and `flash` profiles, USB/serial/console details, and source links.
- Tool installs and version pins: `registry/targets/toolchains/registry.json`
  plus one ready record per host. `scaffold_only` means visible in `tools list`
  but unavailable to build or flash.
- ESP-IDF image sets need more than one output: bootloader, partition table,
  and application images have separate offsets. `FirmwareImageSegment` and the
  optional `FirmwareArtifact.segments` field now scaffold per-image paths,
  hashes, roles, formats, and half-open address ranges. `BuildManager`,
  `ArtifactStore`, and the IDF adapter still need to populate and validate those
  segments before `esptool` can safely program an IDF project.
- UETD/module provenance: generated `manifest/*.json`, `project_sources.txt`,
  target config, and the UETD snapshot fingerprint.

Do not enable a board's generation/flash profile simply because its JSON passes
schema validation. Schema validity means the record is well-shaped; it does not
mean the hardware map or programmer flow has been qualified.
