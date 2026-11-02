/// @file src/uproto/usb_device.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "uproto/usb_device.h"

uetd_status_t uproto_usb_device_validate(const uproto_usb_device_config_t* config) {
    /* TODO: Validate project-owned identity/descriptors, CDC/HID subset and endpoint/control/RAM budget. */
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_usb_device_init(uproto_usb_device_t* device, const uproto_usb_device_config_t* config) {
    /* TODO: Bind the exact device controller and pinned class provider without claiming host-mode support. */
    (void)device;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_usb_device_poll(uproto_usb_device_t* device, uetd_u16_t work_budget) {
    /* TODO: Bound setup/data/reset/suspend/resume processing and reject malformed host requests. */
    (void)device;
    (void)work_budget;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_usb_device_write(uproto_usb_device_t* device, ucore_read_span_t bytes, size_t* accepted) {
    /* TODO: Copy only admitted bytes into bounded endpoint storage and report exact backpressure. */
    (void)device;
    (void)bytes;
    (void)accepted;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_usb_device_read(uproto_usb_device_t* device, ucore_write_span_t bytes, size_t* received) {
    /* TODO: Return bounded class data with explicit empty/truncation semantics and no stale enumeration handle. */
    (void)device;
    (void)bytes;
    (void)received;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_usb_device_stop(uproto_usb_device_t* device) {
    /* TODO: Quiesce endpoint DMA and invalidate enumeration generations before release. */
    (void)device;
    return UETD_NOT_SUPPORTED;
}

