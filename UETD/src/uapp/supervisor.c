/// @file src/uapp/supervisor.c
/// @brief Threshold-based response to sustained subsystem health faults.

#include "uapp/supervisor.h"
#include "ucore/time.h"
#include <string.h>

void uapp_supervisor_init(uapp_supervisor_t* supervisor, const uapp_supervisor_cfg_t* config) {
    if (supervisor == NULL) { return; }
    memset(supervisor, 0, sizeof(*supervisor));
    if (config != NULL) { supervisor->cfg = *config; }
}

void uapp_supervisor_tick(uapp_supervisor_t* supervisor) {
    if (supervisor == NULL || supervisor->cfg.fault_threshold == 0u) { return; }
    /* This polling API is cooperative: callers choose its context and must invoke it regularly. */
    const uetd_u64_t now_us = uetd_time_now_us();
    const uetd_u64_t check_period_us = (uetd_u64_t)supervisor->cfg.check_period_ms * 1000u;
    if (supervisor->last_check_us != 0u && now_us - supervisor->last_check_us < check_period_us) { return; }
    supervisor->last_check_us = now_us;
    /* The fault counter saturates at the threshold and calls back only on the crossing tick. */
    const uetd_u8_t count = umid_health_count();
    const uetd_u64_t stale_after_us = (uetd_u64_t)supervisor->cfg.stale_after_ms * 1000u;
    for (uetd_u16_t index = 0u; index < (uetd_u16_t)count && index < UMID_HEALTH_MAX_ENTRIES; ++index) {
        const uetd_u8_t id = (uetd_u8_t)index;
        umid_health_entry_t entry;
        if (!umid_health_entry_copy(id, &entry)) {
            supervisor->fault_counts[id] = 0u;
            supervisor->active_reasons[id] = UAPP_SUPERVISOR_REASON_NONE;
            continue;
        }
        uapp_supervisor_reason_t reason = UAPP_SUPERVISOR_REASON_NONE;
        umid_health_status_t effective_status = entry.status;
        uetd_u64_t age_us = now_us >= entry.last_update_us ? now_us - entry.last_update_us : ~(uetd_u64_t)0u;
        if (entry.status >= UMID_HEALTH_FAULT) {
            reason = UAPP_SUPERVISOR_REASON_REPORTED_FAULT;
        } else if (supervisor->cfg.stale_after_ms != 0u && age_us > stale_after_us) {
            reason = UAPP_SUPERVISOR_REASON_STALE_REPORT;
            effective_status = UMID_HEALTH_FAULT;
        }
        if (reason == UAPP_SUPERVISOR_REASON_NONE) {
            supervisor->fault_counts[id] = 0u;
            supervisor->active_reasons[id] = UAPP_SUPERVISOR_REASON_NONE;
            continue;
        }
        /* A transition between an explicit fault and a missed lease starts a
         * fresh persistence count; one kind of failure cannot inherit the
         * debounce accumulated for another.
         */
        if (supervisor->active_reasons[id] != reason) {
            supervisor->fault_counts[id] = 0u;
            supervisor->active_reasons[id] = reason;
        }
        if (supervisor->fault_counts[id] < supervisor->cfg.fault_threshold) {
            ++supervisor->fault_counts[id];
            if (supervisor->fault_counts[id] == supervisor->cfg.fault_threshold) {
                const uapp_supervisor_event_t event = {
                    .id = id,
                    .reported_status = entry.status,
                    .effective_status = effective_status,
                    .reason = reason,
                    .fault_code = entry.fault_code,
                    .report_age_us = age_us,
                };
                if (supervisor->cfg.on_event != NULL) { supervisor->cfg.on_event(&event, supervisor->cfg.ctx); }
                if (supervisor->cfg.on_fault != NULL) { supervisor->cfg.on_fault(id, effective_status, supervisor->cfg.ctx); }
            }
        }
    }
}
