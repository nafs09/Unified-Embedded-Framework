/// @file include/uapp/future/secure_update.h
/// @brief Design-only update state and platform capability contract.
///
/// This header is not included by uapp.h. Secure boot/update behavior is a
/// future bootloader/platform service; declarations here are not callable.

#ifndef UAPP_FUTURE_SECURE_UPDATE_H
#define UAPP_FUTURE_SECURE_UPDATE_H

#include "ucore/status.h"
#include "ucore/types.h"

typedef uetd_u32_t uapp_update_policy_t;
enum {
    UAPP_UPDATE_REQUIRE_AUTHENTICATION = 1u << 0,
    UAPP_UPDATE_REQUIRE_RECOVERY = 1u << 1,
    UAPP_UPDATE_REQUIRE_ANTI_ROLLBACK = 1u << 2,
    UAPP_UPDATE_REQUIRE_ENCRYPTION = 1u << 3,
    UAPP_UPDATE_ALLOW_UNSIGNED_DEVELOPMENT = 1u << 4,
};

/* Recovery and anti-rollback are independent bits and may be required together.
 * Recovery/anti-rollback imply authentication. Unsigned-development is only
 * valid when no authenticated-release requirement is selected. */

typedef enum {
    UAPP_UPDATE_IDLE = 0,
    UAPP_UPDATE_RECEIVING,
    UAPP_UPDATE_STAGED,
    UAPP_UPDATE_HASH_VERIFIED,
    UAPP_UPDATE_SIGNATURE_VERIFIED,
    UAPP_UPDATE_COMPATIBLE,
    UAPP_UPDATE_PENDING_TRIAL,
    UAPP_UPDATE_TRIAL_BOOT,
    UAPP_UPDATE_CONFIRMED,
    UAPP_UPDATE_ROLLBACK,
    UAPP_UPDATE_FAILED,
} uapp_update_state_t;

typedef struct {
    bool verifies_signatures;
    bool protected_trust_anchor;
    bool has_staging_region;
    bool has_trial_boot;
    bool has_rollback;
    bool has_monotonic_security_index;
    bool supports_artifact_decryption;
    bool supports_secure_transport;
    uetd_u32_t maximum_image_bytes;
    uetd_u32_t minimum_bootloader_version;
    uetd_u32_t maximum_transfer_chunk_bytes;
    uetd_u32_t maximum_session_time_ms;
} uapp_update_capabilities_t;

typedef enum {
    UAPP_IMAGE_SEGMENT_EXECUTABLE = 0,
    UAPP_IMAGE_SEGMENT_CONFIGURATION,
    UAPP_IMAGE_SEGMENT_RESOURCE,
} uapp_image_segment_role_t;

typedef struct {
    uapp_image_segment_role_t role;
    uetd_u32_t load_address;
    uetd_u32_t byte_length;
    uetd_u8_t sha256[32];
} uapp_update_segment_t;

typedef struct {
    const char* project_id;
    uetd_u16_t manifest_schema_version;
    const char* chip_id;
    const char* board_id;
    const char* minimum_hardware_revision;
    const char* maximum_hardware_revision;
    const char* firmware_version;
    uetd_u32_t minimum_bootloader_version;
    uetd_u32_t trial_health_window_ms;
    uetd_u64_t security_index;
    uetd_u32_t total_image_bytes;
    uetd_u16_t segment_count;
    const uapp_update_segment_t* segments;
    uetd_u8_t artifact_sha256[32];
    const char* canonicalization_id;
    /* Canonical payload bytes exclude the detached signature envelope. They
     * may be empty only for explicitly allowed unsigned-development updates. */
    const uetd_u8_t* canonical_manifest;
    uetd_u32_t manifest_bytes;
    uapp_update_policy_t declared_policy;
    /* Signature metadata is absent only for explicitly allowed unsigned-dev. */
    const uetd_u8_t* signature;
    uetd_u16_t signature_bytes;
    const char* signature_algorithm;
    const char* key_id;
    uetd_u16_t key_id_bytes;
} uapp_update_descriptor_t;

typedef struct {
    uetd_u16_t schema_version;
    uetd_u16_t record_bytes;
    uetd_u32_t sequence;
    uapp_update_state_t state;
    uapp_update_policy_t policy;
    uetd_u8_t artifact_sha256[32];
    uetd_u8_t manifest_sha256[32];
    uetd_u32_t received_bytes;
    uetd_u16_t verified_segment_count;
    uetd_u16_t boot_attempt_count;
    char staging_slot_id[24];
    char confirmed_slot_id[24];
    uetd_u32_t last_error;
    uetd_u32_t integrity_crc32;
} uapp_update_progress_t;

typedef struct {
    uetd_status_t (*verify_signature)(void* context, const uapp_update_descriptor_t* descriptor);
    uetd_status_t (*check_compatibility)(void* context, const uapp_update_descriptor_t* descriptor);
    uetd_status_t (*begin_staging)(void* context, uetd_u32_t image_bytes);
    uetd_status_t (*write_staging)(void* context, uetd_u32_t offset, const uetd_u8_t* bytes, uetd_u32_t length);
    uetd_status_t (*verify_staged_image)(void* context, const uapp_update_descriptor_t* descriptor);
    uetd_status_t (*select_trial_image)(void* context);
    uetd_status_t (*confirm_trial_image)(void* context);
    uetd_status_t (*rollback_trial_image)(void* context);
    uetd_status_t (*read_security_index)(void* context, uetd_u64_t* index);
    /* Called only after trial confirmation; must be power-loss safe. */
    uetd_status_t (*advance_security_index)(void* context, uetd_u64_t index);
    uetd_status_t (*read_progress)(void* context, uapp_update_progress_t* progress);
    uetd_status_t (*commit_progress_atomic)(void* context, const uapp_update_progress_t* progress);
    uetd_status_t (*clear_progress)(void* context, uetd_u32_t expected_sequence);
    void* context;
    uapp_update_capabilities_t capabilities;
} uapp_update_backend_t;

typedef struct {
    uapp_update_backend_t backend;
    uapp_update_policy_t policy;
    uapp_update_state_t state;
    const uapp_update_descriptor_t* descriptor;
    /* Writes are ordered and contiguous; retry semantics are defined by the
     * backend, and bytes are never reinterpreted after a partial write. */
    uetd_u32_t received_bytes;
    uetd_u32_t next_expected_offset;
    uetd_u64_t last_progress_us;
    uetd_status_t last_error;
} uapp_update_session_t;

uetd_status_t uapp_update_begin(uapp_update_session_t* session, const uapp_update_backend_t* backend,
                               const uapp_update_descriptor_t* descriptor, uapp_update_policy_t requested_policy);
uetd_status_t uapp_update_resume(uapp_update_session_t* session, const uapp_update_backend_t* backend,
                                const uapp_update_descriptor_t* descriptor, uapp_update_policy_t requested_policy);
uetd_status_t uapp_update_write(uapp_update_session_t* session, uetd_u32_t offset, const uetd_u8_t* bytes, uetd_u32_t length);
uetd_status_t uapp_update_finalize(uapp_update_session_t* session);
uetd_status_t uapp_update_confirm_trial(uapp_update_session_t* session);
uetd_status_t uapp_update_rollback(uapp_update_session_t* session);

/* The descriptor, segment array, canonical manifest and signature remain
 * caller-owned and immutable until the session reaches a terminal state. */

#endif /* UAPP_FUTURE_SECURE_UPDATE_H */
