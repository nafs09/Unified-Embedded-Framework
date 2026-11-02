/// @file include/uapp/secure_update.h
/// @brief Portable, fail-closed update orchestration over a trusted boot provider.

#ifndef UAPP_SECURE_UPDATE_H
#define UAPP_SECURE_UPDATE_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    /* Nonempty canonical payload bytes exclude the detached signature envelope,
     * including in unsigned-development mode (identity still binds recovery). */
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

typedef enum { UAPP_BOOT_STAGED = 0, UAPP_BOOT_PENDING, UAPP_BOOT_TRIAL,
               UAPP_BOOT_CONFIRMED, UAPP_BOOT_ROLLED_BACK } uapp_update_boot_state_t;

typedef struct {
    uetd_status_t (*verify_signature)(void* context, const uapp_update_descriptor_t* descriptor);
    uetd_status_t (*check_compatibility)(void* context, const uapp_update_descriptor_t* descriptor);
    uetd_status_t (*begin_staging)(void* context, const uapp_update_descriptor_t* descriptor);
    uetd_status_t (*write_staging)(void* context, uetd_u32_t offset, const uetd_u8_t* bytes, uetd_u32_t length);
    uetd_status_t (*verify_staged_image)(void* context, const uapp_update_descriptor_t* descriptor);
    uetd_status_t (*select_trial_image)(void* context);
    uetd_status_t (*confirm_trial_image)(void* context);
    uetd_status_t (*rollback_trial_image)(void* context);
    uetd_status_t (*read_security_index)(void* context, uetd_u64_t* index);
    /* Called only after trial confirmation; must be power-loss safe. */
    uetd_status_t (*advance_security_index)(void* context, uetd_u64_t index);
    uetd_status_t (*read_progress)(void* context, uapp_update_progress_t* progress);
    uetd_status_t (*commit_progress_atomic)(void* context, const uapp_update_progress_t* progress, uetd_u32_t expected_sequence);
    uetd_status_t (*clear_progress)(void* context, uetd_u32_t expected_sequence);
    void* context;
    uapp_update_capabilities_t capabilities;
    uetd_status_t (*now_us)(void* context, uetd_u64_t* now);
    uetd_status_t (*hash_manifest)(void* context, const uapp_update_descriptor_t* descriptor, uetd_u8_t sha256[32]);
    uetd_status_t (*resume_staging)(void* context, const uapp_update_descriptor_t* descriptor, uetd_u32_t verified_prefix_bytes);
    uetd_status_t (*boot_status)(void* context, const uetd_u8_t artifact_sha256[32],
                                uapp_update_boot_state_t* state, bool* health_window_passed);
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
    uetd_u64_t started_us;
    uapp_update_progress_t progress;
    bool initialized, busy;
} uapp_update_session_t;

#define UAPP_UPDATE_SESSION_INITIALIZER {0}
#define UAPP_UPDATE_PROGRESS_BYTES 144u
#define UAPP_UPDATE_MAX_SEGMENTS 32u
#define UAPP_UPDATE_MAX_MANIFEST_BYTES 65536u
/* Owner-only, nonreentrant, zero-initialize. One active owner per backend;
 * retire the old session before resuming into a fresh object. Checkpoints reject
 * stale sessions before flash/boot mutation, but do not replace synchronization.
 * No hidden heap. Providers must be
 * synchronous and bounded, consume borrowed arguments during the call, and
 * implement durable/idempotent staging and boot selection. Errors latch FAILED;
 * recovery uses a fresh session and authenticated resume, never a blind retry.
 * check_compatibility MUST bind every descriptor field to canonical manifest
 * bytes and validate exact target/load ranges. verify_signature authenticates
 * those bytes using protected trust anchors. verify_staged_image checks the full
 * artifact AND every segment. hash_manifest computes SHA-256, never a caller ID.
 * resume_staging verifies durable prefix identity before accepting more bytes.
 * boot_status identifies this exact artifact and reports actual boot/health
 * evidence. A progress CRC does not authorize activation. Provider progress
 * serialization is canonical, never a native struct dump. Atomic commits use
 * compare-and-swap sequence; preserve the previous record on failure/power loss.
 * Encrypted staging providers decrypt/authenticate under the declared policy;
 * transfer offsets/lengths refer to the descriptor's declared staging stream.
 * Flash, crypto, keys, boot health and power-loss recovery are provider duties. */
uetd_u32_t uapp_update_progress_crc(const uapp_update_progress_t* progress);

/// Begin a secure update session after validating the image descriptor.
uetd_status_t uapp_update_begin(uapp_update_session_t* session, const uapp_update_backend_t* backend,
                               const uapp_update_descriptor_t* descriptor, uapp_update_policy_t requested_policy);
/// Resume a secure update session from validated persisted progress.
uetd_status_t uapp_update_resume(uapp_update_session_t* session, const uapp_update_backend_t* backend,
                                const uapp_update_descriptor_t* descriptor, uapp_update_policy_t requested_policy);
/// Write an image chunk to the secure update staging area.
uetd_status_t uapp_update_write(uapp_update_session_t* session, uetd_u32_t offset, const uetd_u8_t* bytes, uetd_u32_t length);
/// Verify the staged image and select it for trial boot.
uetd_status_t uapp_update_finalize(uapp_update_session_t* session);
/// Confirm the running trial image as the accepted update.
uetd_status_t uapp_update_confirm_trial(uapp_update_session_t* session);
/// Roll back the trial image and restore the previous boot selection.
uetd_status_t uapp_update_rollback(uapp_update_session_t* session);

/* The descriptor, segment array, canonical manifest and signature remain
 * caller-owned and immutable for every API call on this session, including
 * rollback/recovery after a failed operation. */

#ifdef __cplusplus
}
#endif
#endif /* UAPP_SECURE_UPDATE_H */
