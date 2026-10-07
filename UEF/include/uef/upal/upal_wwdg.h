/// @file include/uef/upal/upal_wwdg.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UPAL_WWDG_H
#define UPAL_WWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

/* Window watchdog — fires if serviced too early OR too late */
uef_status_t upal_wwdg_init(uef_u32_t window_early_ms,
                               uef_u32_t window_late_ms);
void         upal_wwdg_refresh(void);

/* Early-warning ISR — fired before reset if refresh window missed */
void upal_wwdg_irq_handler(void);

/* Application hook — called from early-warning ISR; must not return */
UEF_WEAK
void upal_wwdg_early_warning_hook(void);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_WWDG_H */
