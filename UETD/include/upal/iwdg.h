/// @file include/upal/iwdg.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_IWDG_H
#define UPAL_IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

/* Independent watchdog — driven by LSI; cannot be stopped once started */
uetd_status_t upal_iwdg_init(uetd_u32_t timeout_ms);
void upal_iwdg_refresh(void); /* call from verified execution points */

#ifdef __cplusplus
}
#endif

#endif /* UPAL_IWDG_H */
