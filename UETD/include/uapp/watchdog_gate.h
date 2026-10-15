/// @file include/uapp/watchdog_gate.h
/// @brief Health-lease gate for one hardware watchdog instance.

#ifndef UAPP_WATCHDOG_GATE_H
#define UAPP_WATCHDOG_GATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "umid/health.h"

typedef enum {
    UAPP_WDG_GATE_HEALTHY = 0,
    UAPP_WDG_GATE_BOOT_GRACE,
    UAPP_WDG_GATE_MISSING_HEALTH_ID,
    UAPP_WDG_GATE_STALE_LEASE,
    UAPP_WDG_GATE_UNHEALTHY_LEASE,
    UAPP_WDG_GATE_REFRESH_FAILED,
    UAPP_WDG_GATE_LATCHED,
} uapp_watchdog_gate_reason_t;

typedef struct {
    uetd_u8_t health_id;
    umid_health_status_t maximum_accepted_status; /* usually WARN */
    uetd_u64_t maximum_age_us;
} uapp_watchdog_lease_t;

typedef struct {
    uapp_watchdog_gate_reason_t reason;
    uetd_u8_t health_id; /* UMID_HEALTH_INVALID_ID if not lease-specific */
    umid_health_status_t observed_status;
    uetd_u32_t fault_code;
    uetd_u64_t age_us;
    uetd_status_t refresh_status;
    bool feed_permitted;
} uapp_watchdog_gate_event_t;

typedef uetd_status_t (*uapp_watchdog_refresh_fn)(void* context);
typedef void (*uapp_watchdog_gate_event_fn)(const uapp_watchdog_gate_event_t* event, void* context);

typedef struct {
    const uapp_watchdog_lease_t* required_leases;
    uetd_u8_t required_lease_count;
    uetd_u64_t boot_grace_us;
    uapp_watchdog_refresh_fn refresh;
    uapp_watchdog_gate_event_fn on_event;
    void* context;
} uapp_watchdog_gate_cfg_t;

typedef struct {
    uapp_watchdog_lease_t leases[UMID_HEALTH_MAX_ENTRIES];
    uetd_u8_t lease_count;
    uetd_u64_t boot_started_us;
    uapp_watchdog_refresh_fn refresh;
    uapp_watchdog_gate_event_fn on_event;
    void* context;
    uetd_u32_t latched_fault_code;
    uapp_watchdog_gate_reason_t last_reason;
    uetd_u8_t last_health_id;
    umid_health_status_t last_observed_status;
    uetd_u32_t last_fault_code;
    uetd_status_t last_refresh_status;
    bool initialized;
    bool latched;
    bool event_pending;
} uapp_watchdog_gate_t;

/* The caller owns the watchdog's UPAL instance and supplies its refresh method.
 * Route all refreshes for that watchdog through this gate. The gate allows
 * refresh during boot grace, then only while every required lease is current
 * and no worse than its configured status ceiling. Call tick from one periodic
 * task/main loop, never from the watchdog ISR.
 */
uetd_status_t uapp_watchdog_gate_init(uapp_watchdog_gate_t* gate, const uapp_watchdog_gate_cfg_t* config);
uetd_status_t uapp_watchdog_gate_tick(uapp_watchdog_gate_t* gate);

/* ISR-safe latch: blocks refresh on subsequent gate ticks. A tick that already
 * passed its lease/latch authorization check may still perform one refresh if
 * the latch races that tick. The normal tick path publishes the LATCHED event
 * in task context, and hardware reset remains watchdog-owned.
 */
void uapp_watchdog_gate_latch(uapp_watchdog_gate_t* gate, uetd_u32_t fault_code);
bool uapp_watchdog_gate_is_latched(const uapp_watchdog_gate_t* gate);

#ifdef __cplusplus
}
#endif

#endif /* UAPP_WATCHDOG_GATE_H */
