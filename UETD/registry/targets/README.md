# Registering chips and development boards

The chip and board indexes are the only target-data sources used by `uef-gen`.
`api.json` points to both indexes and their schemas. Each index entry is
only `{ "id", "path" }`; the facts live in one JSON file beside the index's
`manifest/` tree. Add a family record only when the silicon family is new.

## 1. Register the exact chip

Create `chips/manifest/<vendor>/<part-id>.json` and add its path to
`chips/registry.json`. Use the manufacturer's exact ordering code as
`part_number`; `id` is its lowercase registry key. Point `family_id` to an
existing family record. Keep the first record small: identity, core, package,
headline memory, source links, fields checked against those sources, and
`generation_available: false` are enough to catalog the part.

The STM32F722RET6 chip record already exists at
`chips/manifest/stmicroelectronics/stm32f722ret6.json`. Its current pin and
resource maps are intentionally incomplete, so it remains catalogued but
unavailable for generation. The ESP32-D0WDQ6-V3 record is at
`chips/manifest/espressif/esp32-d0wdq6-v3.json`; its `flash_bytes: 0` reflects
the bare SoC having no in-package flash. Flash on an ESP-WROOM module belongs
to the board record in the next step.

The starter index includes exact ATmega328P-PU, ATmega328P-32MUR, ATmega2560-16AU,
ATmega32U4-AU, and ATmega4809-AFR records for common Arduino boards; STM32F103C8T6
for the widely used STM32F1 line; STM32H743ZIT6, STM32H745ZIT6, and STM32H755ZIT6
for H7 boards; RP2350A for the newer Pico 2; and nRF9160-SICA-B1A-R for the
Phase 3 nRF9160 target. The family catalogue includes the corresponding
`megaavr-8bit` family and a catalogue-only `rp2350` family with no assigned
roadmap phase. These records carry sourced identity, core, package, headline
memory, and peripheral-group facts; none includes a complete generation-ready
resource map. No generic Blue Pill
board is registered because third-party revisions do not identify one uniform
pin, clock, or programming configuration.

Add verified resource details to the *same chip JSON* as you work through the
part: architecture word width/capabilities, peripheral types and instances, package pin
and alternate-function maps, clock/reset domains, DMA requests, interrupts,
memory regions, startup/linker data, and errata review. The schema requires
these sections before a physical chip can be marked generation-ready.

## 2. Register the development board

Create `development_boards/manifest/<vendor>/<board-id>.json` and add one
reference to `development_boards/registry.json`. `chip_id` links to the exact
chip's registry `id`; the board file owns oscillator sources, board-reserved
pins, any board-added memory regions, build backend settings, and
flash/programmer settings. Store module
details inline under `module`—there is no hardware `modules/` registry to
maintain. The `registry/modules/` directory elsewhere is for UETD software
modules such as UMATH and UCON.

When a SoC record declares no in-package flash, the resolver uses an inline
module `flash_bytes` value as the assembled target's flash capacity. The chip
record remains an accurate description of the silicon.

The registry includes an ESP32-DevKitC V4 binding with ESP-WROOM-32 module
facts and an STM32F722RET6 carrier placeholder. Both are not-ready records;
verify the actual model/revision you own, then fill sourced board facts as you
verify them. A custom WROOM carrier can use this minimal shape:

```json
{
  "id": "your-wroom-board-id",
  "name": "Manufacturer board model, revision",
  "chip_id": "esp32-d0wdq6-v3",
  "module": {
    "name": "ESP-WROOM-32"
  },
  "clock_sources": {},
  "reserved_pins": [],
  "generation_available": false
}
```

Add this one index reference, using the same ID:

```json
{
  "id": "your-wroom-board-id",
  "path": "registry/targets/development_boards/manifest/espressif/your-wroom-board-id.json"
}
```

Use the physical board model and hardware revision in `name`. For an
ESP-WROOM-32 setup, reference `esp32-d0wdq6-v3` and record WROOM flash, crystal,
antenna, and exposed-pin details from the module datasheet; record carrier
board clocks, USB-to-UART/programmer, and reserved pins from that board's
schematic. For an STM32F722RET6 setup, reference `stm32f722ret6` and record the
board's clock sources, debug/programming interface, and pin reservations from
its schematic. If a module sits on a separate carrier, the board record should
describe the actual assembled carrier you plan to generate and flash.

Other seeded board bindings include Arduino UNO R3, Mega 2560 Rev3, Leonardo,
Nano Classic, and Nano Every; Raspberry Pi Pico and Pico 2 (non-W); ST NUCLEO-G474RE,
NUCLEO-F746ZG, NUCLEO-H743ZI2, NUCLEO-H745ZI-Q, and NUCLEO-H755ZI-Q; and the
Nordic nRF9160 DK. They document chip links and sourced board identity, but
remain fail-closed until exact revisions, clock sources, reserved pins, build
settings, and flash interfaces are reviewed. ST currently lists the H745 Nucleo
as out of production; the H755 Nucleo is the active dual-core counterpart.

Start each board with `generation_available: false`. Add `build` and `flash`
objects from the board's build and programming toolchain documentation. Each
must name both a `backend` adapter and a `toolchain` package ID from
`registry/targets/toolchains/registry.json`; `dependencies` lists additional
UETD package IDs needed for that board/profile (for example an Arduino core or
STM32Cube device package); and `settings` hold exact board-level
arguments such as serial interface, output format, memory offsets, reset-line
policy, or programmer serial. Only mark both the chip and board ready after
the chip maps and board-specific settings are complete and reviewed. A project
can then select the board alone:

```yaml
target:
  development_board: your-board-id
```

`uef-gen` follows that board's `chip_id` into the chip registry. It applies
board clock sources and reserved pins while resolving the chip's clock and pin
maps, and includes board-added memory regions in the effective target. If the
SoC has no in-package flash, the inline module capacity supplies the target
flash limit. It checks the chip/board readiness flags and reads build/flash
profiles from UETD. Project YAML keeps application choices and board selection;
it is not a second target database.
