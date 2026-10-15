# AVR / ATmega target integration template scaffold

Use this path for an eventual UETD-owned AVR adapter. The exact ATmega part and
Arduino board are separate records: the chip owns flash/SRAM/EEPROM and
peripheral facts; the development board owns clock, reserved pins, bootloader,
USB interface, and programmer selection.

## Generated files to design

- `Makefile` or CMake toolchain fragment: exact `avr-gcc` device flag,
  optimization/section policy, MCU macro, include/library list, and link
  options. Do not invent a generic `link.ld`; the compiler CRT and linker
  scripts for the selected MCU own startup sections.
- `target/board_config.h`: verified F_CPU, pin mapping, timer ownership,
  serial upload settings, and board revision.
- `manifest/target_build.json`: chip/board registry IDs, package IDs/versions,
  compiler flags, output image format, flash/SRAM budgets, and artifact range.
- `flash_profile.json` in generated provenance: programmer, transport, port
  selection policy, baud/settings, bootloader range, and verification method.

TODO(avr-template): make bootloader preservation the default. Fuse writes,
bootloader replacement, chip erase, and recovery require a separate explicit
operation. Generation must not silently select pins that the board reserves
for USB, serial, clock, reset, or onboard peripherals.
