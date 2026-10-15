# Arduino CLI target adapter scaffold

An Arduino development board record still binds to an exact ATmega part.
Arduino CLI is a build/upload adapter; the Arduino board core supplies the
variant, startup/CRT recipes, pin mapping, compile flags, bootloader recipe,
and tool package dependencies. Do not replace the UETD chip and board registry
with Arduino FQBN strings.

## Registry inputs

- `build.backend`: the uef-gen adapter ID, `arduino-cli`.
- `build.toolchain`: UETD package ID `arduino-cli`.
- `build.dependencies`: the board's exact core package ID; for example
  `arduino-avr-core` for Uno/Mega/Leonardo/Nano Classic or
  `arduino-megaavr-core` for Nano Every.
- `build.settings`: exact FQBN, core/variant, menu selections, serial upload
  recipe, output directory policy, and extra properties reviewed against the
  board record.
- `flash.backend` and `flash.toolchain`: `avrdude` for bootloader/programmer
  paths that uef-gen owns directly; `flash.settings` gives exact programmer,
  port/baud, signature, application/bootloader boundaries, and reset wiring.

## Generated files and function seams

1. `render_arduino_project(project, profile)` — emit sketch/component files and
   a deterministic Arduino CLI invocation from the selected core/board profile.
2. `resolve_arduino_core(profile, package_registry)` — select only the
   UETD-pinned core version and local package root; do not use global CLI indexes
   or implicit network downloads.
3. `parse_arduino_build_output(stdout, stderr, build_dir)` — capture canonical
   source/include/define/library lists and every produced HEX/ELF/map image;
   reject missing or ambiguous output files.
4. `extract_upload_ranges(images, board_profile)` — map application and
   bootloader images to exact flash ranges and protect the bootloader by
   default.
5. `upload_with_avrdude(artifact, profile)` — program only the selected
   application image; never pass fuse, EEPROM, lock-bit, or chip-erase options
   during ordinary upload.

## Board-family split

UNO R3, Mega 2560, Leonardo, and Nano Classic use the Arduino AVR core but have
different FQBNs, pin maps, bootloaders, flash boundaries, and upload protocols.
Nano Every uses the megaAVR core and ATmega4809; it is not the same build recipe
as classic ATmega328P AVR-GCC. Keep these differences in each UETD board profile.

TODO(arduino-cli-adapter): implement the adapter under `uef-gen`, add its
validated project/output model, and only then mark board build profiles ready.
