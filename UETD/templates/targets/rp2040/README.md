# Raspberry Pi RP2040 target adapter template

This template defines the Phase 2 adapter contract for the `rp2040` chip and
Pico board records. `CMakeLists.txt.tmpl` is an SDK-native CMake project
scaffold; it is not a programmer or generation-ready build path.

The template uses the `pico-sdk` package with the `arm-none-eabi`, `cmake`,
and `ninja` packages. These package records remain scaffold-only until a
versioned SDK/toolchain payload and host installation procedure are verified.

Render `CMakeLists.txt.tmpl` with `project_name`, `target_name`, `sources`,
`include_dirs`, `compile_definitions`, `compile_options`, `libraries`,
`stdio_usb`, and `stdio_uart`. The build environment must resolve the pinned
Pico SDK through `PICO_SDK_PATH` and the selected board through `PICO_BOARD`;
the template fails configuration when either is missing. The SDK creates ELF,
binary, HEX, and UF2 outputs. The board profile still decides which format is
programmed and how.

## Board and boot inputs

RP2040 contains no program flash; standalone images execute from external flash
through the XIP subsystem. The resolved board must name the flash device and
capacity, connected flash pins, clock source, boot-ROM boot path, debug probe,
and recovery behavior. The Pico board records include the on-board flash/XIP
window, but that is not enough to establish build/flash readiness.

The project profile must select the pinned Pico SDK/runtime, C/C++ toolchain,
startup owner, linker script, boot-stage policy, image format (`ELF`, binary,
and/or board-supported `UF2`), and flashing method. Do not treat the external
flash as chip memory or publish a linker map until the board-specific flash
range and boot-stage reservation are verified.

## Shared hardware allocation

The project must allocate the following resources explicitly:

- core 0/core 1 startup, entry points, stacks, interrupt ownership, and shared
  state; include multicore lockout and watchdog/recovery policy;
- shared SRAM regions and synchronization protocol for cross-core data;
- PIO blocks/state machines/instruction memory and the generated PIO program
  inventory;
- DMA channels, request sources, transfer ownership, and IRQ routes;
- timer/alarm and GPIO/pin assignments, including board-reserved functions;
- SIO divider use and contention rules.

The Cortex-M0+ instruction set has no integer divide instruction. RP2040 adds a
dedicated SIO divider for each core; the profile should use the Pico SDK's
divider interface rather than label this as a core divide instruction. The
chip's XIP cache is not a Cortex-M instruction/data cache and must be modeled
as a separate SoC/flash subsystem.

## Generated artifacts and validation

Emit the SDK-native CMake project, selected linker input, generated target and
board capability headers, PIO source/header outputs, and a `target_build.json`
manifest. The manifest records chip/board record revisions, Pico SDK and
compiler package IDs, boot and memory profile fingerprints, source closure,
artifact hashes, image formats, and flash method.

The validator must check that both core images/entry points and all PIO/DMA
allocations are declared, linker regions fit the on-chip SRAM and selected
external flash, image conversion preserves boot metadata, and the chosen
programmer can identify and verify the exact board. It must reject flash or
reset operations when the board/recovery profile is incomplete.

Reference sources: [RP2040 specifications](https://www.raspberrypi.com/products/rp2040/specifications/),
[RP2040 datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf),
and [Pico board documentation](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html).
