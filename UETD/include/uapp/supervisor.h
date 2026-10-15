/// @file include/uapp/supervisor.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UAPP_SUPERVISOR_H
#define UAPP_SUPERVISOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"
#include "umid/health.h"

typedef enum {
    UAPP_SUPERVISOR_REASON_NONE = 0,
    UAPP_SUPERVISOR_REASON_REPORTED_FAULT = 1,
    UAPP_SUPERVISOR_REASON_STALE_REPORT = 2,
} uapp_supervisor_reason_t;

typedef struct {
    uetd_u8_t id;
    umid_health_status_t reported_status;
    umid_health_status_t effective_status;
    uapp_supervisor_reason_t reason;
    uetd_u32_t fault_code;
    uetd_u64_t report_age_us;
} uapp_supervisor_event_t;

/* Supervisor monitors health entries and triggers fault actions */
typedef struct {
    uetd_u32_t check_period_ms;
    uetd_u32_t fault_threshold; /* consecutive faults before action */
    uetd_u32_t stale_after_ms;  /* 0 disables stale-report detection */
    void (*on_fault)(uetd_u8_t id, umid_health_status_t s, void* ctx);
    void (*on_event)(const uapp_supervisor_event_t* event, void* ctx);
    void* ctx;
} uapp_supervisor_cfg_t;

typedef struct {
    uapp_supervisor_cfg_t cfg;
    uetd_u32_t fault_counts[UMID_HEALTH_MAX_ENTRIES];
    uapp_supervisor_reason_t active_reasons[UMID_HEALTH_MAX_ENTRIES];
    uetd_u64_t last_check_us; /* UETD monotonic microsecond time */
} uapp_supervisor_t;

void uapp_supervisor_init(uapp_supervisor_t* sv, const uapp_supervisor_cfg_t* cfg);
void uapp_supervisor_tick(uapp_supervisor_t* sv); /* call periodically */

#ifdef __cplusplus
}
#endif

#endif /* UAPP_SUPERVISOR_H */
