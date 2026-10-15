/// @file src/uapp/watchdog_gate.c
/// @brief Refreshes a watchdog only while its declared health leases are valid.

#include "uapp/watchdog_gate.h"
#include "ucore/time.h"
#include "uhal/irq.h"
#include <string.h>

static bool event_changed(const uapp_watchdog_gate_t* gate, const uapp_watchdog_gate_event_t* event) {
    return gate->event_pending || gate->last_reason != event->reason || gate->last_health_id != event->health_id ||
           gate->last_observed_status != event->observed_status || gate->last_fault_code != event->fault_code ||
           gate->last_refresh_status != event->refresh_status;
}

uetd_status_t uapp_watchdog_gate_init(uapp_watchdog_gate_t* gate, const uapp_watchdog_gate_cfg_t* config) {
    if (gate == NULL || config == NULL || config->required_leases == NULL || config->required_lease_count == 0u ||
        config->required_lease_count > UMID_HEALTH_MAX_ENTRIES || config->refresh == NULL) { return UETD_INVALID_ARG; }
    for (uetd_u16_t i = 0u; i < config->required_lease_count; ++i) {
        const uapp_watchdog_lease_t* lease = &config->required_leases[i];
        umid_health_entry_t entry;
        if (lease->maximum_age_us == 0u || lease->maximum_accepted_status < UMID_HEALTH_OK ||
            lease->maximum_accepted_status > UMID_HEALTH_FAIL || !umid_health_entry_copy(lease->health_id, &entry)) {
            return UETD_INVALID_ARG;
        }
        for (uetd_u16_t prior = 0u; prior < i; ++prior) {
            if (config->required_leases[prior].health_id == lease->health_id) { return UETD_CONFLICT; }
        }
    }
    memset(gate, 0, sizeof(*gate));
    memcpy(gate->leases, config->required_leases, (size_t)config->required_lease_count * sizeof(gate->leases[0]));
    gate->lease_count = config->required_lease_count;
    gate->boot_started_us = uetd_time_now_us();
    gate->refresh = config->refresh;
    gate->on_event = config->on_event;
    gate->context = config->context;
    gate->last_reason = UAPP_WDG_GATE_LATCHED;
    gate->last_health_id = UMID_HEALTH_INVALID_ID;
    gate->last_refresh_status = UETD_NOT_READY;
    gate->event_pending = true;
    gate->initialized = true;
    return UETD_OK;
}

static uapp_watchdog_gate_event_t check_leases(const uapp_watchdog_gate_t* gate, uetd_u64_t now_us) {
    uapp_watchdog_gate_event_t event = {
        .reason = UAPP_WDG_GATE_HEALTHY,
        .health_id = UMID_HEALTH_INVALID_ID,
        .observed_status = UMID_HEALTH_OK,
        .fault_code = 0u,
        .age_us = 0u,
        .refresh_status = UETD_OK,
        .feed_permitted = true,
    };
    for (uetd_u16_t index = 0u; index < gate->lease_count; ++index) {
        const uapp_watchdog_lease_t* lease = &gate->leases[index];
        umid_health_entry_t entry;
        if (!umid_health_entry_copy(lease->health_id, &entry)) {
            event.reason = UAPP_WDG_GATE_MISSING_HEALTH_ID;
            event.health_id = lease->health_id;
            event.observed_status = UMID_HEALTH_FAIL;
            event.feed_permitted = false;
            return event;
        }
        event.age_us = now_us >= entry.last_update_us ? now_us - entry.last_update_us : ~(uetd_u64_t)0u;
        if (event.age_us > lease->maximum_age_us) {
            event.reason = UAPP_WDG_GATE_STALE_LEASE;
            event.health_id = lease->health_id;
            event.observed_status = entry.status;
            event.fault_code = entry.fault_code;
            event.feed_permitted = false;
            return event;
        }
        if (entry.status > lease->maximum_accepted_status) {
            event.reason = UAPP_WDG_GATE_UNHEALTHY_LEASE;
            event.health_id = lease->health_id;
            event.observed_status = entry.status;
            event.fault_code = entry.fault_code;
            event.feed_permitted = false;
            return event;
        }
    }
    return event;
}

uetd_status_t uapp_watchdog_gate_tick(uapp_watchdog_gate_t* gate) {
    if (gate == NULL || !gate->initialized) { return UETD_INVALID_ARG; }
    const uetd_u64_t now_us = uetd_time_now_us();
    uapp_watchdog_gate_event_t event = {
        .reason = UAPP_WDG_GATE_HEALTHY,
        .health_id = UMID_HEALTH_INVALID_ID,
        .observed_status = UMID_HEALTH_OK,
        .fault_code = 0u,
        .age_us = 0u,
        .refresh_status = UETD_OK,
        .feed_permitted = false,
    };
    const uhal_critical_t critical_state = uhal_critical_enter();
    const bool latched = gate->latched;
    const uetd_u32_t latched_code = gate->latched_fault_code;
    uhal_critical_exit(critical_state);
    if (latched) {
        event.reason = UAPP_WDG_GATE_LATCHED;
        event.health_id = UMID_HEALTH_INVALID_ID;
        event.fault_code = latched_code;
        event.refresh_status = UETD_NOT_READY;
    } else if (now_us < gate->boot_started_us) {
        event.reason = UAPP_WDG_GATE_STALE_LEASE;
        event.health_id = UMID_HEALTH_INVALID_ID;
        event.observed_status = UMID_HEALTH_FAIL;
        event.refresh_status = UETD_ERROR;
    } else {
        const uetd_u64_t elapsed_us = now_us - gate->boot_started_us;
        /* Startup grace permits feeding while drivers/tasks publish their
         * first leases; it ends once and cannot restart after a fault.
         */
        if (elapsed_us < gate->boot_grace_us) {
            event.reason = UAPP_WDG_GATE_BOOT_GRACE;
            event.feed_permitted = true;
        } else {
            event = check_leases(gate, now_us);
        }
        if (event.feed_permitted) {
            event.refresh_status = gate->refresh(gate->context);
            if (event.refresh_status != UETD_OK) {
                event.reason = UAPP_WDG_GATE_REFRESH_FAILED;
                event.feed_permitted = false;
            }
        }
    }
    const bool changed = event_changed(gate, &event);
    gate->last_reason = event.reason;
    gate->last_health_id = event.health_id;
    gate->last_observed_status = event.observed_status;
    gate->last_fault_code = event.fault_code;
    gate->last_refresh_status = event.refresh_status;
    gate->event_pending = false;
    if (changed && gate->on_event != NULL) { gate->on_event(&event, gate->context); }
    if (event.reason == UAPP_WDG_GATE_REFRESH_FAILED) { return event.refresh_status; }
    return event.feed_permitted ? UETD_OK : UETD_NOT_READY;
}

void uapp_watchdog_gate_latch(uapp_watchdog_gate_t* gate, uetd_u32_t fault_code) {
    if (gate == NULL) { return; }
    const uhal_critical_t critical_state = uhal_critical_enter();
    gate->latched = true;
    gate->latched_fault_code = fault_code;
    uhal_critical_exit(critical_state);
}

bool uapp_watchdog_gate_is_latched(const uapp_watchdog_gate_t* gate) {
    if (gate == NULL) { return false; }
    const uhal_critical_t critical_state = uhal_critical_enter();
    const bool latched = gate->latched;
    uhal_critical_exit(critical_state);
    return latched;
}
