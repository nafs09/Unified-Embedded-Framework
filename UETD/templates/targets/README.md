# Target project and linker scaffolds

Target startup and link behavior belongs to a selected build profile, not to a
chip-name heuristic in `uef-gen`. The exact chip and board records live in
`registry/targets/`; this directory documents the generated-file contracts and
is the place for UETD-owned starter templates as those contracts are promoted.

## Files a generated standalone project needs

| Generated file or input | Purpose | Required source of truth |
| --- | --- | --- |
| `config/uetd_target.h` | Compile-time architecture and capability snapshot | Exact chip record, with data-verification state retained. |
| `target/system_init.c` | Board clock, memory-protection, and cache setup hook | Board clock tree plus chip clock/reset and memory data. |
| `target/startup.*` | Reset entry, vectors, stack, and initialized-data/BSS setup | Vendor startup package or a reviewed UETD-owned startup template. |
| `target/link.*` | Memory regions, section placement, stack/heap, load addresses | Verified chip memory regions and selected runtime/toolchain ABI. |
| `project_sources.txt` | Canonical generated and copied compilation units | UETD module manifest and generated-file inventory. |
| `project_includes.txt` | Canonical include roots | Selected module closure and external dependency profile. |
| `project_defines.txt` / `project_cflags.txt` | Compiler policy for the selected target | Board build profile plus project configuration. |
| `project_libraries.txt` | Link libraries and ordering | Selected UETD modules and build profile. |

`uef-gen` currently emits intentionally incomplete startup and linker
placeholders from `uef-gen/src/uef_gen/templates/project_scaffold.py`. They are
not included in the canonical source list. The generated project validator
checks file/path closure; it does not prove that startup code, memory addresses,
or the final linked image are correct.

## Target-specific generation rules

The current architecture notes are split by startup/link model:
[`cortex_m/README.md`](cortex_m/README.md), [`avr/README.md`](avr/README.md),
[`arduino/README.md`](arduino/README.md), and
[`esp_idf/README.md`](esp_idf/README.md). These are implementation
checklists, not generation-ready template payloads. Keep a board's
`generation_available` flag false until the corresponding adapter files and
validator are implemented and its compiler can consume the generated project.

### Cortex-M (including STM32 M4, M7, and dual-core H745/H755)

- A concrete part/board profile supplies the device header, startup source,
  linker script, vector-table location, flash banks, RAM domains, stack/heap
  reservations, FPU/ABI flags, and programmer interface.
- M4 and M7 are distinct architecture profiles. The H745/H755 dual-core parts
  need an explicit selected core and per-core startup/link/memory configuration;
  a single generic M7 linker script is not enough.
- `system_init.c` must configure flash wait states before raising clocks, then
  verify the actual clock and configure MPU/cache regions before cacheable DMA
  buffers are used.
- Keep CMSIS/device headers and vendor startup sources in declared, versioned
  dependencies; do not copy undocumented local machine paths into the project.

### AVR / ATmega

- The compiler's AVR CRT normally owns reset entry and C runtime initialization.
  Do not emit a Cortex-M vector table or arbitrary linker script. Board-specific
  fuses and bootloader upload behavior are separate, explicit configuration.
- Resolve exact part flags, clock source/frequency, flash/SRAM/EEPROM limits,
  pin mapping, and `avrdude` programmer/port/baud settings from verified UETD
  chip and development-board records.
- Preserve a user-installed bootloader by default. Fuse changes and chip erase
  require a distinct, explicit operation and must never be part of ordinary
  `flash`.

### ESP32-WROOM-32 / ESP-IDF

- ESP-IDF owns startup, linker fragments, partition table, bootloader, compiler
  flags, image conversion, and most flashing command construction. Generate an
  IDF component/project adapter and pass UETD modules through IDF's component
  model rather than inventing a parallel bare-metal link path.
- Bind `ESP-WROOM-32` module flash size and board console/reset behavior to the
  selected development-board record. The bare ESP32 SoC does not contain the
  module's external flash.
- Keep partition layout, bootloader mode, and serial port selection explicit;
  never infer them from a board alias.

## Function-level implementation checklist

When moving the current placeholders into target-aware rendering, keep these
function boundaries visible in `project_scaffold.py` or a target adapter:

1. `render_target_scaffold(project, build_profile)` — choose only a supported
   startup/link strategy from the resolved board profile; return named files
   plus explicit manual actions for unsupported/incomplete profiles.
2. `render_system_init(project)` — render only sourced clock and memory setup
   inputs; keep unknown settings as TODOs and never synthesize register values.
3. `render_startup(project)` — consume a pinned vendor file or reviewed
   template; verify vector/core/ABI assumptions and declare how it enters build.
4. `render_linker_script(project)` — consume verified memory regions and reserve
   bootloader, secure, dual-core, DMA, stack, and heap ranges explicitly.
5. `validate_link_plan(project, files)` — check every referenced source/include,
   linker path, memory range, entry symbol, and output format before publishing.

Do not mark a board generation-ready until each generated build file has a
traceable UETD record or pinned tool/package source and the matching toolchain
can consume it.
