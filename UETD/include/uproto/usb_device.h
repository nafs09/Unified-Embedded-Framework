/// @file include/uproto/usb_device.h
/// @brief First-pass scaffold: Bounded CDC/HID device-class profile.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/CONNECTIVITY_SECURITY.md#first-pass-application-protocols.
#ifndef UETD_UPROTO_USB_DEVICE_H
#define UETD_UPROTO_USB_DEVICE_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/span.h"

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
typedef enum { UPROTO_USB_CDC_ACM = 0, UPROTO_USB_HID } uproto_usb_class_t;
typedef struct { uetd_u16_t vendor_id, product_id; uproto_usb_class_t device_class; const uetd_u8_t* descriptors; size_t descriptor_bytes; uetd_u8_t endpoint_count; size_t maximum_transfer_bytes; void* provider_context; } uproto_usb_device_config_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } uproto_usb_device_t;

/// Validate project-owned identity/descriptors, CDC/HID subset and endpoint/control/RAM budget.
uetd_status_t uproto_usb_device_validate(const uproto_usb_device_config_t* config);
/// Bind the exact device controller and pinned class provider without claiming host-mode support.
uetd_status_t uproto_usb_device_init(uproto_usb_device_t* device, const uproto_usb_device_config_t* config);
/// Bound setup/data/reset/suspend/resume processing and reject malformed host requests.
uetd_status_t uproto_usb_device_poll(uproto_usb_device_t* device, uetd_u16_t work_budget);
/// Copy only admitted bytes into bounded endpoint storage and report exact backpressure.
uetd_status_t uproto_usb_device_write(uproto_usb_device_t* device, ucore_read_span_t bytes, size_t* accepted);
/// Return bounded class data with explicit empty/truncation semantics and no stale enumeration handle.
uetd_status_t uproto_usb_device_read(uproto_usb_device_t* device, ucore_write_span_t bytes, size_t* received);
/// Quiesce endpoint DMA and invalidate enumeration generations before release.
uetd_status_t uproto_usb_device_stop(uproto_usb_device_t* device);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPROTO_USB_DEVICE_H */
