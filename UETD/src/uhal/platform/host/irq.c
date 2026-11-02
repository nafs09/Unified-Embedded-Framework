/// @brief Host critical sections serialize threads and allow nested entry.
#include <uhal/irq.h>
#include <uhal/atomic.h>
#include <uhal/core.h>
#include <ucore/assert.h>

static volatile uetd_u32_t lock;
#if defined(_MSC_VER)
static __declspec(thread) uetd_u32_t depth;
#else
static _Thread_local uetd_u32_t depth;
#endif

uhal_critical_t uhal_critical_enter(void) {
    const uhal_critical_t saved = depth;
    if (depth == 0u) {
        while (!uhal_atomic_cas_u32(&lock, 0u, 1u)) { UHAL_NOP(); }
    }
    ++depth;
    return saved;
}
void uhal_critical_exit(uhal_critical_t saved) {
    /* A mismatched token must never unlock someone else's critical section. */
    UETD_ASSERT(depth != 0u && saved == depth - 1u);
    if (depth == 0u || saved != depth - 1u) { return; }
    depth = (uetd_u32_t)saved;
    if (depth == 0u) { uhal_atomic_store_u32(&lock, 0u); }
}
uetd_status_t uhal_irq_enable(uhal_irq_t irq, uetd_u8_t priority, uetd_u8_t subpriority) {
    (void)irq; (void)priority; (void)subpriority; return UETD_NOT_SUPPORTED;
}
uetd_status_t uhal_irq_disable(uhal_irq_t irq) { (void)irq; return UETD_NOT_SUPPORTED; }
uetd_status_t uhal_irq_set_pending(uhal_irq_t irq) { (void)irq; return UETD_NOT_SUPPORTED; }
uetd_status_t uhal_irq_clear_pending(uhal_irq_t irq) { (void)irq; return UETD_NOT_SUPPORTED; }
