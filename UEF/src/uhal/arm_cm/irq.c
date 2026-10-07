/// @file src/uhal/arm_cm/irq.c
/// @brief Source scaffold for the V1.2 public contract in uef/uhal/uhal_irq.h.
///
/// Implementation intent: Map IRQ operations to CMSIS NVIC calls and document priority grouping
///   at board startup.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uhal/uhal_irq.h>

void uhal_irq_enable(
    IRQn_Type irqn,
    uef_u8_t preempt_priority,
    uef_u8_t sub_priority
) {
    /* TODO(UEF Cortex-M):
     * Configure priority grouping and NVIC state using the target priority-bit count;
     * preserve pending/enabled state and document safe ISR use. Implement this contract for
     * the selected Cortex-M CMSIS device without assuming a particular vendor register map.
     * Keep interrupt and register side effects documented, bounded, and safe for the active
     * target.
     */
    (void)irqn;
    (void)preempt_priority;
    (void)sub_priority;
}

void uhal_irq_disable(
    IRQn_Type irqn
) {
    /* TODO(UEF Cortex-M):
     * Configure priority grouping and NVIC state using the target priority-bit count;
     * preserve pending/enabled state and document safe ISR use. Implement this contract for
     * the selected Cortex-M CMSIS device without assuming a particular vendor register map.
     * Keep interrupt and register side effects documented, bounded, and safe for the active
     * target.
     */
    (void)irqn;
}

void uhal_irq_set_pending(
    IRQn_Type irqn
) {
    /* TODO(UEF Cortex-M):
     * Configure priority grouping and NVIC state using the target priority-bit count;
     * preserve pending/enabled state and document safe ISR use. Implement this contract for
     * the selected Cortex-M CMSIS device without assuming a particular vendor register map.
     * Keep interrupt and register side effects documented, bounded, and safe for the active
     * target.
     */
    (void)irqn;
}

void uhal_irq_clear_pending(
    IRQn_Type irqn
) {
    /* TODO(UEF Cortex-M):
     * Configure priority grouping and NVIC state using the target priority-bit count;
     * preserve pending/enabled state and document safe ISR use. Implement this contract for
     * the selected Cortex-M CMSIS device without assuming a particular vendor register map.
     * Keep interrupt and register side effects documented, bounded, and safe for the active
     * target.
     */
    (void)irqn;
}
