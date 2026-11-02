/// @file include/umid/logging/blackbox.h
/// @brief First-pass UMID binary acquisition logging contract.

#ifndef UMID_BLACKBOX_H
#define UMID_BLACKBOX_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* id;
    const char* type_id;
    const char* units_id;
    uetd_u32_t rate_millihz;
    uetd_u16_t maximum_encoded_bytes;
} umid_blackbox_channel_t;

typedef enum {
    UMID_BLACKBOX_DROP_NEWEST = 0,
    UMID_BLACKBOX_DROP_OLDEST,
    UMID_BLACKBOX_DECIMATE,
    UMID_BLACKBOX_STOP_ON_FULL,
} umid_blackbox_overflow_policy_t;

typedef struct {
    uetd_status_t (*write)(void* context, const uetd_u8_t* bytes, uetd_u32_t length, uetd_u32_t* written);
    uetd_status_t (*sync)(void* context);
    void* context;
    uetd_u32_t maximum_write_stall_us;
} umid_blackbox_backend_t;

typedef struct {
    const umid_blackbox_channel_t* channels;
    uetd_u16_t channel_count;
    /* The caller supplies the bounded queue storage for the recording lifetime. */
    uetd_u8_t* queue_storage;
    uetd_u32_t queue_capacity_bytes;
    uetd_u64_t usable_storage_bytes;
    uetd_u64_t worst_storage_stall_us;
    uetd_u64_t burst_margin_bytes;
    uetd_u64_t sustained_write_bytes_per_second;
    umid_blackbox_overflow_policy_t overflow_policy;
    const umid_blackbox_backend_t* backend;
} umid_blackbox_config_t;

typedef struct {
    uetd_u64_t records_written;
    uetd_u64_t records_dropped;
    uetd_u64_t bytes_written;
    uetd_u64_t storage_stalls;
    uetd_u64_t last_committed_sequence;
    uetd_status_t last_storage_error;
} umid_blackbox_stats_t;

typedef struct {
    umid_blackbox_config_t config;
    umid_blackbox_stats_t stats;
    uetd_u32_t queue_read_offset;
    uetd_u32_t queue_write_offset;
    uetd_u32_t queue_used_bytes;
    uetd_u32_t next_sequence;
    bool active;
} umid_blackbox_t;

enum {
    UMID_BLACKBOX_FLAG_DISCONTINUITY = 1u << 0,
    UMID_BLACKBOX_FLAG_INVALID_SAMPLE = 1u << 1,
    UMID_BLACKBOX_FLAG_TIME_RESET = 1u << 2,
    UMID_BLACKBOX_FLAG_OVERFLOW = 1u << 3,
};

typedef struct {
    uetd_u64_t timestamp_us;
    uetd_u32_t sequence;
    uetd_u16_t channel_id;
    uetd_u16_t payload_length;
    const uetd_u8_t* payload;
    uetd_u16_t flags;
    uetd_u32_t payload_crc32;
} umid_blackbox_record_t;

/// Validate the configuration for the blackbox.
uetd_status_t umid_blackbox_validate(const umid_blackbox_config_t* config, uetd_u64_t* required_queue_bytes,
                                    uetd_u64_t* estimated_retention_seconds);
/* start borrows the immutable channel array, queue buffer, and backend through
 * stop; append consumes the record payload synchronously or copies it into
 * caller-provided bounded queue storage. */
/// Start bounded blackbox recording with the supplied channel configuration.
uetd_status_t umid_blackbox_start(umid_blackbox_t* blackbox, const umid_blackbox_config_t* config);
/// Append a timestamped sample record to the blackbox queue.
uetd_status_t umid_blackbox_append(umid_blackbox_t* blackbox, const umid_blackbox_record_t* record);
/// Write up to the requested number of queued blackbox bytes to storage.
uetd_status_t umid_blackbox_flush(umid_blackbox_t* blackbox, uetd_u32_t maximum_bytes, uetd_u32_t* bytes_written);
/// Copy the blackbox record, byte, drop, and storage-stall counters.
uetd_status_t umid_blackbox_get_stats(const umid_blackbox_t* blackbox, umid_blackbox_stats_t* stats);
/// Stop blackbox recording and finish pending storage writes.
uetd_status_t umid_blackbox_stop(umid_blackbox_t* blackbox);

#ifdef __cplusplus
}
#endif

#endif /* UMID_BLACKBOX_H */