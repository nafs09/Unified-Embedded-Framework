# Cortex-M target integration template scaffold

Use this path for the eventual UETD-owned target adapter used by uef-gen. The
selected board record supplies exact part/core/device-header IDs and the
toolchain record supplies local executable roles. Do not select a device by
substring or generate a memory map from a family name.

## Generated files to design

- `CMakeLists.txt` or a generated toolchain fragment: select the exact GNU
  cross compiler, CPU/FPU/float ABI, CMSIS include package, UETD target backend,
  startup source, linker script, libraries, and output format.
- `target/startup.s` or a pinned startup source reference: core reset entry,
  vector table, stack, `.data` copy, `.bss` zero, FPU enable, and system init
  call. For an H745/H755 build, name the selected M4 or M7 core explicitly.
- `target/link.ld`: exact flash banks and RAM domains, vector placement,
  bootloader/secure reservations, core ownership, DMA/cache sections, stack,
  heap, load address, and retained sections.
- `target/system_init.c`: verified clock tree, flash wait states, MPU/cache
  setup, bounded readiness waits, and published clock values.
- `manifest/target_build.json`: registry IDs/revisions, startup/linker source
  identities, compiler package ID/version, flags, output formats, and memory
  map fingerprint used by artifact provenance.

## Core profile differences

M4 targets require the selected FPU/ABI flags and their specific flash/RAM map.
M7 targets add instruction/data cache, MPU, DTCM/ITCM and DMA coherency rules.
H745/H755 dual-core parts need separate M4 and M7 images, startup, linker
regions, inter-core/shared-memory ownership, and flash ranges. Treating the
whole part as a single generic M7 target is not sufficient.

TODO(cortex-m-template): define registry-backed substitutions and validation
for every generated field before adding executable template files. Keep an
unknown/ambiguous core or memory region as a generation diagnostic.
