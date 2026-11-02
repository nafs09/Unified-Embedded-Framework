/// @file include/uhal/irq.h
/// @brief Interrupt configuration and short, nestable critical sections.
#ifndef UHAL_IRQ_H
#define UHAL_IRQ_H
#include <ucore/status.h>
#include <uhal/target.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Priority/subpriority support is platform-specific; unsupported requests fail.
 * AVR source masks and ESP-IDF allocated interrupts require platform mappings. */
uetd_status_t uhal_irq_enable(uhal_irq_t irq, uetd_u8_t priority, uetd_u8_t subpriority);
uetd_status_t uhal_irq_disable(uhal_irq_t irq);
uetd_status_t uhal_irq_set_pending(uhal_irq_t irq);
uetd_status_t uhal_irq_clear_pending(uhal_irq_t irq);

typedef uintptr_t uhal_critical_t;
/* Save/restore in LIFO order in the same execution context. Never block/yield.
 * Excludes callers of this API and maskable ISRs that use it. SMP platforms must
 * combine the local interrupt mask with a shared lock. NMI, faults and DMA are
 * outside this guarantee. */
uhal_critical_t uhal_critical_enter(void);
void uhal_critical_exit(uhal_critical_t saved);

/* Optional extension: do not emulate a priority threshold by masking everything.
 * In particular Cortex-M0/M0+ and AVR do not provide BASEPRI. */
#if UHAL_HAS_IRQ_PRIORITY_MASK
uhal_critical_t uhal_basepri_enter(uetd_u8_t min_priority);
void uhal_basepri_exit(uhal_critical_t saved);
#endif
#ifdef __cplusplus
}
#endif
#endif
