# UHAL architecture, platform and target integration

UHAL has a common C contract with implementations selected from independent CPU
and platform layers. A development board is an optional configuration preset.
Bare chips, modules on custom PCBs and commercial development boards use the
same target-resolution path. The physical assembly and programming connection
do not change the CPU instruction set.

## Which information belongs where?

| Layer | Owns | Examples |
|---|---|---|
| CPU architecture | Instruction barriers, local interrupt masking, core exception mechanics, available counter instructions | Cortex-M0/M4/M7, AVR, Xtensa LX6, RV32 |
| SoC/family platform | GPIO, clock/reset tree, peripheral instances, pin mux, DMA routing, interrupt-source mapping, memory controllers | STM32G4, STM32H7, classic ESP32, ATmega328P |
| SDK/runtime integration | Startup owner, driver API adapters, linker/build conventions, RTOS ownership | Bare-metal CMSIS/device headers, ESP-IDF, Arduino AVR core |
| Physical assembly | External crystal/flash/RAM, wiring, reserved pins, bootloader reservations | Bare chip with an SWD header, a custom PCB, a WROOM module, an optional DevKit preset |
| Host tools/deployment | Pinned compiler/flasher packages, programmer transport, artifact segments and addresses | Arm GNU + ST-LINK/OpenOCD, IDF + esptool, AVR-GCC + avrdude |

Two chips with the same Cortex-M4 or RV32 core can have entirely different
peripherals. Conversely, the ESP-IDF API can wrap both Xtensa and RISC-V ESP
SoCs. Arduino is a framework/board ecosystem, not a CPU architecture.
CMSIS standardizes core access and the device-header boundary; it does not
provide one GPIO or RCC implementation for every Cortex-M MCU.

## Source organisation and current implementation

```text
include/uhal/                   architecture-neutral public contracts
include/uhal/stm32.h           explicitly STM32-specific identifiers
src/uhal/arch/cortex_m/         CPU primitives and private CMSIS inclusion
src/uhal/arch/avr/              AVR CPU work area (.todo, not selectable)
src/uhal/platform/host/         host simulation (independent of host CPU ISA)
src/uhal/platform/avr_atmega/   AVR chip-family work area (.todo)
src/uhal/platform/esp_idf/      SDK-backed ESP platform work area (.todo)
src/uhal/platform/cortex_m_scaffold/  incomplete peripheral examples, never selected
src/uhal/targets/              descriptive family profiles, not drivers
templates/targets/            build/startup adapters, separate from CPU code
registry/targets/             exact chips, optional boards and host tool packages
```

Implemented and tested on the host: host simulation and nested, thread-safe
critical sections. CPU code has register-model tests for Cortex-M interrupt
mask restoration, IRQ validation, atomics, barriers and cache operations.
Instruction-cache support is independent of data-cache support.

Cortex-M clock, GPIO, memory and time functions require an exact platform
implementation. The old generic no-op sources are excluded from normal
hardware builds. AVR and ESP-IDF remain explicitly unavailable until their
platform and SDK integration exists. Family profiles and a successful static
library build are not proof that a chip can boot or be flashed.

The legacy module IDs `uhal/x86` and `uhal/arm_cm` remain stable registry aliases.
Their source directories now express the actual scopes. New AVR/ESP-IDF
module records are discoverable but cannot be selected for generated firmware.
Unknown architectures never default to the host simulator.

## Shared contracts and migration

UHAL contract revision 2 is declared in `api.json`. Recompile consumers.

- Replace public `IRQn_Type` fields with `uhal_irq_t`. CMSIS types and device
  headers are private to the Cortex-M implementation. Interrupt configuration
  returns `uetd_status_t`; a host cannot claim it enabled a physical IRQ.
- Critical sections are backend functions with a saved-state token. Restore
  nested entries in LIFO order in the same execution context. SMP ports need
  both local interrupt control and a shared lock. Do not block inside them.
- BASEPRI is an optional extension gated by `UHAL_HAS_IRQ_PRIORITY_MASK`.
  Cortex-M0/M0+ and AVR cannot supply it. The default Cortex-M critical section
  is single-core and rejects a multicore capability configuration.
- The fault context carries common addresses plus an architecture-specific
  snapshot. Cortex exception-handler symbols are not public portable APIs.
  Current Cortex handlers fail-stop; full frame capture/persistence still
  requires implementation. SDK panic handlers retain ownership.
- `uhal_core_id()` means the executing core's zero-based index, not a silicon
  serial number or CPU model. CPU frequency is distinct from a timer frequency.
- `uetd_time_now_us()` is the platform's monotonic time API. A 32-bit cycle
  counter with occasional reads cannot guarantee a 64-bit monotonic timebase:
  multiple wraps, sleep and clock changes must be handled explicitly.
- GPIO `port` is an opaque platform token and `pin` a platform line index.
  Generated bindings hide whether a chip uses GPIOA/pin 5 or GPIO number 21.
- Common clock IDs are CPU, system and monotonic. STM32 APB/PLL identifiers
  and STM32-oriented TCM/CCM/AXI memory-region IDs moved to
  `uhal/stm32.h`; other platforms define their real domains.
- Cache maintenance returns status and requires aligned, whole-line buffers.
  Invalidating partial dirty lines can lose neighbouring data. A `.dma_buffers`
  section name alone does not establish non-cacheability; the linker and memory
  attributes must agree. Callers must handle cache-operation errors.

## Generic target configuration in UEF-PL

The target selects an exact registered chip, or an optional board that supplies
that chip. The `hardware` block adds literal physical/build/deployment settings:

```uef
project CustomController {
  target = "stm32g474ret6";
  hardware {
    build.backend = cmake;
    build.toolchain = "arm-none-eabi";
    build.settings.platform = "stm32g4";
    clock_sources.hse = 8000000;
    reserved_pins = ["PA13", "PA14"];
    flash.backend = openocd;
    flash.toolchain = openocd;
    flash.interface = swd;
    // Add the exact probe/target/boot-layout settings required by that adapter.
  }
  // Application signatures, operatingsystem and implementation follow.
}
```

This is a target-configuration excerpt, not a claim that the STM32G4 hardware
port or CLI flash runner is ready. Frequencies and probe settings must describe
the actual assembly. Hardware properties accept literal values, identifiers,
lists and `map(...)` records; runtime expressions are rejected.

Board presets supply defaults. Explicit hardware settings override them;
reserved preset pins are retained. Changing a build/flash backend replaces its
whole recipe so that settings from another tool cannot leak into it. A custom
assembly can add external memory, but cannot replace or overlap on-chip regions
in the same address space. Separate program/data spaces can be declared for
Harvard architectures.

The resolver carries the resulting hardware profile to project scaffolding,
dependency selection and generated manifests. Build and flash profiles remain
separate. Compilation needs a build profile; a flash recipe is required when
flashing. No project flag can make an incomplete chip, SDK port or tool package
generation-ready. A missing fact produces a diagnostic about that fact, rather
than a demand to purchase or register a development board.

An exact chip can declare approved implementations in `resources.uhal`:

```json
{
  "default_platform": "stm32g4",
  "platforms": {
    "stm32g4": {
      "architecture": "CORTEX_M",
      "module": "uhal/stm32g4",
      "build_backend": "cmake"
    }
  }
}
```

This illustrates the binding shape; it does not register an implemented G4
module. `build.settings.platform` selects one of that chip's approved entries.
The generator verifies CPU and SDK/build compatibility and resolves the module
from the canonical registry. Its normal readiness checks still apply. A new
chip/platform therefore needs data and implementation, rather than another
chip-name branch in the generator. Core count is preserved from chip records
into resolved data and generated metadata; a multicore chip must not silently
use single-core exclusion.

For direct CMake integration, choose `UETD_ARCH` and `UETD_PLATFORM`.
`HOST/HOST` builds simulation. `CORTEX_M/CUSTOM` additionally requires exact
CMSIS include paths/header, capability definitions and absolute
`UETD_UHAL_PLATFORM_SOURCES`. Those sources supply CPU frequency, extended
counter policy, clocks, GPIO, memory and `uetd_time_now_us`/delays. A missing
platform stops configuration. Legacy `UETD_TARGET` remains an input alias.

## What each real target still needs

### ESP, including Xtensa and RISC-V variants

1. Bind an exact SoC to the pinned ESP-IDF release and matching compiler bundle.
   Put shared IDF wrappers in the platform adapter; split CPU-specific operations
   only when SDK services do not hide the difference.
2. Implement GPIO, clock/reset and IRQ allocation mappings using that release's
   supported APIs. Preserve task/ISR restrictions and propagate errors.
3. Provide SMP-safe atomics and critical sections. Disabling local interrupts
   alone does not exclude the other core. Define core affinity and counter scope.
4. Adapt `esp_timer_get_time()` to monotonic microseconds. Define light/deep-sleep
   and clock-scaling behavior; do not derive deadlines from a variable CPU clock.
5. Use IDF-owned startup, linker fragments, partitions, bootloader and application
   images. Record external flash capacity with the physical assembly, not a bare
   SoC with no in-package flash.
6. Adapt UOS to the IDF-owned kernel. Do not compile UETD's bundled FreeRTOS
   kernel into the same image. Check stack-size units and scheduler ownership;
   the existing vanilla-FreeRTOS wrapper cannot be copied without review.
7. Implement the IDF build runner and segment-aware esptool deployment adapter.
   Offsets come from IDF's output metadata and the selected partition layout.

Classic ESP32 is Xtensa LX6; ESP32-C3 is RISC-V. The SDK/platform can remain
shared while those architecture records differ.

### STM32, on any compatible PCB

1. Reuse Cortex-M CPU primitives, but implement the exact family/platform's RCC,
   GPIO/alternate functions, reset causes, backup memory and peripheral maps.
   STM32F4/G4/H7 are not one interchangeable register map.
2. Bind the exact package/part, interrupt count, clock inputs, pin mux and DMA
   routes. Choose vendor HAL/LL or direct device-register access deliberately;
   this choice is independent of the board and CPU core.
3. Supply startup/vector ownership, memory map, flash latency and oscillator/PLL
   configuration. Configure MPU attributes before enabling relevant caches.
   Dual-core and TrustZone parts require explicit image/resource ownership.
4. Provide a verified timer, UART console and the UPAL drivers needed by the
   application. A Cortex-M core port does not implement UART/SPI/I2C/ADC/PWM.
5. Bind an SWD/JTAG or bootloader transport and exact flash geometry to a pinned
   programmer. A custom PCB with a probe header needs no development-board ID.

### AVR and Arduino

UNO R3 uses ATmega328P, Mega 2560 uses ATmega2560, and Leonardo uses ATmega32U4.
ATmega32 is a different part. Arduino support must state the exact board/core
recipe when using that framework; a bare ATmega can use AVR-GCC/avr-libc directly.

1. Implement AVR saved-SREG interrupt masking and protected 32-bit atomics.
   An 8-bit load/store machine does not make a 32-bit counter atomic.
2. Implement the exact chip's DDR/PORT/PIN, clock/reset and interrupt-source maps.
   Define timer ownership and overflow extension for monotonic microseconds.
3. In an Arduino integration, preserve its CRT, `setup()`/`loop()` and timer
   ownership. The classic AVR core uses Timer0 for timekeeping; do not claim it
   twice. A `micros()` wrapper still needs wrap handling and a sampling policy.
4. Gate absent DMA/cache/MPU/priority features. Audit flash/SRAM, static object
   sizes, stack use, floating-point/64-bit cost and Harvard address spaces.
   ATmega328P has 2 KiB SRAM; UOS currently defaults to 1024 bytes of task control
   storage before allocating a stack. Start with a small bare-metal subset.
5. Pin AVR-GCC/avr-libc or the Arduino core/recipes, and avrdude. Encode exact
   MCU/signature, programmer protocol and application/bootloader ranges.

## UEF-PL to IDE/CLI deployment

The intended single pipeline is:

```text
.uef -> parse/type-check -> resolve chip + hardware + modules
     -> generate sources/build/target manifests -> build -> test
     -> validate image segments and target identity -> program -> verify
```

The VS integration should invoke this CLI pipeline and consume its structured
diagnostics; it must not maintain a second hardware database or flasher recipe.
Architecture cannot choose a programmer. A chip may be flashed through SWD,
UART, USB DFU, ISP or another supported transport depending on its assembly.

Target resolution and metadata propagation are implemented here. The existing
build/deploy CLI runners, tool-package bootstrap and hardware deployment
backends still contain explicit development scaffolds. Completing those and
exact chip/SDK/UPAL ports is required before the whole physical workflow works.
Do not substitute unconditional success or enable catalogue readiness flags.
Host tests and register models do not replace cross-compilation and hardware
verification.

Run focused checks with `UETD_BUILD_UHAL_TESTS=ON`, then CTest. The Python
`test_uhal_targets.py` tests custom targets, UEF-PL lowering, SDK/ISA selection,
memory constraints and manifest propagation. `UETD_BUILD_TESTS=ON` also runs the
framework's existing contract suites.

## Vendor references

- [CMSIS-Core scope](https://arm-software.github.io/CMSIS_6/latest/Core/index.html)
- [ESP32 architecture](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html)
- [ESP32-C3 architecture](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/get-started/index.html)
- [ESP-IDF SMP and stack conventions](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos_idf.html)
- [ESP timer semantics](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html)
- [Arduino UNO R3](https://docs.arduino.cc/hardware/uno-rev3), [Mega](https://docs.arduino.cc/hardware/mega-2560), [Leonardo](https://docs.arduino.cc/hardware/leonardo)
- [Arduino AVR timekeeping implementation](https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/wiring.c)
- [ATmega328P memory/resources](https://www.microchip.com/en-us/product/atmega328p)
