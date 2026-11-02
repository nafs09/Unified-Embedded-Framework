/// @brief CMSIS interrupt-controller and local interrupt-mask implementation.
#include "cmsis.h"
#include <uhal/irq.h>
#include <ucore/assert.h>
#if UHAL_CORE_COUNT != 1
#error "This local-mask critical section is single-core; select a multicore platform lock"
#endif

uhal_critical_t uhal_critical_enter(void) {
    const uhal_critical_t saved = __get_PRIMASK();
    __disable_irq();
    __DMB();
    return saved;
}
void uhal_critical_exit(uhal_critical_t saved) {
    __DMB();
    __set_PRIMASK((uint32_t)saved);
}
#if UHAL_HAS_IRQ_PRIORITY_MASK
#if UETD_CM_BASELINE
#error "BASEPRI is unavailable on baseline Cortex-M cores"
#endif
uhal_critical_t uhal_basepri_enter(uetd_u8_t priority) {
    const uhal_critical_t saved = __get_BASEPRI();
    UETD_ASSERT(priority < (1u << __NVIC_PRIO_BITS));
    if (priority >= (1u << __NVIC_PRIO_BITS)) { return saved; }
    __set_BASEPRI_MAX((uint32_t)priority << (8u - __NVIC_PRIO_BITS));
    __DSB(); __ISB();
    return saved;
}
void uhal_basepri_exit(uhal_critical_t saved) { __DMB(); __set_BASEPRI((uint32_t)saved); }
#endif

static uetd_status_t validate_irq(uhal_irq_t irq) {
    if (irq < 0) { return UETD_INVALID_ARG; }
    if (UHAL_IRQ_COUNT == 0u) { return UETD_NOT_SUPPORTED; }
    return (uint32_t)irq < UHAL_IRQ_COUNT ? UETD_OK : UETD_INVALID_ARG;
}
uetd_status_t uhal_irq_enable(uhal_irq_t irq, uetd_u8_t priority, uetd_u8_t subpriority) {
    const uetd_status_t status = validate_irq(irq);
    if (status != UETD_OK) { return status; }
    uint32_t encoded;
#if UETD_CM_BASELINE
    if (subpriority != 0u || priority >= (1u << __NVIC_PRIO_BITS)) { return UETD_INVALID_ARG; }
    encoded = priority;
#else
    uint32_t decoded_priority, decoded_subpriority;
    const uint32_t group = NVIC_GetPriorityGrouping();
    encoded = NVIC_EncodePriority(group, priority, subpriority);
    NVIC_DecodePriority(encoded, group, &decoded_priority, &decoded_subpriority);
    if (decoded_priority != priority || decoded_subpriority != subpriority) { return UETD_INVALID_ARG; }
#endif
    NVIC_SetPriority((IRQn_Type)irq, encoded);
    NVIC_EnableIRQ((IRQn_Type)irq);
    return UETD_OK;
}
uetd_status_t uhal_irq_disable(uhal_irq_t irq) {
    const uetd_status_t status = validate_irq(irq);
    if (status != UETD_OK) { return status; }
    NVIC_DisableIRQ((IRQn_Type)irq); __DSB(); __ISB(); return UETD_OK;
}
uetd_status_t uhal_irq_set_pending(uhal_irq_t irq) {
    const uetd_status_t status = validate_irq(irq);
    if (status != UETD_OK) { return status; }
    NVIC_SetPendingIRQ((IRQn_Type)irq); return UETD_OK;
}
uetd_status_t uhal_irq_clear_pending(uhal_irq_t irq) {
    const uetd_status_t status = validate_irq(irq);
    if (status != UETD_OK) { return status; }
    NVIC_ClearPendingIRQ((IRQn_Type)irq); return UETD_OK;
}
