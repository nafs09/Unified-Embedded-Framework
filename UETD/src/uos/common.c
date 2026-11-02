/// @file src/uos/common.c
/// @brief Shared monotonic epoch and transactional periodic deadlines.
#include "uos/uos.h"
#include "ucore/time.h"
#include "detail.h"

static uos_clock_provider_t clock_provider;
uos_status_t uos_clock_bind(const uos_clock_provider_t* provider) {
    if (provider == NULL || provider->read == NULL) { return UOS_INVALID_PARAM; }
    if (clock_provider.read != NULL) { return UOS_BUSY; }
    clock_provider = *provider;
    return UOS_OK;
}
static uos_status_t platform_read(void* context, uos_time_t* now) {
    (void)context; *now = uetd_time_now_us(); return UOS_OK;
}
static uos_status_t platform_delay(void* context, uos_dur_t duration) {
    (void)context; uetd_delay_us(duration); return UOS_OK;
}
uos_status_t uos_clock_use_platform(void) {
    const uos_clock_provider_t provider = {platform_read, platform_delay, NULL};
    return uos_clock_bind(&provider);
}
uos_status_t uos_clock_delay(uos_dur_t duration) {
    if (clock_provider.read == NULL) { return UOS_NOT_READY; }
    if (clock_provider.delay == NULL) { return UOS_NOT_SUPPORTED; }
    return clock_provider.delay(clock_provider.context, duration);
}

uos_status_t uos_time_now(uos_time_t* now) {
    if (now == NULL) { return UOS_INVALID_PARAM; }
    if (clock_provider.read == NULL) { return UOS_NOT_READY; }
    uos_time_t value;
    const uos_status_t status = clock_provider.read(clock_provider.context, &value);
    if (status == UOS_OK) { *now = value; }
    return status;
}
uos_status_t uos_duration_from_ms(uetd_u64_t value, uos_dur_t* result) {
    if (result == NULL) { return UOS_INVALID_PARAM; }
    if (value > (UINT64_MAX - 1u) / 1000u) { return UOS_OVERFLOW; }
    *result = value * 1000u;
    return UOS_OK;
}
uos_status_t uos_duration_from_s(uetd_u64_t value, uos_dur_t* result) {
    if (result == NULL) { return UOS_INVALID_PARAM; }
    if (value > (UINT64_MAX - 1u) / 1000000u) { return UOS_OVERFLOW; }
    *result = value * 1000000u;
    return UOS_OK;
}
uos_status_t uos_task_delay_until(uos_time_t* next, uos_dur_t period) {
    if (next == NULL || period == 0u || period == UOS_WAIT_FOREVER) { return UOS_INVALID_PARAM; }
    const uos_time_t anchor = *next;
    if (period > UINT64_MAX - anchor) { return UOS_OVERFLOW; }
    uos_time_t now;
    uos_status_t status = uos_backend_task_ready();
    if (status != UOS_OK) { return status; }
    status = uos_time_now(&now);
    if (status != UOS_OK) { return status; }
    if (anchor > now) {
        status = uos_task_delay(anchor - now);
        if (status != UOS_OK) { return status; }
    }
    uos_time_t after;
    status = uos_time_now(&after);
    if (status != UOS_OK) { return status; }
    if (after < now || after < anchor) { return UOS_BACKEND_ERROR; }
    uetd_u64_t periods = after >= anchor ? (after - anchor) / period : 0u;
    if (periods == UINT64_MAX) { return UOS_OVERFLOW; }
    ++periods;
    if (periods > (UINT64_MAX - anchor) / period) { return UOS_OVERFLOW; }
    *next = anchor + periods * period;
    return UOS_OK;
}
