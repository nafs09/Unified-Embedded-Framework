/// @file include/upal/display/touch.h
/// @brief First-pass scaffold: Coherent bounded touchscreen contact snapshots.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/MYOVENA_HANDHELD.md#rendering-flush-and-touch.
#ifndef UETD_UPAL_TOUCH_H
#define UETD_UPAL_TOUCH_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/deadline.h"

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
#define UPAL_TOUCH_CONTACT_LIMIT 10u
typedef enum { UPAL_TOUCH_DOWN = 0, UPAL_TOUCH_MOVE, UPAL_TOUCH_UP, UPAL_TOUCH_CANCEL } upal_touch_phase_t;
typedef struct { uetd_u16_t id; upal_touch_phase_t phase; uetd_i32_t x, y; uetd_u16_t pressure; bool pressure_valid; } upal_touch_contact_t;
typedef struct { ucore_clock_stamp_t captured; uetd_u64_t sequence; uetd_u32_t source_generation; uetd_u32_t quality; uetd_u8_t count; upal_touch_contact_t contacts[UPAL_TOUCH_CONTACT_LIMIT]; bool complete_state; } upal_touch_frame_t;
typedef struct { uetd_u32_t logical_width, logical_height; uetd_u8_t maximum_contacts; uetd_u32_t maximum_age_us; void* provider_context; } upal_touch_config_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } upal_touch_t;

/// Validate contact capacity, dimensions and the provider timestamp/coordinate/calibration contract.
uetd_status_t upal_touch_init(upal_touch_t* touch, const upal_touch_config_t* config);
/// Defer bus reads and decode within a fixed budget; detect lost IRQ, overrun and controller reset.
uetd_status_t upal_touch_poll(upal_touch_t* touch, uetd_u16_t work_budget);
/// Commit a coherent complete frame with contact IDs, quality and generation; never partially publish contacts.
uetd_status_t upal_touch_read(upal_touch_t* touch, upal_touch_frame_t* frame);
/// Cancel active contacts coherently and gate input until post-wake controller state is valid.
uetd_status_t upal_touch_set_sleep(upal_touch_t* touch, bool sleeping);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPAL_TOUCH_H */
