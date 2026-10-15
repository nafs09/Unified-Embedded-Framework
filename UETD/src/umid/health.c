/// @file src/umid/health.c
/// @brief Fixed-capacity health registry for application supervision.

#include "umid/health.h"
#include "ucore/time.h"
#include "uhal/irq.h"
#include <string.h>

static umid_health_entry_t g_entries[UMID_HEALTH_MAX_ENTRIES];
static uetd_u8_t g_count;

/* The registry stores caller-owned names by pointer; use string literals or static storage. */
uetd_u8_t umid_health_register(const char* name) {
    if (name == NULL || name[0] == '\0') { return UMID_HEALTH_INVALID_ID; }
    const uhal_critical_t critical_state = uhal_critical_enter();
    for (uetd_u8_t index = 0u; index < g_count; ++index) {
        if (strcmp(g_entries[index].name, name) == 0) {
            uhal_critical_exit(critical_state);
            return index;
        }
    }
    if (g_count >= UMID_HEALTH_MAX_ENTRIES) {
        uhal_critical_exit(critical_state);
        return UMID_HEALTH_INVALID_ID;
    }
    const uetd_u8_t id = g_count++;
    g_entries[id].name = name; /* Caller keeps this name alive for the system. */
    g_entries[id].status = UMID_HEALTH_OK;
    g_entries[id].fault_code = 0u;
    g_entries[id].last_update_us = uetd_time_now_us();
    uhal_critical_exit(critical_state);
    return id;
}

void umid_health_report(uetd_u8_t id, umid_health_status_t status, uetd_u32_t fault_code) {
    if (status < UMID_HEALTH_OK || status > UMID_HEALTH_FAIL) { return; }
    /* Keep status, code, and timestamp as one logical update for concurrent readers. */
    const uhal_critical_t critical_state = uhal_critical_enter();
    if (id >= g_count) {
        uhal_critical_exit(critical_state);
        return;
    }
    g_entries[id].status = status;
    g_entries[id].fault_code = fault_code;
    g_entries[id].last_update_us = uetd_time_now_us();
    uhal_critical_exit(critical_state);
}

umid_health_status_t umid_health_system_status(void) {
    /* Enum severity ordering lets the maximum entry status represent aggregate health. */
    umid_health_status_t result = UMID_HEALTH_OK;
    const uhal_critical_t critical_state = uhal_critical_enter();
    for (uetd_u8_t index = 0u; index < g_count; ++index) {
        if (g_entries[index].status > result) { result = g_entries[index].status; }
    }
    uhal_critical_exit(critical_state);
    return result;
}

const umid_health_entry_t* umid_health_entry(uetd_u8_t id) {
    /* Legacy borrowed view: callers must serialize reads against health reports. */
    return id < g_count ? &g_entries[id] : NULL;
}

bool umid_health_entry_copy(uetd_u8_t id, umid_health_entry_t* out) {
    if (out == NULL) { return false; }
    const uhal_critical_t critical_state = uhal_critical_enter();
    if (id >= g_count) {
        uhal_critical_exit(critical_state);
        return false;
    }
    *out = g_entries[id];
    uhal_critical_exit(critical_state);
    return true;
}

uetd_u8_t umid_health_count(void) {
    const uhal_critical_t critical_state = uhal_critical_enter();
    const uetd_u8_t count = g_count;
    uhal_critical_exit(critical_state);
    return count;
}

bool umid_health_entry_is_fresh(uetd_u8_t id, uetd_u64_t now_us, uetd_u64_t maximum_age_us) {
    umid_health_entry_t entry;
    if (!umid_health_entry_copy(id, &entry) || now_us < entry.last_update_us) { return false; }
    return now_us - entry.last_update_us <= maximum_age_us;
}
