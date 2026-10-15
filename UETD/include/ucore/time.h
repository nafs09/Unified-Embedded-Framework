/// @file include/ucore/time.h
/// @brief Time types. UETD public time unit is microseconds throughout.

#ifndef UETD_TIME_H
#define UETD_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"

typedef uetd_u64_t uetd_time_us_t; /* absolute time in microseconds */
typedef uetd_u64_t uetd_dur_us_t;  /* duration in microseconds */

#define UETD_US(n) ((uetd_dur_us_t)(n))
#define UETD_MS(n) ((uetd_dur_us_t)((n) * 1000ULL))
#define UETD_S(n) ((uetd_dur_us_t)((n) * 1000000ULL))

/* Elapsed time since system start — implemented by UHAL DWT or SysTick */
uetd_time_us_t uetd_time_now_us(void);

/* Blocking delay — uses DWT spin; for init sequences only */
void uetd_delay_us(uetd_dur_us_t us);
void uetd_delay_ms(uetd_u32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* UETD_TIME_H */
