from pathlib import Path
import json
root = Path(__file__).resolve().parents[1]
texts = {
'UETD/src/uhal/README.md': '''# UHAL implementation map

Read [the porting and target pipeline guide](../../docs/design/UHAL_PORTING.md)
for the contracts, migration notes, exact implementation tasks and current limits.

| Scope | Directory | Status |
|---|---|---|
| CPU | `arch/cortex_m` | CPU primitives; private exact-device CMSIS boundary. Register-model tested. |
| CPU | `arch/avr` | Development headings, not selectable. |
| Platform | `platform/host` | Host simulation and nested thread-safe critical sections. |
| Platform | `platform/avr_atmega` | Exact AVR GPIO/clock/memory/time work area. |
| Platform/SDK | `platform/esp_idf` | ESP-IDF wrappers work area for supported ESP SoCs; not selectable. |
| Incomplete examples | `platform/cortex_m_scaffold` | GPIO, clock, memory and time TODOs; excluded from hardware builds. |
| Metadata | `targets` | Family capability records; not peripheral drivers. |

`cmake/Uhal.cmake` composes architecture and platform sources using `UETD_ARCH`
and `UETD_PLATFORM`. `CORTEX_M/CUSTOM` requires exact CMSIS/device headers and
`UETD_UHAL_PLATFORM_SOURCES`; CPU type alone cannot supply GPIO or a clock tree.
Unknown/unfinished architectures never fall back to simulation.

Physical targets are exact chips plus assembly, startup/build and programming
settings. A development-board record is an optional preset. UEF-PL's `hardware`
block can supply custom settings without a catalogue board. SDK templates live
under `templates/targets`; flash recipes and host tools are not CPU drivers.

Legacy module IDs `uhal/arm_cm` and `uhal/x86` remain stable registry aliases;
the actual source paths now describe architecture versus platform scope.
''',
'UETD/include/uhal/README.md': '''# UHAL public contracts

UHAL provides common CPU/platform interfaces without requiring vendor headers.
See [UHAL porting, target composition and the CLI/IDE pipeline](../../docs/design/UHAL_PORTING.md).
`api.json` declares UHAL contract revision 2; source/ABI migrations are listed in
that guide. Recompile consumers of the IRQ and fault structures.

| Header | Contract |
|---|---|
| `target.h` | Platform IRQ ID and compiler attributes; no CMSIS imports or fake intrinsics. |
| `target_config.h` | Chip/platform capability defaults; independent of a board preset. |
| `core.h` | Executing core index, optional CPU counters, reset and real backend barriers. |
| `irq.h` | Status-returning IRQ configuration and nested critical sections; optional priority-mask extension. |
| `atomic.h` | 32-bit atomic operations with documented memory ordering/context limits. |
| `cache.h` | Independently gated I/D caches; checked whole-line maintenance and platform placement attributes. |
| `clock.h` | Generic clock IDs, peripheral gates/reset and reset cause; platform owns domain mapping. |
| `gpio.h` | Platform pin tokens and configuration; mode/electrical capabilities vary by chip. |
| `fault.h` | Common fault addresses and an optional architecture-specific register snapshot. |
| `memory.h` | Platform memory/MPU and retained-storage boundary. |
| `platform/stm32.h` | Explicit STM32 clock IDs, kept out of the portable contract. |
| `target_profiles.h`, `targets/*.h` | Descriptive family profiles; no implied peripheral/board readiness. |
| `uhal.h` | Common umbrella. |

The monotonic microsecond API remains declared in `ucore/time.h` and is
implemented by the selected platform. It must not depend on a CPU counter that
can miss wraps or change frequency unnoticed. CPU architecture, chip peripherals,
SDK integration, physical assembly and flashing transport are separate choices.
''',
'UETD/src/uhal/arch/avr/README.md': '''# AVR architecture work area

Implement CPU-specific interrupt-state save/restore, barriers, atomic fallbacks
and reset/fault primitives here. The `.c.todo` files are headings, not sources.
The corresponding chip-specific GPIO, clocks, retained memory and monotonic
timer work belongs in `../../platform/avr_atmega/`.

Public headers now use `uhal_irq_t` and backend critical-section functions;
there is no CMSIS/PRIMASK fallback. An AVR implementation must save SREG, mask
interrupts briefly and restore the caller's state. Thirty-two-bit operations
need protection on an eight-bit CPU. AVR has no BASEPRI priority mask.

AVR-GCC/avr-libc startup and an optional Arduino core are separate runtime/build
choices. Arduino is not the architecture. See the full [porting guide](../../../../docs/design/UHAL_PORTING.md)
for MCU identity, timer ownership, memory budgets and programmer requirements.
''',
'UETD/src/uhal/platform/avr_atmega/README.md': '''# ATmega platform work area

Implement the exact part's DDR/PORT/PIN, clock/reset, interrupt-source routing,
retained-memory policy and monotonic timer here. AVR CPU operations live in
`../../arch/avr/`. Arduino runtime/build integration belongs to the Arduino
adapter, not this chip-family implementation. No physical development board is
required: the chip plus a custom assembly and programmer profile is sufficient
once its implementation and metadata are complete.

The `.c.todo` files are not compiled. See [the porting guide](../../../../docs/design/UHAL_PORTING.md).
''',
'UETD/src/uhal/platform/esp_idf/README.md': '''# ESP-IDF platform work area

This is an SDK/platform adapter, not an ISA. It can share supported API wrappers
across Xtensa and RISC-V ESP SoCs while the target record identifies the exact
CPU/SoC. Classic ESP32 is Xtensa LX6; ESP32-C3 is RISC-V. The `.c.todo` files are
unimplemented headings and are not selected by CMake or firmware generation.

The public contracts no longer require CMSIS. Implement them against a pinned
ESP-IDF release, including SMP-safe exclusion, allocated IRQ mappings,
monotonic time, reset/GPIO/clock services and truthful capability/error behavior.
Do not export Cortex-M exception handlers. ESP-IDF owns startup, bootloader,
linker fragments, partitions and its FreeRTOS kernel. UETD's vanilla FreeRTOS
adapter requires an IDF-specific integration, including stack-unit review.

Physical assembly settings can describe any bare SoC/module/custom PCB. A
DevKit record is only a preset. Record module/external flash and wiring with
that assembly, not as nonexistent on-chip SoC resources. Keep build/image and
esptool deployment adapters separate. See [the porting guide](../../../../docs/design/UHAL_PORTING.md).
''',
'UETD/src/uhal/platform/cortex_m_scaffold/README.md': '''# Incomplete peripheral examples

These sources contain TODOs for an exact SoC/platform. They are intentionally
excluded from `CORTEX_M/CUSTOM` builds; they must not make a hardware library
appear implemented. CPU primitives live under `../../arch/cortex_m/`.

A platform supplies clock queries/gates/reset, GPIO mapping, memory and retained
storage, CPU frequency, an optional extended cycle counter and the monotonic
microsecond clock/delays. Startup, linker layout, chip capabilities and the
physical assembly must agree. See [the porting guide](../../../../docs/design/UHAL_PORTING.md).
''',
}
for name, content in texts.items():
    (root / name).write_text(content, encoding='utf-8', newline='\n')

p = root / 'UETD/include/uhal/fault.h'
s = p.read_text(encoding='utf-8').replace('uintptr_t pc;', 'uetd_u64_t pc;').replace('uintptr_t sp;', 'uetd_u64_t sp;').replace('uintptr_t fault_address;', 'uetd_u64_t fault_address;')
s = s.replace('typedef struct {', '/* Fixed-width addresses also represent program addresses wider than a data\n * pointer on Harvard MCUs. The platform defines their address-space meaning. */\ntypedef struct {', 1)
p.write_text(s, encoding='utf-8', newline='\n')

for name in ['UETD/registry/targets/development_boards/record.schema.json', 'uef-gen/src/uef_gen/schemas/project.schema.json']:
    p = root / name
    data = json.loads(p.read_text(encoding='utf-8'))
    props = data['properties'] if name.startswith('UETD') else data['properties']['target']['properties']['hardware']['properties']
    props['memory_regions']['additionalProperties']['properties']['address_space'] = {'type': 'string', 'minLength': 1, 'description': 'Explicit address space, e.g. program/data for Harvard targets; defaults to unified.'}
    p.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8', newline='\n')

p = root / 'uef-gen/src/uef_gen/language/registry/registry.json'
data = json.loads(p.read_text(encoding='utf-8'))
data['registry_version'] = '1.4'
data['language']['version'] = '14'
data['grammar']['productions']['project_decl'] = data['grammar']['productions']['project_decl'].replace('numeric_decl?', 'numeric_decl? hardware_decl?')
data['grammar']['productions']['declaration'] = data['grammar']['productions']['declaration'].replace('target_decl |', 'target_decl | hardware_decl |')
data['constructs']['project']['syntax'] = data['constructs']['project']['syntax'].replace('numeric?;', 'numeric?; hardware?;')
p.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8', newline='\n')
p = root / 'uef-gen/editors/vscode/syntaxes/uef.tmLanguage.json'
p.write_text(p.read_text(encoding='utf-8').replace('routine|target|numeric|', 'routine|target|numeric|hardware|'), encoding='utf-8', newline='\n')

for name in ['UETD/README.md', 'README.md', 'UETD/registry/targets/README.md', 'uef-gen/README.md']:
    p = root / name
    s = p.read_text(encoding='utf-8')
    link = ('docs/design/UHAL_PORTING.md' if name == 'UETD/README.md' else
            '../../docs/design/UHAL_PORTING.md' if name == 'UETD/registry/targets/README.md' else
            '../UETD/docs/design/UHAL_PORTING.md' if name == 'uef-gen/README.md' else 'UETD/docs/design/UHAL_PORTING.md')
    notice = f'\nCPU architecture, SoC/SDK implementation, physical assembly and programming transport\nare independent. Development boards are optional presets; UEF-PL `hardware`\nsettings can describe bare chips and custom PCBs. See the [UHAL and target\npipeline guide]({link}) for implementation status and migration details.\n'
    if notice not in s:
        pos = s.find('\n')
        s = s[:pos+1] + notice + s[pos+1:]
        p.write_text(s, encoding='utf-8', newline='\n')
print('Updated UHAL porting docs, generic target schema, language grammar and editor highlighting.')
