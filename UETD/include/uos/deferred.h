/// @file include/uos/deferred.h
/// @brief Allocation-free, single-owner deferred callbacks and finite timers.
#ifndef UETD_UOS_DEFERRED_H
#define UETD_UOS_DEFERRED_H
#include "ucore/deadline.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*uos_deferred_callback_t)(void* context);
typedef struct { uetd_u64_t id, generation; } uos_deferred_token_t;
typedef struct { uos_deferred_callback_t callback; void* context; ucore_deadline_t release; } uos_deferred_item_t;
typedef struct { uos_deferred_item_t item; uos_deferred_token_t token; bool occupied; } uos_deferred_slot_t;
typedef struct {
    uos_deferred_slot_t* slots;
    size_t capacity;
    ucore_clock_stamp_t clock; /* establishes domain/generation, not a clock reader */
} uos_deferred_config_t;
typedef struct {
    uos_deferred_config_t config;
    uetd_u64_t generation, next_id, running_id;
    ucore_clock_stamp_t last_poll;
    bool initialized, polling;
} uos_deferred_t;

#define UOS_DEFERRED_INITIALIZER {0}
/* Initialize with {0}. All APIs are owner-only, never ISR APIs. Slots and callback
 * contexts outlive queued work. Callbacks may submit/cancel but cannot init/poll.
 * Poll snapshots admission IDs: work submitted by callbacks runs on a later poll.
 * Ready items execute in admission order; future items do not block ready items.
 * No callback executes under a lock. A budget bounds callback count, not cost. */
uetd_status_t uos_deferred_init(uos_deferred_t* deferred, const uos_deferred_config_t* config);
uetd_status_t uos_deferred_submit(uos_deferred_t* deferred, const uos_deferred_item_t* item, uos_deferred_token_t* token);
uetd_status_t uos_deferred_cancel(uos_deferred_t* deferred, uos_deferred_token_t token);
uetd_status_t uos_deferred_poll(uos_deferred_t* deferred, ucore_clock_stamp_t now, uetd_u16_t callback_budget);
/* Reset discards queued work and invalidates tokens; cannot run inside callbacks. */
uetd_status_t uos_deferred_reset(uos_deferred_t* deferred, ucore_clock_stamp_t clock);
#ifdef __cplusplus
}
#endif
#endif
