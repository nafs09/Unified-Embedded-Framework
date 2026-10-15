/// @file include/uhal/target.h
/// @brief Isolates CMSIS and core intrinsics from portable UETD interfaces.

#ifndef UETD_UHAL_TARGET_H
#define UETD_UHAL_TARGET_H

#include <ucore/types.h>
#include <uhal/target_config.h>

#if defined(UETD_TARGET_CORTEX_M)
/*
 * The board toolchain supplies this CMSIS device header. No vendor header is
 * included by middleware or application modules directly.
 */
#if defined(UETD_CMSIS_DEVICE_HEADER)
#include UETD_CMSIS_DEVICE_HEADER
#else
#include <cmsis_device.h>
#endif
#else
/* Host simulation has no NVIC; integer IRQ identifiers keep contracts testable. */
typedef int32_t IRQn_Type;
#ifndef __NVIC_PRIO_BITS
#define __NVIC_PRIO_BITS 4U
#endif
#ifndef __DMB
#define __DMB() ((void)0)
#endif
#ifndef __DSB
#define __DSB() ((void)0)
#endif
#ifndef __ISB
#define __ISB() ((void)0)
#endif
#ifndef __NOP
#define __NOP() ((void)0)
#endif
#ifndef __get_PRIMASK
#define __get_PRIMASK() 0U
#endif
#ifndef __disable_irq
#define __disable_irq() ((void)0)
#endif
#ifndef __set_PRIMASK
#define __set_PRIMASK(value) ((void)(value))
#endif
#ifndef __get_BASEPRI
#define __get_BASEPRI() 0U
#endif
#ifndef __set_BASEPRI_MAX
#define __set_BASEPRI_MAX(value) ((void)(value))
#endif
#ifndef __set_BASEPRI
#define __set_BASEPRI(value) ((void)(value))
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#define UETD_WEAK __attribute__((weak))
#define UETD_SECTION(name) __attribute__((section(name)))
#define UETD_ALIGNED(bytes) __attribute__((aligned(bytes)))
#else
#define UETD_WEAK
#define UETD_SECTION(name)
#define UETD_ALIGNED(bytes) __declspec(align(bytes))
#endif

#endif /* UETD_UHAL_TARGET_H */
