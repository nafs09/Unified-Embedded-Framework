/// @file include/umid/future/data_services.h
/// @brief Design-only typed outlines for deferred UMID storage/data services.
///
/// Device-specific sensor drivers remain consumer authored. This header is
/// not included by umid.h and its functions are not implemented/selectable.

#ifndef UMID_FUTURE_DATA_SERVICES_H
#define UMID_FUTURE_DATA_SERVICES_H

#include "ucore/status.h"
#include "ucore/types.h"

typedef enum {
    UMID_STORE_CONFIGURATION = 0,
    UMID_STORE_CALIBRATION,
    UMID_STORE_FAULT_SUMMARY,
} umid_store_record_kind_t;

/* Every text field uses a fixed byte capacity and explicit byte length. The
 * store copies metadata; it never retains caller pointers after a call. */
#define UMID_STORE_KEY_CAPACITY 64u
#define UMID_STORE_SCHEMA_ID_CAPACITY 48u
#define UMID_STORE_OWNER_ID_CAPACITY 48u
#define UMID_STORE_TARGET_ID_CAPACITY 48u
#define UMID_STORE_DEVICE_ID_CAPACITY 64u
#define UMID_STORE_ENCODING_ID_CAPACITY 24u

typedef struct {
    char bytes[UMID_STORE_KEY_CAPACITY];
    uetd_u16_t length;
} umid_store_key_t;

typedef struct {
    char schema_id[UMID_STORE_SCHEMA_ID_CAPACITY];
    uetd_u16_t schema_id_length;
    uetd_u16_t schema_version;
    umid_store_record_kind_t kind;
    char owner_id[UMID_STORE_OWNER_ID_CAPACITY];
    uetd_u16_t owner_id_length;
    char target_id[UMID_STORE_TARGET_ID_CAPACITY];
    uetd_u16_t target_id_length;
    char device_identity[UMID_STORE_DEVICE_ID_CAPACITY];
    uetd_u16_t device_identity_length;
    char encoding_id[UMID_STORE_ENCODING_ID_CAPACITY];
    uetd_u16_t encoding_id_length;
    uetd_u32_t sequence;
    uetd_u32_t payload_length;
    uetd_u8_t payload_sha256[32];
    uetd_u32_t payload_crc32;
    const uetd_u8_t* payload;
} umid_store_record_t;

typedef struct {
    /* begin reserves an inactive generation; read only exposes committed data. */
    uetd_status_t (*begin_generation)(void* context, const umid_store_key_t* key, uetd_u32_t generation, uetd_u32_t total_length);
    uetd_status_t (*read)(void* context, const umid_store_key_t* key, uetd_u32_t offset, uetd_u8_t* data, uetd_u32_t length);
    uetd_status_t (*write)(void* context, const umid_store_key_t* key, uetd_u32_t generation, uetd_u32_t offset,
                          const uetd_u8_t* data, uetd_u32_t length);
    uetd_status_t (*commit_generation)(void* context, const umid_store_key_t* key, uetd_u32_t generation, uetd_u32_t total_length,
                                      uetd_u32_t payload_crc32, const uetd_u8_t payload_sha256[32]);
    uetd_status_t (*abort_generation)(void* context, const umid_store_key_t* key, uetd_u32_t generation);
    void* context;
    uetd_u32_t maximum_record_bytes;
    uetd_u32_t required_alignment;
    uetd_u32_t atomic_commit_bytes;
    uetd_u32_t maximum_write_stall_us;
} umid_store_backend_t;

typedef struct {
    umid_store_backend_t backend;
    bool initialized;
    bool transaction_active;
    umid_store_key_t active_key; /* single serialized transaction per store */
    uetd_u32_t active_generation;
} umid_store_t;

/* On read, metadata->payload aliases the caller's payload buffer and remains
 * valid only for that caller-owned buffer's lifetime. */
uetd_status_t umid_store_init(umid_store_t* store, const umid_store_backend_t* backend);
uetd_status_t umid_store_read(umid_store_t* store, const umid_store_key_t* key, uetd_u8_t* payload, uetd_u32_t capacity,
                             uetd_u32_t* payload_length, umid_store_record_t* metadata);
uetd_status_t umid_store_write_atomic(umid_store_t* store, const umid_store_key_t* key, const umid_store_record_t* record);
uetd_status_t umid_store_erase(umid_store_t* store, const umid_store_key_t* key);

#define UMID_CAL_DRIVER_ID_CAPACITY 48u
#define UMID_CAL_SENSOR_ID_CAPACITY 64u
#define UMID_CAL_BOARD_ID_CAPACITY 48u
#define UMID_CAL_UNITS_ID_CAPACITY 24u
#define UMID_CAL_METHOD_ID_CAPACITY 32u

typedef struct {
    char driver_id[UMID_CAL_DRIVER_ID_CAPACITY];
    uetd_u16_t driver_id_length;
    char sensor_identity[UMID_CAL_SENSOR_ID_CAPACITY];
    uetd_u16_t sensor_identity_length;
    char board_id[UMID_CAL_BOARD_ID_CAPACITY];
    uetd_u16_t board_id_length;
    char units_id[UMID_CAL_UNITS_ID_CAPACITY];
    uetd_u16_t units_id_length;
    uetd_u16_t calibration_schema_version;
    char method_id[UMID_CAL_METHOD_ID_CAPACITY];
    uetd_u16_t method_id_length;
    uetd_u16_t method_revision;
    uetd_u32_t clock_id;
    uetd_u64_t calibrated_at_us;
    uetd_u32_t payload_length;
    const uetd_u8_t* opaque_driver_payload;
    uetd_u32_t payload_crc32;
    uetd_u8_t payload_sha256[32];
} umid_calibration_record_t;

uetd_status_t umid_calibration_store_save(umid_store_t* store, const umid_calibration_record_t* record);
uetd_status_t umid_calibration_store_load(umid_store_t* store, const char* driver_id, const char* sensor_identity,
                                         uetd_u16_t expected_schema_version, uetd_u8_t* payload, uetd_u32_t capacity,
                                         uetd_u32_t* payload_length);

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
    const umid_blackbox_channel_t* channels;
    uetd_u16_t channel_count;
    uetd_u64_t queue_capacity_bytes;
    uetd_u64_t usable_storage_bytes;
    uetd_u64_t worst_storage_stall_us;
    uetd_u64_t burst_margin_bytes;
    uetd_u64_t sustained_write_bytes_per_second;
    umid_blackbox_overflow_policy_t overflow_policy;
} umid_blackbox_config_t;

typedef struct {
    uetd_u64_t records_written;
    uetd_u64_t records_dropped;
    uetd_u64_t bytes_written;
    uetd_u64_t storage_stalls;
    uetd_u64_t last_committed_sequence;
    uetd_status_t last_storage_error;
} umid_blackbox_stats_t;

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

uetd_status_t umid_blackbox_validate(const umid_blackbox_config_t* config, uetd_u64_t* required_queue_bytes,
                                    uetd_u64_t* estimated_retention_seconds);
/* start borrows the immutable channel array until stop; append consumes the
 * record payload synchronously or copies it into owned bounded storage. */
uetd_status_t umid_blackbox_start(const umid_blackbox_config_t* config);
uetd_status_t umid_blackbox_append(const umid_blackbox_record_t* record);
uetd_status_t umid_blackbox_flush(uetd_u32_t maximum_bytes, uetd_u32_t* bytes_written);
uetd_status_t umid_blackbox_get_stats(umid_blackbox_stats_t* stats);
uetd_status_t umid_blackbox_stop(void);

typedef enum {
    UMID_PROCESS_QUALITY_GOOD = 0,
    UMID_PROCESS_QUALITY_UNCERTAIN,
    UMID_PROCESS_QUALITY_BAD,
    UMID_PROCESS_QUALITY_STALE,
} umid_process_quality_t;

typedef enum {
    UMID_PROCESS_DIRECTION_INPUT = 0,
    UMID_PROCESS_DIRECTION_OUTPUT,
    UMID_PROCESS_DIRECTION_INPUT_OUTPUT,
} umid_process_direction_t;

typedef enum {
    UMID_PROCESS_AUTHORITY_LOCAL_ONLY = 0,
    UMID_PROCESS_AUTHORITY_PEER_COMMAND,
    UMID_PROCESS_AUTHORITY_LOCAL_ARBITRATION,
} umid_process_authority_t;

typedef enum {
    UMID_PROCESS_COMMAND_LEVEL_HELD = 0,
    UMID_PROCESS_COMMAND_EDGE_TRIGGERED,
} umid_process_command_semantics_t;

typedef struct {
    uetd_u16_t point_id;
    const char* data_type_id;
    const char* units_id;
    const char* owner_id;
    const char* timestamp_domain_id;
    umid_process_direction_t direction;
    umid_process_authority_t write_authority;
    umid_process_command_semantics_t command_semantics;
    uetd_u32_t update_period_ms;
    uetd_u32_t stale_after_ms;
    uetd_u32_t maximum_encoded_bytes;
    const uetd_u8_t* minimum_encoded_value;
    const uetd_u8_t* maximum_encoded_value;
    const uetd_u8_t* safe_encoded_value;
    bool requires_acknowledgement;
} umid_process_point_schema_t;

typedef struct {
    uetd_u16_t point_id;
    umid_process_direction_t direction;
    umid_process_quality_t quality;
    uetd_u64_t source_timestamp_us;
    uetd_u32_t payload_length;
    const uetd_u8_t* payload;
} umid_process_point_t;

typedef struct {
    uetd_u32_t generation;
    uetd_u64_t committed_at_us;
    uetd_u16_t point_count;
    const umid_process_point_t* points;
} umid_process_scan_t;

typedef struct {
    uetd_u16_t maximum_points;
    uetd_u32_t maximum_payload_bytes;
    uetd_u32_t stale_after_ms;
    uetd_u32_t maximum_scan_commit_us;
    const umid_process_point_schema_t* point_schema;
    uetd_u16_t point_schema_count;
} umid_process_image_config_t;

uetd_status_t umid_process_image_init(const umid_process_image_config_t* config, umid_process_point_t* staging_points,
                                     uetd_u8_t* staging_payload, umid_process_point_t* published_points,
                                     uetd_u8_t* published_payload);
uetd_status_t umid_process_image_begin_scan(uetd_u32_t expected_generation, uetd_u64_t started_at_us);
uetd_status_t umid_process_image_stage_point(const umid_process_point_t* point);
uetd_status_t umid_process_image_commit_scan(uetd_u64_t committed_at_us);
uetd_status_t umid_process_image_snapshot(umid_process_scan_t* scan);

/* stage_point copies the payload into caller-provided staging storage. A
 * snapshot's point/payload views remain valid only until the next successful
 * commit. An implementation must publish all fields and the generation with
 * one bounded atomic swap or a versioned copy protocol. Writers are serialized. */

#endif /* UMID_FUTURE_DATA_SERVICES_H */
