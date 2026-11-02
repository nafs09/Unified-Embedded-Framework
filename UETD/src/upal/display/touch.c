/// @file src/upal/display/touch.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "upal/display/touch.h"

uetd_status_t upal_touch_init(upal_touch_t* touch, const upal_touch_config_t* config) {
    /* TODO: Validate contact capacity, dimensions and the provider timestamp/coordinate/calibration contract. */
    (void)touch;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_touch_poll(upal_touch_t* touch, uetd_u16_t work_budget) {
    /* TODO: Defer bus reads and decode within a fixed budget; detect lost IRQ, overrun and controller reset. */
    (void)touch;
    (void)work_budget;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_touch_read(upal_touch_t* touch, upal_touch_frame_t* frame) {
    /* TODO: Commit a coherent complete frame with contact IDs, quality and generation; never partially publish contacts. */
    (void)touch;
    (void)frame;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_touch_set_sleep(upal_touch_t* touch, bool sleeping) {
    /* TODO: Cancel active contacts coherently and gate input until post-wake controller state is valid. */
    (void)touch;
    (void)sleeping;
    return UETD_NOT_SUPPORTED;
}

