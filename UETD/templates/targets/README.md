# Target project templates

The files in this directory are Jinja source templates for target-owned build
files. Render them with `uef_gen.templates.render_uetd_template`; each template
requires the values listed in its architecture README. Missing values raise a
rendering error.

| Architecture | Template files | Runtime owner |
|---|---|---|
| Cortex-M | GNU toolchain, CMake executable, GNU assembler startup, GNU ld memory layout, system-init sequence, TrustZone two-image build, dual-core M7/M4 build | UETD startup and linker profile |
| AVR | CMake toolchain and objcopy-to-Intel-HEX avrdude flash target | AVR CRT and selected programmer |
| Arduino | Arduino `setup()` / `loop()` adapter | Selected Arduino core |
| ESP-IDF | Project/component CMake files and `app_main()` adapter | ESP-IDF |
| Nordic nRF52840 | Zephyr/NCS application scaffold | Selected Nordic/Zephyr runtime |
| RP2040 | Pico SDK project build | Pico SDK and board flash profile |
| i.MX RT1060 | MCUXpresso SDK application build | MCUXpresso SDK and board boot profile |

## Phase 0–2 target mapping

| Roadmap stage | Families/parts | Template and profile constraints |
| --- | --- | --- |
| 0 | STM32F4 F407; F7 F722/F746; G0 G0B1; G4 G474/G4A1; H7 H743; nRF52840 | Cortex-M GNU templates cover basic Cortex-M startup/linking. The separate H7 dual-core scaffold builds M7/M4 artifacts for catalogued H745/H755 parts, but core boot, memory, and ownership remain unresolved. Nordic's Zephyr scaffold is a separate runtime. |
| 1 | STM32L4+ L4R5; H7 SDMMC use case | Cortex-M templates only apply after the selected board binds the exact SDMMC instance, pins, DMA/cache policy, and clock. No L4R5 board is registered yet. |
| 2 | STM32H5 H563; STM32U5 U575; RP2040; i.MX RT1060 MIMXRT1062 | H5/U5 require separate secure/non-secure build and link profiles. RP2040 and RT1062 have external program flash; the exact board and boot profile supplies it. Pico is catalogued; no H563, U575, or RT1062 board record exists. |

The architecture notes under this directory define the required values and
render boundaries. Files not yet represented by a `.tmpl` are documented
contracts only; they must not be advertised as generated outputs.

The Cortex-M linker and startup templates require an exact verified memory map
and vector table. The dual-core template builds separate artifacts but does
not decide inter-core boot, shared-memory ownership, or image programming.
AVR uses the compiler CRT. Arduino and ESP-IDF retain their framework startup
and linker behavior. None of these templates enables target generation by
itself; the selected board and chip records remain the source of hardware facts
and readiness.

Phase 0–2 chip records now include sourced core capability snapshots,
reference-manual links, and partial memory-map entries where the base and
extent are established. Memory-map entries are catalogue facts only: they can
omit bank aliases, reservations, secure attribution, TCM assignments, DMA
visibility, or board-added external memory. They are not linker or flash
profiles. Exact pin/peripheral/DMA/IRQ/clock maps, startup/linker/errata
reviews, and board build/flash records are still incomplete. Keep
`generation_available` false until those source-backed inputs and the matching
render validators are complete.
