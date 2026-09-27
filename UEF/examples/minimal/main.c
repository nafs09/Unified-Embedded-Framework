/// @file examples/minimal/main.c
/// @brief Minimal component lifecycle without board-specific peripherals.

#include "uef/uapp/uapp_lifecycle.h"

static uef_status_t initialize_component(void* context) {
    (void)context;
    return UEF_OK;
}

static uef_status_t start_component(void* context) {
    (void)context;
    return UEF_OK;
}

static void stop_component(void* context) {
    (void)context;
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
    };

    if (uapp_component_init(&component) != UEF_OK) {
        return 1;
    }
    if (uapp_component_start(&component) != UEF_OK) {
        return 2;
    }

    /* Stop through the lifecycle API so the component reaches its declared terminal state. */
    uapp_component_stop(&component);
    return 0;
}
