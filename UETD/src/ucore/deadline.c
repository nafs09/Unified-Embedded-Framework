/// @file src/ucore/deadline.c
/// @brief Checked arithmetic in explicitly identified monotonic clock domains.
#include "ucore/deadline.h"

uetd_status_t ucore_deadline_after(ucore_clock_stamp_t now, uetd_u64_t duration_us, ucore_deadline_t* result) {
    if (result == NULL) { return UETD_INVALID_ARG; }
    if (duration_us > UINT64_MAX - now.ticks_us) { return UETD_OVERFLOW; }
    const ucore_deadline_t candidate = {{now.ticks_us + duration_us, now.domain, now.generation}, false};
    *result = candidate;
    return UETD_OK;
}

uetd_status_t ucore_deadline_expired(ucore_deadline_t deadline, ucore_clock_stamp_t now, bool* expired) {
    if (expired == NULL) { return UETD_INVALID_ARG; }
    if (deadline.expires.domain != now.domain || deadline.expires.generation != now.generation) {
        return UETD_CONFLICT;
    }
    *expired = !deadline.infinite && now.ticks_us >= deadline.expires.ticks_us;
    return UETD_OK;
}

uetd_status_t ucore_deadline_remaining(ucore_deadline_t deadline, ucore_clock_stamp_t now, uetd_u64_t* remaining_us) {
    bool expired;
    if (remaining_us == NULL) { return UETD_INVALID_ARG; }
    const uetd_status_t status = ucore_deadline_expired(deadline, now, &expired);
    if (status != UETD_OK) { return status; }
    *remaining_us = deadline.infinite ? UINT64_MAX : expired ? 0u : deadline.expires.ticks_us - now.ticks_us;
    return UETD_OK;
}

uetd_status_t ucore_duration_from_ms(uetd_u64_t milliseconds, uetd_u64_t* microseconds) {
    if (microseconds == NULL) { return UETD_INVALID_ARG; }
    if (milliseconds > UINT64_MAX / 1000u) { return UETD_OVERFLOW; }
    *microseconds = milliseconds * 1000u;
    return UETD_OK;
}
