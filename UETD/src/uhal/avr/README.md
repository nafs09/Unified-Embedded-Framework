# AVR UHAL implementation scaffold

This folder is a write-in location for an AVR backend. CMake does not select
it. Do not add it to the build until `target.h`, `irq.h`, atomic-width
guarantees, timekeeping, and the exact device-header selection have been
reconciled with AVR hardware. In particular, the current CMSIS `IRQn_Type`,
PRIMASK, and BASEPRI contract is not an AVR interrupt model.

Each `*.c.todo` file beside this README contains the public function headings
for that source. Copy/rename it to `.c` after the shared API contract is
refactored. Keep the API signature from the paired public header, add a detailed
TODO immediately above each unfinished function, and return only documented
failure/neutral values until real behavior exists.

| Source file | Required function headings | Implementation TODO checklist |
| --- | --- | --- |
| `atomic.c` | `uhal_atomic_cas_u32`, `uhal_atomic_fetch_add_u32`, `uhal_atomic_fetch_and_u32`, `uhal_atomic_fetch_or_u32`, `uhal_atomic_load_u32`, `uhal_atomic_store_u32` | Define pointer/alignment checks; specify whether AVR interrupt masking protects 32-bit read-modify-write; restore prior interrupt state exactly; define ISR-safe contexts and barriers. Do not claim lock-free behavior on an 8-bit core. |
| `cache.c` | `uhal_cache_clean`, `uhal_cache_invalidate`, `uhal_cache_clean_invalidate`, `uhal_dcache_enable`, `uhal_icache_enable` | AVR parts without data/instruction caches should use the public capability path; do not create fake cache behavior or invalidate ordinary SRAM. |
| `clock.c` | `uhal_clk_freq_hz`, `uhal_clk_periph_enable`, `uhal_clk_periph_disable`, `uhal_reset_periph_assert`, `uhal_reset_periph_release`, `uhal_reset_cause` | Map clock IDs/gates/reset flags for the exact MCU. State whether the prescaler can change at runtime and how the monotonic clock responds. |
| `core.c` | `uhal_core_id`, `uhal_cpu_freq_hz`, `uhal_system_reset`, `uhal_cycle_count`, `uhal_cycle_count_64`, `uhal_cycles_per_us` | Report unsupported core identity/cycle counters explicitly; use a verified timer for any 64-bit monotonic extension; make reset behavior target-specific. |
| `fault.c` | `uhal_fault_hook`, `HardFault_Handler`, `MemManage_Handler`, `BusFault_Handler`, `UsageFault_Handler` | Implement only vectors the MCU actually defines; AVR faults/reset causes do not map to Cortex exception frames. Capture reset cause and a documented minimal context without calling unsafe services in an ISR. |
| `gpio.c` | `uhal_gpio_init`, `uhal_gpio_write`, `uhal_gpio_read`, `uhal_gpio_toggle`, `uhal_gpio_set`, `uhal_gpio_clear` | Replace the port-pointer/pin encoding with an AVR-compatible, type-safe port/pin representation before mapping DDR/PORT/PIN registers; validate pin ranges and preserve unrelated bits. |
| `irq.c` | `uhal_irq_enable`, `uhal_irq_disable`, `uhal_irq_set_pending`, `uhal_irq_clear_pending`, `uhal_critical_enter`, `uhal_critical_exit` | Redesign the shared interface around architecture-neutral IRQ IDs and save/restore global interrupt state. AVR generally lacks nested per-IRQ priority and BASEPRI semantics; represent unsupported operations honestly. |
| `memory.c` | `uhal_mpu_configure`, `uhal_backup_write`, `uhal_backup_read` | Report MPU unsupported where absent; identify actual backup registers/retention domains per part and validate index and power-domain behavior. |
| `timing.c` | `uetd_time_now_us`, `uetd_delay_us`, `uetd_delay_ms` | Select a free-running timer, handle counter wrap without losing monotonicity, define clock/prescaler changes, bound conversion overflow, and document delay jitter/interrupt behavior. |

Before enabling this backend, also add an AVR board build profile in UETD with
the exact MCU macro, oscillator, fuse policy, memory sizes, pin mapping,
`avr-gcc` executable profile, `avrdude` programmer/transport settings, and
bootloader-preserving default behavior.
