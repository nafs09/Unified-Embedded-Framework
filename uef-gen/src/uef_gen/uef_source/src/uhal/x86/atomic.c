/// @file src/uhal/x86/atomic.c
/// @brief Sequentially consistent host atomics used by simulation code.

#include "uef/uhal/uhal_atomic.h"

#if defined(_MSC_VER)
#  include <Windows.h>
#else
#  include <stdatomic.h>
#endif

/* Host fallback semantics use sequential consistency so simulation code gets a strong ordering. */
/* A null address is handled defensively with the documented neutral result for each operation. */
bool uhal_atomic_cas_u32(volatile uef_u32_t* target,
                         uef_u32_t expected, uef_u32_t desired) {
    if (target == NULL) {
        return false;
    }
#if defined(_MSC_VER)
    /* InterlockedCompareExchange returns the observed old value, matching compare/exchange. */
    return (uef_u32_t)InterlockedCompareExchange((volatile LONG*)target,
        (LONG)desired, (LONG)expected) == expected;
#else
    uef_u32_t observed = expected;
    return __atomic_compare_exchange_n(target, &observed, desired, false,
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#endif
}

uef_u32_t uhal_atomic_fetch_add_u32(volatile uef_u32_t* target,
                                    uef_u32_t value) {
    if (target == NULL) {
        return 0u;
    }
#if defined(_MSC_VER)
    return (uef_u32_t)InterlockedExchangeAdd((volatile LONG*)target, (LONG)value);
#else
    return __atomic_fetch_add(target, value, __ATOMIC_SEQ_CST);
#endif
}

uef_u32_t uhal_atomic_fetch_and_u32(volatile uef_u32_t* target,
                                    uef_u32_t value) {
    if (target == NULL) {
        return 0u;
    }
#if defined(_MSC_VER)
    /* Retry if another thread changes the value between observation and the attempted update. */
    LONG observed;
    LONG prior;
    do {
        observed = InterlockedCompareExchange((volatile LONG*)target, 0, 0);
        prior = InterlockedCompareExchange((volatile LONG*)target,
            observed & (LONG)value, observed);
    } while (prior != observed);
    return (uef_u32_t)observed;
#else
    return __atomic_fetch_and(target, value, __ATOMIC_SEQ_CST);
#endif
}

uef_u32_t uhal_atomic_fetch_or_u32(volatile uef_u32_t* target,
                                   uef_u32_t value) {
    if (target == NULL) {
        return 0u;
    }
#if defined(_MSC_VER)
    /* Return the value observed before the successful atomic OR, as fetch_or requires. */
    LONG observed;
    LONG prior;
    do {
        observed = InterlockedCompareExchange((volatile LONG*)target, 0, 0);
        prior = InterlockedCompareExchange((volatile LONG*)target,
            observed | (LONG)value, observed);
    } while (prior != observed);
    return (uef_u32_t)observed;
#else
    return __atomic_fetch_or(target, value, __ATOMIC_SEQ_CST);
#endif
}

uef_u32_t uhal_atomic_load_u32(const volatile uef_u32_t* source) {
    if (source == NULL) {
        return 0u;
    }
#if defined(_MSC_VER)
    return (uef_u32_t)InterlockedCompareExchange((volatile LONG*)source, 0, 0);
#else
    return __atomic_load_n(source, __ATOMIC_SEQ_CST);
#endif
}

void uhal_atomic_store_u32(volatile uef_u32_t* destination, uef_u32_t value) {
    if (destination == NULL) {
        return;
    }
#if defined(_MSC_VER)
    (void)InterlockedExchange((volatile LONG*)destination, (LONG)value);
#else
    __atomic_store_n(destination, value, __ATOMIC_SEQ_CST);
#endif
}
