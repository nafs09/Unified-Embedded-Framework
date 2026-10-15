/// @file examples/minimal/main.c
/// @brief Minimal component lifecycle without board-specific peripherals.

#include "uapp/lifecycle.h"

static uetd_status_t initialize_component(void* context) {
    (void)context;
    return UETD_OK;
}

static uetd_status_t start_component(void* context) {
    (void)context;
    return UETD_OK;
}

static uetd_status_t stop_component(void* context) {
    (void)context;
    return UETD_OK;
}

int main(void) {
    /* The minimal example exercises UAPP lifecycle ordering without pretending to own hardware. */
    uapp_component_t component = {
        .state = UAPP_STATE_CREATED,
        .name = "minimal",
        .init = initialize_component,
        .start = start_component,
        .stop = stop_component,
        .on_fault = NULL,
        .ctx = NULL,
        .fault_code = 0u,
    };
    if (uapp_component_init(&component) != UETD_OK) { return 1; }
    if (uapp_component_start(&component) != UETD_OK) { return 2; }
    /* Stop through the lifecycle API so the component returns to INITIALISED. */
    if (uapp_component_stop(&component) != UETD_OK) { return 3; }
    return 0;
}
