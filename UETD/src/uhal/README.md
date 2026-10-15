# UHAL target implementation map

UHAL is the boundary between portable UETD modules and a concrete processor.
`UETD_TARGET` selects exactly one implementation at CMake configure time. The
only implemented selections today are `HOST` (`x86/`) and `CORTEX_M` (`arm_cm/`).
The `avr/` and `esp_idf/` directories are implementation work areas, not
buildable targets yet.

## Existing implementation files

Each backend is organized around the same public UHAL headers:

| Source file | Public contract | Work to complete |
| --- | --- | --- |
| `atomic.c` | `atomic.h` | Choose native atomics or a documented critical-section fallback; preserve memory-order and ISR guarantees. |
| `cache.c` | `cache.h` | Implement cache-line alignment/rounding, barriers, and capability-safe no-op behavior. |
| `clock.c` | `clock.h` | Read core/bus clocks; enable/reset peripherals without disturbing unrelated RCC state. |
| `core.c` | `core.h` | Implement core identity, reset, cycle counter, and monotonic cycle extension. |
| `fault.c` | `fault.h` | Capture the fault frame and status registers into a bounded, persistent diagnostic. |
| `gpio.c` | `gpio.h` | Validate pin encoding and configure/write/read without changing unowned pin state. |
| `irq.c` | `irq.h` | Validate interrupt numbers and priority grouping; keep peripheral source flags separate from NVIC pending state. |
| `memory.c` | `memory.h` | Configure MPU regions and backup storage only when that hardware capability exists. |
| `timing.c` | `ucore/time.h` | Provide monotonic microseconds and bounded delay behavior from a verified clock source. |

The files in `arm_cm/` map these contracts to CMSIS. The files in `x86/` are
host simulation support; simulated GPIO, timing, backup registers, and faults
are not evidence of embedded behavior. Family profile files under
`targets/<family>/profile.c` are descriptive capability records and do not
replace a real register-level backend.

## AVR implementation scaffold

Start in `avr/` using the shared file map above. Before implementing those
functions, resolve these public-contract mismatches:

1. `target.h` currently has a Cortex-M CMSIS branch and a generic host fallback.
   Add a target-specific device-header boundary; AVR code must include the exact
   device header selected by the toolchain profile, never a guessed MCU header.
2. `irq.h` currently exposes CMSIS `IRQn_Type`, PRIMASK operations, and
   BASEPRI operations. AVR has different interrupt-vector, global-mask, and
   priority behavior. Replace the architecture-specific public details with an
   architecture-neutral IRQ identifier and critical-section API, then keep any
   architecture-specific priority API explicitly capability-gated.
3. Define atomic width/alignment guarantees for 8-bit AVR. A 32-bit operation
   cannot be treated as naturally atomic; specify whether each operation uses
   an interrupt-masked critical section and which contexts may call it.
4. Define the timer/counter source, overflow extension, and clock-change policy
   for `uhal_time_now_us()` before implementing deadlines or delays.
5. Keep reset-cause, MPU, cache, and cycle-count functions fail-closed or
   capability-reported when the selected AVR part has no equivalent hardware.

The corresponding `avr/*.c.todo` and `esp_idf/*.c.todo` files now hold the
public function headings next to the architecture-specific TODO; copy/rename a
file to `.c` only when its contract can be compiled and implemented. The
intended work files are `avr/atomic.c`, `cache.c`, `clock.c`, `core.c`,
`fault.c`, `gpio.c`, `irq.c`, `memory.c`, and `timing.c`. Put the exact public
function headings and implementation notes in each file before wiring this
directory into `CMakeLists.txt`.

## ESP-IDF / classic ESP32 implementation scaffold

Start in `esp_idf/` using the shared file map above. The first board is an
ESP-WROOM-32 module on an ESP32-DevKitC V4. Keep the SoC-specific UHAL
implementation separate from ESP-IDF board/project integration:

1. Use the ESP-IDF component/build-system contract for startup, linker scripts,
partition tables, bootloader, and image generation. Do not replace IDF-owned
artifacts with the generic Cortex-M `startup.s` or `link.ld` placeholders.
2. Record the exact ESP-IDF release, Xtensa compiler bundle, Python environment,
`idf.py`, and `esptool` pins in the UETD toolchain registry before marking a
board generation-ready.
3. Define interrupt allocation/dispatch and critical-section semantics using
the selected ESP-IDF release. The current CMSIS `IRQn_Type` and BASEPRI API
cannot be reused as-is.
4. Use ESP-IDF clock, GPIO, cache, timer, and reset services where they satisfy
the UETD contract. Document task/ISR restrictions and error propagation at each
wrapper boundary.
5. Model the ESP-WROOM module's flash and board-level USB-to-UART console in
the development-board record. The ESP32 SoC record must continue to describe
only on-chip resources.

The intended work files are `esp_idf/atomic.c`, `cache.c`, `clock.c`, `core.c`,
`fault.c`, `gpio.c`, `irq.c`, `memory.c`, and `timing.c`, plus an ESP-IDF
component/project adapter in `templates/targets/esp_idf/`. Add the selector to
the root CMake flow only after that adapter and all UHAL contracts have a
compile-ready implementation.

## Bring-up checklist for a new backend

- [ ] Add one architecture-specific source file for each public UHAL contract.
- [ ] Give every function either a verified implementation or a fail-closed
  result; do not return success for a no-op hardware operation.
- [ ] Document reset/clock prerequisites and the legal initialization order.
- [ ] Define interrupt priority, ISR-callable APIs, atomics, and memory barriers.
- [ ] Tie memory/cache behavior to verified chip capabilities and linker regions.
- [ ] Add an exact chip record and board binding in `registry/targets/` before
  enabling project generation for physical hardware.
- [ ] Add the backend to CMake only after its headers, sources, toolchain, startup
  path, and board profile agree.
