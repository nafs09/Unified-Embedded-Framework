from pathlib import Path
root = Path(__file__).resolve().parents[1]
files = {
'UETD/include/uhal/irq.h': '''/// @file include/uhal/irq.h
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
''',
'UETD/src/uhal/platform/host/irq.c': '''/// @brief Host critical sections serialize threads and allow nested entry.
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
''',
'UETD/src/uhal/arch/cortex_m/irq.c': '''/// @brief CMSIS interrupt-controller and local interrupt-mask implementation.
#include "cmsis.h"
#include <uhal/irq.h>

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
#if defined(__ARM_ARCH_6M__) || (defined(__CORTEX_M) && (__CORTEX_M == 0U))
#error "BASEPRI is unavailable on Cortex-M0/M0+"
#endif
uhal_critical_t uhal_basepri_enter(uetd_u8_t priority) {
    const uhal_critical_t saved = __get_BASEPRI();
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
#if defined(__CORTEX_M) && (__CORTEX_M == 0U)
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
''',
'UETD/src/uhal/arch/cortex_m/core.c': '''/// @brief CPU-only Cortex-M primitives. Clocks and timebase belong to the platform.
#include "cmsis.h"
#include <uhal/core.h>
/* This backend is single-core. A multicore platform must provide its own index,
 * shared exclusion and reset policy instead of selecting this implementation. */
uetd_u32_t uhal_core_id(void) { return 0u; }
void uhal_system_reset(void) { __DSB(); NVIC_SystemReset(); for (;;) {} }
uetd_u32_t uhal_cycle_count(void) {
#if UHAL_HAS_DWT
    /* Startup owns counter enable; reads never turn on tracing as a side effect. */
    return (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) != 0u ? DWT->CYCCNT : 0u;
#else
    return 0u;
#endif
}
uetd_u32_t uhal_cycles_per_us(void) { return uhal_cpu_freq_hz() / 1000000u; }
void uhal_data_memory_barrier(void) { __DMB(); }
void uhal_data_sync_barrier(void) { __DSB(); }
void uhal_instruction_sync_barrier(void) { __ISB(); }
void uhal_nop(void) { __NOP(); }
''',
}
for name, content in files.items():
    (root / name).write_text(content, encoding='utf-8', newline='\n')
for path in (root / 'UETD/include/upal').rglob('*.h'):
    content = path.read_text(encoding='utf-8')
    if 'IRQn_Type' in content:
        path.write_text(content.replace('IRQn_Type', 'uhal_irq_t'), encoding='utf-8', newline='\n')
path = root / 'UETD/src/uhal/arch/cortex_m/atomic.c'
content = path.read_text(encoding='utf-8').replace('#include <stddef.h>', '#include "cmsis.h"\n#include <stddef.h>')
path.write_text(content, encoding='utf-8', newline='\n')
print('Updated neutral contracts and CPU/host primitives.')
