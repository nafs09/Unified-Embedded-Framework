/// @file include/ucore/deadline.h
/// @brief Clock-domain stamps and checked finite deadlines.
/// Outputs are unchanged on error. Infinite deadlines still carry a clock
/// domain/generation; remaining returns UINT64_MAX for an infinite deadline.
/// See docs/design/complete-system/FOUNDATIONS.md#monotonic-time-and-deadlines.
#ifndef UETD_UCORE_DEADLINE_H
#define UETD_UCORE_DEADLINE_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Intended implementation: serialized task/main owner; no hidden heap.
 * Configurations, provider context and backing storage outlive the object.
 * Explicitly asynchronous operations retain buffers only after admission and
 * until terminal completion; synchronous spans are borrowed only for the call.
 * Schema IDs bind typed, versioned layouts/units; they do not permit arbitrary
 * native structs as a wire representation. Output/state commits and callback
 * context must be reviewed against the linked design before implementation.
 */
typedef struct { uetd_u64_t ticks_us; uetd_u32_t domain; uetd_u32_t generation; } ucore_clock_stamp_t;
typedef struct { ucore_clock_stamp_t expires; bool infinite; } ucore_deadline_t;

/// Reject finite addition overflow; preserve clock domain/generation without mapping overflow to infinity.
uetd_status_t ucore_deadline_after(ucore_clock_stamp_t now, uetd_u64_t duration_us, ucore_deadline_t* result);
/// Reject mismatched clock domain/generation and distinguish the exact expiration boundary.
uetd_status_t ucore_deadline_expired(ucore_deadline_t deadline, ucore_clock_stamp_t now, bool* expired);
/// Define expired/infinite results explicitly and never subtract across clock resets.
uetd_status_t ucore_deadline_remaining(ucore_deadline_t deadline, ucore_clock_stamp_t now, uetd_u64_t* remaining_us);
/// Convert units with checked multiplication; do not silently saturate or wrap.
uetd_status_t ucore_duration_from_ms(uetd_u64_t milliseconds, uetd_u64_t* microseconds);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UCORE_DEADLINE_H */
