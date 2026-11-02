/// @file src/umid/logging/blackbox.c
/// @brief Fail-closed entry points for the first-pass UMID binary logging service.
#include "umid/logging/blackbox.h"

uetd_status_t umid_blackbox_validate(const umid_blackbox_config_t* config,
                                     uetd_u64_t* required_queue_bytes,
                                     uetd_u64_t* estimated_retention_seconds) {
    (void)config;
    (void)required_queue_bytes;
    (void)estimated_retention_seconds;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_blackbox_start(umid_blackbox_t* blackbox, const umid_blackbox_config_t* config) {
    (void)blackbox;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_blackbox_append(umid_blackbox_t* blackbox, const umid_blackbox_record_t* record) {
    (void)blackbox;
    (void)record;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_blackbox_flush(umid_blackbox_t* blackbox, uetd_u32_t maximum_bytes,
                                  uetd_u32_t* bytes_written) {
    (void)blackbox;
    (void)maximum_bytes;
    (void)bytes_written;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_blackbox_get_stats(const umid_blackbox_t* blackbox, umid_blackbox_stats_t* stats) {
    (void)blackbox;
    (void)stats;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_blackbox_stop(umid_blackbox_t* blackbox) {
    (void)blackbox;
    return UETD_NOT_SUPPORTED;
}