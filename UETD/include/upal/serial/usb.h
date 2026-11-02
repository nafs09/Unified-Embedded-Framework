/// @file include/upal/serial/usb.h
/// @brief Reserved public boundary for the planned USB OTG transport module.
///
/// The V1.2 specification lists this header/module in its file tree but does
/// not define USB functions or structures. CDC/HID integration is now a
/// first-pass requirement. Draft profiles live in capability_contracts.h and
/// uproto/usb_device.h; the exact controller provider still needs implementation.

#ifndef UETD_UPAL_USB_H
#define UETD_UPAL_USB_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/target.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Add the USB controller/profile contract when the V1.2 planned API is defined. */

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPAL_USB_H */
