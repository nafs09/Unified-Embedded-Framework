# UHAL public contracts

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
| `stm32.h` | Explicit STM32 clock IDs, kept out of the portable contract. |
| `target_profiles.h`, `targets/*.h` | Descriptive family profiles; no implied peripheral/board readiness. |
| `uhal.h` | Common umbrella. |

The monotonic microsecond API remains declared in `ucore/time.h` and is
implemented by the selected platform. It must not depend on a CPU counter that
can miss wraps or change frequency unnoticed. CPU architecture, chip peripherals,
SDK integration, physical assembly and flashing transport are separate choices.
