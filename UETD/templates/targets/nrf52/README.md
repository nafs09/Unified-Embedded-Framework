# Nordic nRF52840 target adapter template

This template defines the nRF52840 target-adapter contract for the Phase 0
catalogue entry `nrf52840-qiaa`. The `zephyr/` Jinja files provide a Zephyr
application scaffold only; UETD does not yet register a generation-ready
nRF52840 board or nRF-aware build/flash adapter. The corresponding package
records are `nrf-connect-sdk`, `zephyr-sdk`, `cmake`, and `ninja`; all are
scaffold-only with no installable pinned payloads.

Render `zephyr/CMakeLists.txt.tmpl` with `project_name`, `sources`,
`include_dirs`, and `compile_definitions`. Render `zephyr/prj.conf.tmpl` with
`configuration`, an ordered list of `{name, value}` Kconfig entries. Validate
Kconfig names/values and generated Devicetree against the selected board before
building. The selected board and SDK/toolchain come from the West build profile;
these templates do not generate Devicetree overlays, partition maps, or flash
commands.

## Build profile inputs

The resolved profile must provide:

- exact nRF52840 chip record, silicon revision, package, and board revision;
- selected runtime and SDK family/version (for example a pinned nRF Connect
  SDK/Zephyr integration or an explicitly defined bare-metal profile);
- compiler, architecture flags, startup/CRT owner, linker script, and output
  artifact role;
- application, bootloader, MBR/SoftDevice if used, settings page, UICR, and
  retained calibration/configuration ranges;
- LFCLK source and accuracy, HFCLK request policy, regulator/power setup,
  antenna/RF front-end configuration, and board-specific GPIO reservations;
- exact debug probe, transport, serial number selection, reset behavior,
  image conversion/signing, and verify procedure.

Do not assume a SoftDevice, MBR, MCUboot, Zephyr partition layout, LF crystal,
QSPI device, or bootloader. These are profile choices and must have explicit
addresses and ownership in the resolved memory map.

## Generated project contract

The generated manifest should pin the chip and board IDs/revisions; Nordic SDK
and toolchain package IDs; compiler flags; Kconfig/Devicetree inputs; generated
source inventory; linker/partition-map fingerprint; image format; and flash
profile. SDK-owned startup, linker, radio and interrupt code must enter through
declared package paths and the SDK's project model. Do not copy local machine
paths or hand-maintained duplicate Nordic device headers into the project.

The adapter must preserve the distinction between application-owned
peripherals and the radio/controller/runtime. Validate that selected BLE,
802.15.4, USB, QSPI, and DMA functions do not conflict with board wiring or SDK
ownership. Radio scheduling and interrupt latency constraints must be carried
into simulation/qualification metadata when an application uses the radio.

## Required functions and validators

1. `resolve_nrf_profile(chip, board, runtime)` — reject missing silicon/board
   revision, SDK, LFCLK, memory, or flash choices.
2. `render_nrf_project(project, profile)` — emit the SDK-native build project
   and provenance manifest using pinned package roots.
3. `resolve_nrf_memory(profile)` — prove all application, bootloader, settings,
   UICR, and optional SoftDevice/MCUboot ranges are disjoint and in range.
4. `render_nrf_flash_plan(artifact, profile)` — choose the exact probe/transport
   and write only the declared image regions; preserve protected ranges.
5. `validate_nrf_artifacts(build)` — check ELF/image identity, region bounds,
   signing metadata, and package/source closure before a flash plan is offered.

Reference sources: [nRF52840 Product Specification](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/keyfeatures_html5.html)
and Nordic's [revision-specific documentation and errata index](https://docs.nordicsemi.com/r/bundle/comp_matrix_nrf52840/page/comp/nrf52840/nrf52840_doc_ref_design_files_overview.html).
