/// @file include/umid/data/process_image.h
/// @brief First-pass UMID typed process-image contract.

#ifndef UMID_PROCESS_IMAGE_H
#define UMID_PROCESS_IMAGE_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct {
    umid_process_image_config_t config;
    umid_process_point_t* staging_points;
    uetd_u8_t* staging_payload;
    umid_process_point_t* published_points;
    uetd_u8_t* published_payload;
    uetd_u16_t staging_point_count;
    uetd_u16_t published_point_count;
    uetd_u32_t staging_payload_used;
    uetd_u32_t published_payload_used;
    uetd_u32_t staging_generation;
    uetd_u32_t generation;
    uetd_u64_t scan_started_at_us;
    uetd_u64_t committed_at_us;
    bool initialized;
    bool scan_active;
} umid_process_image_t;

/* init borrows the schema array and four caller-owned fixed buffers for the
 * image lifetime. Calls on one instance are externally serialized. */
/// Initialize the process image.
uetd_status_t umid_process_image_init(umid_process_image_t* image, const umid_process_image_config_t* config,
                                     umid_process_point_t* staging_points,
                                     uetd_u8_t* staging_payload, umid_process_point_t* published_points,
                                     uetd_u8_t* published_payload);
/// Begin staging a process image scan for the expected generation.
uetd_status_t umid_process_image_begin_scan(umid_process_image_t* image, uetd_u32_t expected_generation,
                                            uetd_u64_t started_at_us);
/// Validate and stage a point for the active process image scan.
uetd_status_t umid_process_image_stage_point(umid_process_image_t* image, const umid_process_point_t* point);
/// Publish the staged process image scan with its commit timestamp.
uetd_status_t umid_process_image_commit_scan(umid_process_image_t* image, uetd_u64_t committed_at_us);
/// Return a coherent snapshot of the most recently committed process scan.
uetd_status_t umid_process_image_snapshot(const umid_process_image_t* image, umid_process_scan_t* scan);

/* stage_point copies the payload into caller-provided staging storage. A
 * snapshot's point/payload views remain valid only until the next successful
 * commit. An implementation must publish all fields and the generation with
 * one bounded atomic swap or a versioned copy protocol. Calls on one image
 * instance must be externally serialized; snapshot views are borrowed until
 * the next successful commit. */

#ifdef __cplusplus
}
#endif

#endif /* UMID_PROCESS_IMAGE_H */