/// @file include/uef/uhal/target.h
/// @brief Isolates CMSIS and core intrinsics from portable UEF interfaces.

#ifndef UEF_UHAL_TARGET_H
#define UEF_UHAL_TARGET_H

#include <uef/ucore/uef_types.h>
#include <uef/uhal/target_config.h>

#if defined(UEF_TARGET_CORTEX_M)
/*
 * The board toolchain supplies this CMSIS device header. No vendor header is
 * included by middleware or application modules directly.
 */
#  if defined(UEF_CMSIS_DEVICE_HEADER)
#    include UEF_CMSIS_DEVICE_HEADER
#  else
#    include <cmsis_device.h>
#  endif
#else
/* Host simulation has no NVIC; integer IRQ identifiers keep contracts testable. */
typedef int32_t IRQn_Type;
#  ifndef __NVIC_PRIO_BITS
#    define __NVIC_PRIO_BITS 4U
#  endif
#  ifndef __DMB
#    define __DMB() ((void)0)
#  endif
#  ifndef __DSB
#    define __DSB() ((void)0)
#  endif
#  ifndef __ISB
#    define __ISB() ((void)0)
#  endif
#  ifndef __NOP
#    define __NOP() ((void)0)
#  endif
#  ifndef __get_PRIMASK
#    define __get_PRIMASK() 0U
#  endif
#  ifndef __disable_irq
#    define __disable_irq() ((void)0)
#  endif
#  ifndef __set_PRIMASK
#    define __set_PRIMASK(value) ((void)(value))
#  endif
#  ifndef __get_BASEPRI
#    define __get_BASEPRI() 0U
#  endif
#  ifndef __set_BASEPRI_MAX
#    define __set_BASEPRI_MAX(value) ((void)(value))
#  endif
#  ifndef __set_BASEPRI
#    define __set_BASEPRI(value) ((void)(value))
#  endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define UEF_WEAK __attribute__((weak))
#  define UEF_SECTION(name) __attribute__((section(name)))
#  define UEF_ALIGNED(bytes) __attribute__((aligned(bytes)))
#else
#  define UEF_WEAK
#  define UEF_SECTION(name)
#  define UEF_ALIGNED(bytes) __declspec(align(bytes))
#endif

#endif /* UEF_UHAL_TARGET_H */
