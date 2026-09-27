/// @file include/uef/upal/upal_iwdg.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPAL_IWDG_H
#define UPAL_IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

/* Independent watchdog — driven by LSI; cannot be stopped once started */
uef_status_t upal_iwdg_init(uef_u32_t timeout_ms);
void         upal_iwdg_refresh(void);  /* call from verified execution points */

#ifdef __cplusplus
}
#endif

#endif /* UPAL_IWDG_H */
