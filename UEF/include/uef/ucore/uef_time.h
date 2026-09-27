/// @file include/uef/ucore/uef_time.h
/// @brief Time types. UEF public time unit is microseconds throughout.

#ifndef UEF_TIME_H
#define UEF_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"

typedef uef_u64_t uef_time_us_t;    /* absolute time in microseconds */
typedef uef_u64_t uef_dur_us_t;     /* duration in microseconds */

#define UEF_US(n)    ((uef_dur_us_t)(n))
#define UEF_MS(n)    ((uef_dur_us_t)((n) * 1000ULL))
#define UEF_S(n)     ((uef_dur_us_t)((n) * 1000000ULL))

/* Elapsed time since system start — implemented by UHAL DWT or SysTick */
uef_time_us_t uef_time_now_us(void);

/* Blocking delay — uses DWT spin; for init sequences only */
void uef_delay_us(uef_dur_us_t us);
void uef_delay_ms(uef_u32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* UEF_TIME_H */
