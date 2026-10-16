# AVR templates

`toolchain.cmake.tmpl` selects an AVR GCC executable, exact MCU, and CPU clock.
Render it with `mcu`, `compiler`, and `clock_hz`.

`flash.cmake.tmpl` adds an explicit `flash` target using avrdude. Render it
with `avrdude_name`, `objcopy_name`, `mcu`, `programmer`, `port`, `baud`, and
the CMake executable `target`. An empty `port` or `baud` omits that option.
The template uses the selected objcopy tool to convert the target ELF into an
Intel HEX file, then uses avrdude's no-auto-erase mode to program it. It does
not change fuses or issue chip erase. The selected application image and
programmer configuration must exclude any bootloader reservation. If
programming needs erased bits, use a separately reviewed erase and programming
operation.

AVR CRT owns reset and runtime initialization. These templates do not emit a
vector table or linker script.
