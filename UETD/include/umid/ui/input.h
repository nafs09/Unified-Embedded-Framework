/// @file include/umid/ui/input.h
/// @brief First-pass scaffold: Bounded touch, button and encoder events.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/MYOVENA_HANDHELD.md#rendering-flush-and-touch.
#ifndef UETD_UMID_INPUT_H
#define UETD_UMID_INPUT_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/display/touch.h"

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
typedef enum { UMID_INPUT_CONTACT = 0, UMID_INPUT_BUTTON, UMID_INPUT_ENCODER, UMID_INPUT_RESET } umid_input_kind_t;
typedef struct { umid_input_kind_t kind; uetd_u16_t source_id, contact_id; uetd_i32_t x, y, delta; ucore_clock_stamp_t captured; uetd_u64_t sequence; uetd_u32_t generation; bool pressed; bool cancelled; } umid_input_event_t;
typedef struct { umid_input_event_t* queue; size_t capacity; uetd_u32_t debounce_us; uetd_u32_t long_press_us; uetd_u32_t logical_width, logical_height; } umid_input_config_t;
typedef struct { void* storage; uetd_u64_t generation; bool initialized; } umid_input_t;

/// Validate fixed queue capacity, timestamp domain and coordinate/debounce policy.
uetd_status_t umid_input_init(umid_input_t* input, const umid_input_config_t* config);
/// Map coherent contact IDs/orientation and preserve release/cancel semantics under overflow.
uetd_status_t umid_input_submit_touch(umid_input_t* input, const upal_touch_frame_t* frame);
/// Admit copied button/encoder events with explicit source generation; never call GUI code in an ISR.
uetd_status_t umid_input_submit_event(umid_input_t* input, const umid_input_event_t* event);
/// Deliver bounded events without fabricating input on an empty queue.
uetd_status_t umid_input_next(umid_input_t* input, umid_input_event_t* event);
/// Cancel active contacts/buttons coherently after reset, disconnect or wake invalidation.
uetd_status_t umid_input_cancel_source(umid_input_t* input, uetd_u16_t source_id, uetd_u32_t generation);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMID_INPUT_H */
