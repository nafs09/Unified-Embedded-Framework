/// @file include/upal/timing/rtc.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_RTC_H
#define UPAL_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

typedef struct {
    uetd_u8_t hours, minutes, seconds;
    uetd_u16_t subseconds; /* 1/256 second units */
} upal_rtc_time_t;

typedef struct {
    uetd_u8_t day, month;
    uetd_u16_t year;   /* full year */
    uetd_u8_t weekday; /* 1=Monday */
} upal_rtc_date_t;

/// Initialize the real-time clock.
uetd_status_t upal_rtc_init(void);
/// Set the time fields on the real-time clock.
void upal_rtc_set_time(const upal_rtc_time_t* t);
/// Set the date fields on the real-time clock.
void upal_rtc_set_date(const upal_rtc_date_t* d);
/// Read the current time from the real-time clock.
void upal_rtc_get_time(upal_rtc_time_t* t);
/// Read the current date from the real-time clock.
void upal_rtc_get_date(upal_rtc_date_t* d);
/// Return the current Unix timestamp from the real-time clock.
uetd_u32_t upal_rtc_get_unix(void);
/// Set the real-time clock from a Unix timestamp.
void upal_rtc_set_unix(uetd_u32_t ts);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_RTC_H */
