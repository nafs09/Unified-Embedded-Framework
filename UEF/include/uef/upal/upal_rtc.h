/// @file include/uef/upal/upal_rtc.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPAL_RTC_H
#define UPAL_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

typedef struct {
    uef_u8_t  hours, minutes, seconds;
    uef_u16_t subseconds;   /* 1/256 second units */
} upal_rtc_time_t;

typedef struct {
    uef_u8_t  day, month;
    uef_u16_t year;          /* full year */
    uef_u8_t  weekday;       /* 1=Monday */
} upal_rtc_date_t;

uef_status_t upal_rtc_init(void);
void         upal_rtc_set_time(const upal_rtc_time_t* t);
void         upal_rtc_set_date(const upal_rtc_date_t* d);
void         upal_rtc_get_time(upal_rtc_time_t* t);
void         upal_rtc_get_date(upal_rtc_date_t* d);
uef_u32_t    upal_rtc_get_unix(void);
void         upal_rtc_set_unix(uef_u32_t ts);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_RTC_H */
