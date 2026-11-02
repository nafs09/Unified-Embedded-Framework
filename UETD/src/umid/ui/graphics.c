/// @file src/umid/ui/graphics.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "umid/ui/graphics.h"

uetd_status_t umid_graphics_init(umid_graphics_t* graphics, const umid_graphics_config_t* config) {
    /* TODO: Bind a pinned GUI adapter, fixed arena and compatible display; declare all GUI/decoder memory and owner context. */
    (void)graphics;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_graphics_flush(umid_graphics_t* graphics, const upal_display_region_t* region, uetd_u64_t* request_id) {
    /* TODO: Submit a bounded region and retain the GUI buffer until genuine display terminal completion. */
    (void)graphics;
    (void)region;
    (void)request_id;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_graphics_poll(umid_graphics_t* graphics, uetd_u16_t work_budget) {
    /* TODO: Bound render/event/completion work; signal GUI flush-ready once after pixel ownership returns. */
    (void)graphics;
    (void)work_budget;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_graphics_register_asset(umid_graphics_t* graphics, const umid_graphics_asset_t* asset) {
    /* TODO: Validate immutable asset identity/dimensions/encoding and bounded decoder requirements before registration. */
    (void)graphics;
    (void)asset;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_graphics_stop(umid_graphics_t* graphics) {
    /* TODO: Reject pending flushes or use explicit cancellation; release resources only after all GUI and display loans end. */
    (void)graphics;
    return UETD_NOT_SUPPORTED;
}

