/// @file include/uef/uapp/uapp_supervisor.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UAPP_SUPERVISOR_H
#define UAPP_SUPERVISOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/umid/umid_health.h"

/* Supervisor monitors health entries and triggers fault actions */
typedef struct {
    uef_u32_t     check_period_ms;
    uef_u32_t     fault_threshold;  /* consecutive faults before action */
    void (*on_fault)(uef_u8_t id, umid_health_status_t s, void* ctx);
    void*         ctx;
} uapp_supervisor_cfg_t;

typedef struct {
    uapp_supervisor_cfg_t cfg;
    uef_u32_t             fault_counts[32];   /* per health ID */
    uef_u64_t             last_check_us;      /* UEF monotonic microsecond time */
} uapp_supervisor_t;

void uapp_supervisor_init(uapp_supervisor_t* sv,
                            const uapp_supervisor_cfg_t* cfg);
void uapp_supervisor_tick(uapp_supervisor_t* sv);  /* call periodically */

#ifdef __cplusplus
}
#endif

#endif /* UAPP_SUPERVISOR_H */
