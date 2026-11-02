/// @file src/uproto/network/time_sync.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "uproto/network/time_sync.h"

uetd_status_t uproto_time_sync_init(uproto_time_sync_t* sync, const uproto_time_sync_config_t* config) {
    /* TODO: Select one implemented provider profile, timestamp capability, trust and holdover policy. */
    (void)sync;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_time_sync_poll(uproto_time_sync_t* sync, uetd_u16_t work_budget) {
    /* TODO: Bound exchange/filter work and track source changes, delay asymmetry and measurement uncertainty. */
    (void)sync;
    (void)work_budget;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_time_sync_mapping(const uproto_time_sync_t* sync, uproto_time_mapping_t* mapping) {
    /* TODO: Publish local-to-remote mapping quality/age without stepping the monotonic deadline clock. */
    (void)sync;
    (void)mapping;
    return UETD_NOT_SUPPORTED;
}

