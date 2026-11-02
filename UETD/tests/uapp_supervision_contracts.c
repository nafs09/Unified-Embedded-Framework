#include "uos_uapp_test.h"
#include "uapp/uapp.h"
#include "ucore/time.h"
#include "uhal/irq.h"
static uetd_u64_t now_us;
uetd_time_us_t uetd_time_now_us(void) { return now_us; }
uhal_critical_t uhal_critical_enter(void) { return 0; }
void uhal_critical_exit(uhal_critical_t saved) { (void)saved; }
static unsigned events, feeds;
static uapp_supervisor_t supervisor;
static void supervisor_event(const uapp_supervisor_event_t* event, void* context) {
    (void)context; ++events; CHECK(event->effective_status >= UMID_HEALTH_FAULT); uapp_supervisor_tick(&supervisor);
}
static uapp_watchdog_gate_t gate;
static uetd_status_t feed(void* context) {
    (void)context; ++feeds; CHECK(uapp_watchdog_gate_tick(&gate) == UETD_BUSY); return UETD_OK;
}
int main(void) {
    const uetd_u8_t id = umid_health_register("worker"); CHECK(id != UMID_HEALTH_INVALID_ID);
    CHECK(!umid_health_entry_is_fresh(id, now_us, 1000));
    const uapp_watchdog_lease_t initial_lease = {id, UMID_HEALTH_WARN, 1000};
    const uapp_watchdog_gate_cfg_t initial_gate = {&initial_lease, 1, 10, feed, NULL, NULL};
    CHECK(uapp_watchdog_gate_init(&gate, &initial_gate) == UETD_OK);
    CHECK(uapp_watchdog_gate_tick(&gate) == UETD_OK && feeds == 1);
    now_us = 10;
    CHECK(uapp_watchdog_gate_tick(&gate) == UETD_NOT_READY && feeds == 1);
    CHECK(gate.last_reason == UAPP_WDG_GATE_UNREPORTED_LEASE);
    now_us = 0; feeds = 0;
    const uapp_supervisor_cfg_t cfg = {1, 2, 2, NULL, supervisor_event, NULL};
    uapp_supervisor_init(&supervisor, &cfg);
    umid_health_report(id, UMID_HEALTH_FAULT, 42);
    CHECK(umid_health_entry_is_fresh(id, now_us, 1000));
    uapp_supervisor_tick(&supervisor); uapp_supervisor_tick(&supervisor);
    CHECK(events == 0 && supervisor.fault_counts[id] == 1); /* timestamp zero is a real check */
    now_us = 1000; uapp_supervisor_tick(&supervisor); CHECK(events == 1);
    now_us = 2000; uapp_supervisor_tick(&supervisor); CHECK(events == 1);
    umid_health_report(id, UMID_HEALTH_OK, 0); now_us = 3000; uapp_supervisor_tick(&supervisor);
    CHECK(supervisor.fault_counts[id] == 0);
    now_us = 5001; uapp_supervisor_tick(&supervisor); CHECK(supervisor.fault_counts[id] == 1);
    now_us = 6001; uapp_supervisor_tick(&supervisor); CHECK(events == 2);
    const uapp_watchdog_lease_t lease = {id, UMID_HEALTH_WARN, 1000};
    const uapp_watchdog_gate_cfg_t gate_cfg = {&lease, 1, 0, feed, NULL, NULL};
    umid_health_report(id, UMID_HEALTH_OK, 0);
    CHECK(uapp_watchdog_gate_init(&gate, &gate_cfg) == UETD_OK);
    CHECK(uapp_watchdog_gate_tick(&gate) == UETD_OK && feeds == 1);
    now_us += 1001; CHECK(uapp_watchdog_gate_tick(&gate) == UETD_NOT_READY && feeds == 1);
    umid_health_report(id, UMID_HEALTH_FAIL, 42);
    CHECK(uapp_watchdog_gate_tick(&gate) == UETD_NOT_READY);
    uapp_watchdog_gate_latch(&gate, 42); uapp_watchdog_gate_latch(&gate, 43);
    CHECK(gate.latched_fault_code == 42 && uapp_watchdog_gate_is_latched(&gate));
    umid_health_report(id, UMID_HEALTH_OK, 0);
    CHECK(uapp_watchdog_gate_tick(&gate) == UETD_NOT_READY && feeds == 1);
    puts("Supervision cadence, stale leases, reentrancy and sticky faults passed"); return 0;
}
