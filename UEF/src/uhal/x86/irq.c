/// @file src/uhal/x86/irq.c
/// @brief No-op interrupt configuration for host simulation.

#include "uef/uhal/uhal_irq.h"

/* Host simulation has no NVIC; these calls preserve source compatibility but do not dispatch IRQs. */
void uhal_irq_enable(IRQn_Type irqn, uef_u8_t preempt_priority,
                     uef_u8_t sub_priority) {
    (void)irqn;
    (void)preempt_priority;
    (void)sub_priority;
}

/* Pending-state changes have no simulated effect until the host event model defines IRQ delivery. */
void uhal_irq_disable(IRQn_Type irqn) { (void)irqn; }
void uhal_irq_set_pending(IRQn_Type irqn) { (void)irqn; }
void uhal_irq_clear_pending(IRQn_Type irqn) { (void)irqn; }
