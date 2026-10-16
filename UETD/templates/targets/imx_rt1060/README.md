# NXP i.MX RT1060 target adapter template

This template covers the Phase 2 exact part `MIMXRT1062CVL5B` using NXP's
MCUXpresso SDK CMake model. It does not provide a registered board profile or
a generation-ready toolchain/flash path.

The project depends on the `mcuxpresso-sdk-rt1060` and `arm-none-eabi`
packages, plus `cmake` and `ninja`. Their registry records are scaffold-only;
the SDK version, host installation, compiler paths, board ports, and license
acceptance still need qualification.

Render `CMakeLists.txt.tmpl` with `project_name`, `board_example_category`,
`sources`, and `include_dirs`. The configure environment must provide
`SdkRootDirPath`, `board_root`, and `board` from the pinned SDK and selected
board profile. The template uses NXP's `mcux.cmake`, SDK root CMake assembly,
`mcux_add_source`, `mcux_add_include`, and `mcux_convert_binary` entry points.
The generated `.bin` is a raw link image; this template does not add an IVT,
boot data, FlexSPI configuration block, signature, or flash operation.

## Board, boot, and external-memory inputs

The selected RT1062 variant has on-chip SRAM but no program flash. The exact
board profile must define the boot source and mode, FlexSPI controller and
attached NOR device, flash capacity and sector geometry, boot configuration
block, image-vector-table (IVT) and boot-data placement, and recovery/debug
method. Optional SDRAM/PSRAM, HyperFlash, SD/eMMC, and secondary FlexSPI devices
must be modeled only when present on that board.

The profile must also supply exact oscillator sources and PLL policy, package
pin mux, power sequencing, silicon revision/errata selection, and external
memory training/readiness rules. Never infer the memory map or boot image
addresses from `MIMXRT1062` alone.

## Build and memory model

Use a pinned MCUXpresso SDK and its device/startup support through an explicit
UETD package record. The generated project must name the compiler, CPU/FPU ABI,
startup owner, linker description, generated pin/clock configuration, image
conversion tools, and artifact format. The linker profile must distinguish
ITCM, DTCM, OCRAM, external RAM, and FlexSPI XIP regions using exact device and
board data; it must mark each region's executable, DMA, cacheable, and retained
properties. TCM and cache-enabled OCRAM/external memory need distinct placement
and DMA-coherency policies.

Emit at minimum the SDK-native build project, linker input, system/clock-init
boundary, boot configuration input, and `target_build.json` manifest. The
manifest records board/chip/silicon revisions, SDK/tool package IDs, boot-mode
and memory-map fingerprints, image conversion/signing tools, output hashes,
and flash ranges.

## Flash and safety checks

The flash plan must identify how the image is delivered (for example, serial
download or a board debug probe), whether it is a RAM-loaded image or a
bootable external-flash image, and how the device is returned to its normal
boot source. A RAM debug load must not be reported as a persistent flash. A
bootable image must include and validate the board-specific IVT/boot data and
any required FlexSPI configuration block. Do not emit fuse/eFuse writes or
modify boot-source straps as part of a normal application flash.

The adapter validator must reject overlapping ITCM/DTCM/OCRAM/external memory
regions, missing external memory timing/configuration, unsupported DMA buffer
placement, mismatched cache-maintenance policy, and artifact ranges outside
the selected flash device.

Reference sources: [NXP i.MX RT1060 product and documentation index](https://www.nxp.com/products/i.MX-RT1060?tab=Documentation_Tab),
[MIMXRT1062CVL5B product record](https://www.nxp.com/part/MIMXRT1062CVL5B),
and the [MCUXpresso MIMXRT1062 device reference](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/api/devices/MIMXRT1062/index.html).
