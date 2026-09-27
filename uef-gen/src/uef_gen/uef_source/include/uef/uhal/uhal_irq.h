/// @file include/uef/uhal/uhal_irq.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UHAL_IRQ_H
#define UHAL_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include <uef/uhal/target.h>   /* IRQn_Type — device specific */

/* Configure and enable an interrupt */
void uhal_irq_enable(IRQn_Type irqn, uef_u8_t preempt_priority,
                      uef_u8_t sub_priority);
void uhal_irq_disable(IRQn_Type irqn);
void uhal_irq_set_pending(IRQn_Type irqn);
void uhal_irq_clear_pending(IRQn_Type irqn);

/* Critical section — PRIMASK based (blocks all maskable interrupts) */
typedef uef_u32_t uhal_critical_t;
static inline uhal_critical_t uhal_critical_enter(void) {
    uhal_critical_t m = __get_PRIMASK();
    __disable_irq();
    return m;
}
static inline void uhal_critical_exit(uhal_critical_t saved) {
    __set_PRIMASK(saved);
}

/* BASEPRI critical section — blocks below a priority threshold.
 * Preferred in production; fault handlers remain active. */
static inline uhal_critical_t uhal_basepri_enter(uef_u8_t min_prio) {
    uhal_critical_t old = __get_BASEPRI();
    __set_BASEPRI_MAX((uef_u32_t)min_prio << (8u - __NVIC_PRIO_BITS));
    return old;
}
static inline void uhal_basepri_exit(uhal_critical_t saved) {
    __set_BASEPRI(saved);
}

#ifdef __cplusplus
}
#endif

#endif /* UHAL_IRQ_H */
