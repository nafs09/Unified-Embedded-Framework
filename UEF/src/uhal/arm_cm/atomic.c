/// @file src/uhal/arm_cm/atomic.c
/// @brief Full-barrier 32-bit atomic operations for a single Cortex-M core.
///
/// The critical-section fallback keeps this backend usable on Cortex-M0/M0+ as
/// well as cores with exclusive-access instructions. Each operation masks
/// interrupts only for the single load/update/store sequence; callers must not
/// use these operations from NMI or fault handlers.
#include <stddef.h>
#include <stdint.h>

#include <uef/uhal/uhal_atomic.h>
#include <uef/uhal/uhal_irq.h>

static bool target_is_valid(const volatile uef_u32_t* target) {
    return target != NULL &&
           (((uintptr_t)target % _Alignof(uef_u32_t)) == 0u);
}

static uhal_critical_t atomic_begin(void) {
    const uhal_critical_t saved = uhal_critical_enter();
    __DMB();
    return saved;
}

static void atomic_end(uhal_critical_t saved) {
    __DMB();
    uhal_critical_exit(saved);
}

bool uhal_atomic_cas_u32(volatile uef_u32_t* target,
                         uef_u32_t expected,
                         uef_u32_t desired) {
    if (!target_is_valid(target)) {
        return false;
    }

    const uhal_critical_t saved = atomic_begin();
    const bool matched = *target == expected;
    if (matched) {
        *target = desired;
    }
    atomic_end(saved);
    return matched;
}

uef_u32_t uhal_atomic_fetch_add_u32(volatile uef_u32_t* target,
                                    uef_u32_t value) {
    if (!target_is_valid(target)) {
        return 0u;
    }

    const uhal_critical_t saved = atomic_begin();
    const uef_u32_t previous = *target;
    *target = previous + value;
    atomic_end(saved);
    return previous;
}

uef_u32_t uhal_atomic_fetch_and_u32(volatile uef_u32_t* target,
                                    uef_u32_t value) {
    if (!target_is_valid(target)) {
        return 0u;
    }

    const uhal_critical_t saved = atomic_begin();
    const uef_u32_t previous = *target;
    *target = previous & value;
    atomic_end(saved);
    return previous;
}

uef_u32_t uhal_atomic_fetch_or_u32(volatile uef_u32_t* target,
                                   uef_u32_t value) {
    if (!target_is_valid(target)) {
        return 0u;
    }

    const uhal_critical_t saved = atomic_begin();
    const uef_u32_t previous = *target;
    *target = previous | value;
    atomic_end(saved);
    return previous;
}

uef_u32_t uhal_atomic_load_u32(const volatile uef_u32_t* source) {
    if (!target_is_valid(source)) {
        return 0u;
    }

    const uhal_critical_t saved = atomic_begin();
    const uef_u32_t value = *source;
    atomic_end(saved);
    return value;
}

void uhal_atomic_store_u32(volatile uef_u32_t* destination,
                           uef_u32_t value) {
    if (!target_is_valid(destination)) {
        return;
    }

    const uhal_critical_t saved = atomic_begin();
    *destination = value;
    atomic_end(saved);
}
