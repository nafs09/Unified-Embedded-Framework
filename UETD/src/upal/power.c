/// @file src/upal/power.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "upal/power.h"

uetd_status_t upal_power_init(upal_power_t* power, const upal_power_config_t* config) {
    /* TODO: Bind exact retention/wake/timer/rail facts and bounded lease storage. */
    (void)power;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_power_acquire(upal_power_t* power, upal_power_mode_t deepest_allowed, upal_power_lease_t* lease) {
    /* TODO: Reserve a generation-tagged veto before a transfer/session starts; fail explicitly on exhaustion. */
    (void)power;
    (void)deepest_allowed;
    (void)lease;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_power_release(upal_power_t* power, upal_power_lease_t lease) {
    /* TODO: Reject stale/double release without dropping another owner power veto. */
    (void)power;
    (void)lease;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_power_enter(upal_power_t* power, upal_power_mode_t mode, ucore_deadline_t next_wake) {
    /* TODO: Validate outstanding DMA/session leases, wake latency, retained state and clock continuity before sleep. */
    (void)power;
    (void)mode;
    (void)next_wake;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_power_get_status(upal_power_t* power, upal_power_status_t* status) {
    /* TODO: Report measured validity and timestamp; never fabricate battery charge or power-good values. */
    (void)power;
    (void)status;
    return UETD_NOT_SUPPORTED;
}

