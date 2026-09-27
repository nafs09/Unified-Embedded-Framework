/// @file src/uapp/supervisor.c
/// @brief Threshold-based response to sustained subsystem health faults.

#include "uef/uapp/uapp_supervisor.h"
#include "uef/ucore/uef_time.h"

#include <string.h>

void uapp_supervisor_init(uapp_supervisor_t* supervisor,
                          const uapp_supervisor_cfg_t* config) {
    if (supervisor == NULL) {
        return;
    }
    memset(supervisor, 0, sizeof(*supervisor));
    if (config != NULL) {
        supervisor->cfg = *config;
    }
}

void uapp_supervisor_tick(uapp_supervisor_t* supervisor) {
    if (supervisor == NULL || supervisor->cfg.fault_threshold == 0u) {
        return;
    }

    /* This polling API is cooperative: callers choose its context and must invoke it regularly. */
    const uef_u64_t now_us = uef_time_now_us();
    const uef_u64_t check_period_us = (uef_u64_t)supervisor->cfg.check_period_ms * 1000u;
    if (supervisor->last_check_us != 0u &&
        now_us - supervisor->last_check_us < check_period_us) {
        return;
    }
    supervisor->last_check_us = now_us;

    /* The fault counter saturates at the threshold and calls back only on the crossing tick. */
    const uef_u8_t count = umid_health_count();
    for (uef_u8_t id = 0u; id < count && id < 32u; ++id) {
        const umid_health_entry_t* entry = umid_health_entry(id);
        if (entry == NULL || entry->status < UMID_HEALTH_FAULT) {
            supervisor->fault_counts[id] = 0u;
            continue;
        }

        if (supervisor->fault_counts[id] < supervisor->cfg.fault_threshold) {
            ++supervisor->fault_counts[id];
            if (supervisor->fault_counts[id] == supervisor->cfg.fault_threshold &&
                supervisor->cfg.on_fault != NULL) {
                supervisor->cfg.on_fault(id, entry->status, supervisor->cfg.ctx);
            }
        }
    }
}
