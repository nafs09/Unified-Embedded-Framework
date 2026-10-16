# Registering chips and development boards

The chip and board indexes are the only target-data sources used by `uef-gen`.
`api.json` points to both indexes and their schemas. Each index entry is
only `{ "id", "path" }`; the facts live in one JSON file beside the index's
`manifest/` tree. Add a family record only when the silicon family is new.

## 1. Register the exact chip

Create `chips/manifest/<vendor>/<part-id>.json` and add its path to
`chips/registry.json`. Use the manufacturer's exact ordering code as
`part_number`; `id` is its lowercase registry key. Point `family_id` to an
existing family record. Keep the first record small: identity, core ISA and
word width, package, headline memory, source links, fields checked against
those sources, and `generation_available: false` are enough to catalog the
part.

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
roadmap phase. Exact chip records now include an explicit ISA and word width in
addition to sourced identity, core, package, headline memory, and peripheral
groups. These are catalogue facts, not full target resource maps: none is
generation-ready. No generic Blue Pill
board is registered because third-party revisions do not identify one uniform
pin, clock, or programming configuration.

Add verified resource details to the *same chip JSON* as you work through the
part: architecture word width/capabilities, peripheral types and instances, package pin
and alternate-function maps, clock/reset domains, DMA requests, interrupts,
memory regions, startup/linker data, and errata review. The schema requires
these sections before a physical chip can be marked generation-ready.
The 18 exact parts currently assigned to Phase 0, 1, or 2 now carry sourced
architecture capability snapshots and reference manuals. Their
`resources.memory_regions` are intentionally partial: only established bases
and extents are recorded, with bank aliases, boot reservations, TrustZone
attribution, cache/DMA policy, external memories, or configurable FlexRAM
omitted where they need a board/profile decision. They are discovery data, not
linker inputs. In particular, the RT1062's empty region map records that its
RAM partition and external boot memory have not been resolved.

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

The registry includes board-level facts where the manufacturer's documents
identify them. For example, the ESP32-DevKitC V4 record captures the selected
ESP-WROOM-32 flash, crystal, USB-UART connection, and flash-reserved GPIOs; the
Pico records include external-flash XIP windows and board-reserved GPIOs; and
the Nucleo records list documented clock sources and on-board debug devices.
These records remain unavailable for generation until the exact board revision,
active solder-bridge configuration, remaining pin constraints, and build/flash
settings are reviewed. The STM32F722RET6 carrier remains a selection placeholder.
A custom WROOM carrier can use this minimal shape:

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
Nordic nRF9160 DK. Board records use `source_verified_fields` to identify which
facts were checked against the cited documents and `onboard_devices` to record
secondary controllers such as USB bridges and debug probes. `clock_sources`
can list both defaults and documented alternatives; it does not select the
active solder-bridge or firmware configuration. `reserved_pins` is for pins
that the selected board profile makes unavailable to application GPIO; shared
or switchable functions need their exact board revision and bridge state before
they can be resolved. Remaining revision-specific routing, build settings, and
flash interfaces are still generation blockers. ST currently lists the H745
Nucleo as out of production; the H755 Nucleo is its active dual-core counterpart.

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
