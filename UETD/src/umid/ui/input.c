/// @file src/umid/ui/input.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "umid/ui/input.h"

uetd_status_t umid_input_init(umid_input_t* input, const umid_input_config_t* config) {
    /* TODO: Validate fixed queue capacity, timestamp domain and coordinate/debounce policy. */
    (void)input;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_input_submit_touch(umid_input_t* input, const upal_touch_frame_t* frame) {
    /* TODO: Map coherent contact IDs/orientation and preserve release/cancel semantics under overflow. */
    (void)input;
    (void)frame;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_input_submit_event(umid_input_t* input, const umid_input_event_t* event) {
    /* TODO: Admit copied button/encoder events with explicit source generation; never call GUI code in an ISR. */
    (void)input;
    (void)event;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_input_next(umid_input_t* input, umid_input_event_t* event) {
    /* TODO: Deliver bounded events without fabricating input on an empty queue. */
    (void)input;
    (void)event;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_input_cancel_source(umid_input_t* input, uetd_u16_t source_id, uetd_u32_t generation) {
    /* TODO: Cancel active contacts/buttons coherently after reset, disconnect or wake invalidation. */
    (void)input;
    (void)source_id;
    (void)generation;
    return UETD_NOT_SUPPORTED;
}

