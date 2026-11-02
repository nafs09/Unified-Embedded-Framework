/// @file src/upal/capability_contracts.c
/// @brief Source scaffold; first-pass peripheral extensions fail closed.
#include "upal/capability_contracts.h"

uetd_status_t upal_uart_configure_advanced(upal_uart_t* uart, const upal_uart_advanced_config_t* config) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)uart;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_uart_transmit_advanced(upal_uart_t* uart, const uetd_u8_t* data, uetd_u32_t length, upal_uart_transfer_callback_t callback, void* context) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)uart;
    (void)data;
    (void)length;
    (void)callback;
    (void)context;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_timer_capture_configure(upal_timer_t* timer, const upal_timer_capture_config_t* config) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)timer;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_timer_capture_read(upal_timer_t* timer, upal_timer_capture_event_t* event) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)timer;
    (void)event;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_sync_group_validate(const upal_sync_group_t* group) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)group;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_sync_group_start(upal_sync_group_t* group) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)group;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_sync_group_stop(upal_sync_group_t* group) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)group;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_usb_start_profile(upal_usb_t* usb, const upal_usb_profile_config_t* config) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)usb;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_usb_send_report(upal_usb_t* usb, const uetd_u8_t* report, uetd_u16_t report_bytes) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)usb;
    (void)report;
    (void)report_bytes;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_usb_stop_profile(upal_usb_t* usb) {
    /* TODO: Implement the bounded operation and qualify its exact target/profile
     * per include/upal/CAPABILITY_CONTRACTS.md; never invoke callbacks or release
     * DMA loans before actual terminal completion. */
    (void)usb;
    return UETD_NOT_SUPPORTED;
}

