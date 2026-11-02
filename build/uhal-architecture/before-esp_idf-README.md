# ESP-IDF UHAL implementation scaffold

This folder is a write-in location for classic ESP32 UHAL wrappers. CMake does
not select it. The first board is an ESP-WROOM-32 on a DevKitC V4, but the UHAL
implementation should describe the ESP32 SoC while the development-board JSON
owns module flash, crystal, antenna, USB-to-UART, and reserved pins.

ESP-IDF owns reset/startup, bootloader, partition/linker fragments, much of the
clock configuration, the Xtensa compiler bundle, and `esptool` integration.
Implement the UETD component/build adapter separately; do not add a standalone
Cortex-M startup file or guessed linker map. The current CMSIS IRQ, PRIMASK,
and BASEPRI headers need an architecture-neutral contract before these source
files can compile correctly.

Each `*.c.todo` file beside this README contains the public function headings
for that source. Copy/rename it to `.c` after the shared API contract is
refactored. Preserve UETD function signatures where they are architecture-
neutral. If ESP-IDF's handle or error model differs, define a reviewed adapter
boundary and map its failures to UETD status/diagnostics.

| Source file | Required function headings | Implementation TODO checklist |
| --- | --- | --- |
| `atomic.c` | `uhal_atomic_cas_u32`, `uhal_atomic_fetch_add_u32`, `uhal_atomic_fetch_and_u32`, `uhal_atomic_fetch_or_u32`, `uhal_atomic_load_u32`, `uhal_atomic_store_u32` | Choose C atomics or ESP-IDF critical-section primitives; specify cross-core SMP behavior, ISR restrictions, alignment, barriers, and lock ownership. Preserve the caller's critical-section state. |
| `cache.c` | `uhal_cache_clean`, `uhal_cache_invalidate`, `uhal_cache_clean_invalidate`, `uhal_dcache_enable`, `uhal_icache_enable` | Verify classic ESP32 cache capabilities and IDF API restrictions; align ranges safely; distinguish cache operations from flash-cache suspension; make unsupported operations explicit. |
| `clock.c` | `uhal_clk_freq_hz`, `uhal_clk_periph_enable`, `uhal_clk_periph_disable`, `uhal_reset_periph_assert`, `uhal_reset_periph_release`, `uhal_reset_cause` | Map UETD domains to documented IDF clock/peripheral services; avoid changing global clock policy from a peripheral gate API; preserve SDK ownership and return truthful failures. |
| `core.c` | `uhal_core_id`, `uhal_cpu_freq_hz`, `uhal_system_reset`, `uhal_cycle_count`, `uhal_cycle_count_64`, `uhal_cycles_per_us` | Cover both ESP32 cores; define whether cycle/time values are per-core or global; use IDF monotonic time where appropriate; specify clock scaling and reset semantics. |
| `fault.c` | `uhal_fault_hook`, `HardFault_Handler`, `MemManage_Handler`, `BusFault_Handler`, `UsageFault_Handler` | Do not export Cortex exception handlers on Xtensa. Map documented ESP-IDF panic/reset hooks through a separate adapter and preserve the original SDK panic handler where required. |
| `gpio.c` | `uhal_gpio_init`, `uhal_gpio_write`, `uhal_gpio_read`, `uhal_gpio_toggle`, `uhal_gpio_set`, `uhal_gpio_clear` | Map the UETD pin identifier to IDF GPIO numbers, validate input-only/strapping/flash-reserved pins from board facts, and handle SDK return codes. Never assume STM32 AF numbers apply. |
| `irq.c` | `uhal_irq_enable`, `uhal_irq_disable`, `uhal_irq_set_pending`, `uhal_irq_clear_pending`, `uhal_critical_enter`, `uhal_critical_exit` | Replace CMSIS IRQ numbers/priority grouping with IDF interrupt allocation and core-affinity rules; document ISR allocation, nesting, SMP critical sections, and unsupported pending operations. |
| `memory.c` | `uhal_mpu_configure`, `uhal_backup_write`, `uhal_backup_read` | The classic ESP32 has no Cortex MPU. Define capability reporting; map retained RTC memory/register behavior with bounds, reset-retention, and deep-sleep semantics. |
| `timing.c` | `uetd_time_now_us`, `uetd_delay_us`, `uetd_delay_ms` | Use a documented monotonic IDF clock/timer; define behavior before scheduler startup, across light/deep sleep, under clock scaling, and for large durations; avoid busy waiting when a task delay is appropriate. |

Before enabling this target, pin a coherent ESP-IDF release and host tool set
in `registry/targets/toolchains/`, then add the board build/flash profile and
the generated IDF component/project scaffolds under `templates/targets/esp_idf/`.
