/* Private device-header boundary. Public headers never include CMSIS. */
#ifndef UETD_PRIVATE_CORTEX_M_CMSIS_H
#define UETD_PRIVATE_CORTEX_M_CMSIS_H
#if !defined(UETD_TARGET_CORTEX_M)
#error "The Cortex-M implementation requires UETD_TARGET_CORTEX_M"
#endif
#if !defined(UETD_CMSIS_DEVICE_HEADER)
#error "Select the exact CMSIS device header; there is no guessed fallback"
#endif
#include UETD_CMSIS_DEVICE_HEADER
#if defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_8M_BASE__) || \
    (defined(__CORTEX_M) && ((__CORTEX_M == 0U) || (__CORTEX_M == 1U) || (__CORTEX_M == 23U)))
#define UETD_CM_BASELINE 1
#else
#define UETD_CM_BASELINE 0
#endif
#endif
