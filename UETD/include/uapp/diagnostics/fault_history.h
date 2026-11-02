/// @file include/uapp/diagnostics/fault_history.h
/// @brief Canonical crash records over an atomic retained-storage provider.
///
/// Single-owner and nonreentrant. Provider callbacks borrow buffers only for
/// their call. An architecture capture adapter supplies the logical record.

#ifndef UAPP_FAULT_HISTORY_H
#define UAPP_FAULT_HISTORY_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UAPP_CRASH_PROJECT_ID_CAPACITY 40u
#define UAPP_CRASH_FIRMWARE_VERSION_CAPACITY 24u
#define UAPP_CRASH_TARGET_ID_CAPACITY 40u
#define UAPP_CRASH_BOARD_ID_CAPACITY 40u
#define UAPP_CRASH_HARDWARE_REVISION_CAPACITY 16u
#define UAPP_CRASH_DETAIL_CAPACITY 48u

typedef enum {
    UAPP_CRASH_RECORD_UNREAD = 0,
    UAPP_CRASH_RECORD_EXPORTED,
    UAPP_CRASH_RECORD_ACKNOWLEDGED,
} uapp_crash_record_state_t;

enum {
    UAPP_CRASH_CAPTURE_PC_LR_VALID = 1ull << 0,
    UAPP_CRASH_CAPTURE_ARCH_STATUS_VALID = 1ull << 1,
    UAPP_CRASH_CAPTURE_TASK_ID_VALID = 1ull << 2,
    UAPP_CRASH_CAPTURE_MONOTONIC_TIME_VALID = 1ull << 3,
    UAPP_CRASH_CAPTURE_RTC_TIME_VALID = 1ull << 4,
    UAPP_CRASH_CAPTURE_ARCH_CONTEXT_VALID = 1ull << 5,
    UAPP_CRASH_DETAIL_TRUNCATED = 1u << 0,
};

/* This logical record is encoded by schema, never persisted by dumping the C
 * struct bytes. capture_valid_mask distinguishes unavailable context from a
 * legitimately captured zero-valued register. CRC detects damage, not forgery. */
typedef struct {
    uetd_u16_t format_version;
    uetd_u16_t header_bytes;
    uetd_u32_t magic;
    uetd_u32_t sequence;
    uetd_u32_t fault_code;
    uetd_u32_t fault_type;
    uetd_u32_t reset_cause;
    uetd_u32_t core_id;
    uetd_u32_t task_or_irq_id;
    uetd_u32_t pc;
    uetd_u32_t lr;
    uetd_u32_t cfsr;
    uetd_u32_t hfsr;
    uetd_u32_t dfsr;
    uetd_u32_t mmfar;
    uetd_u32_t bfar;
    uetd_u32_t afsr;
    uetd_u64_t capture_valid_mask;
    uetd_u32_t clock_id;
    uetd_u64_t monotonic_us;
    uetd_u64_t rtc_time;
    char project_id[UAPP_CRASH_PROJECT_ID_CAPACITY];
    char firmware_version[UAPP_CRASH_FIRMWARE_VERSION_CAPACITY];
    char target_id[UAPP_CRASH_TARGET_ID_CAPACITY];
    char board_id[UAPP_CRASH_BOARD_ID_CAPACITY];
    char hardware_revision[UAPP_CRASH_HARDWARE_REVISION_CAPACITY];
    uetd_u8_t artifact_sha256[32];
    uetd_u16_t detail_length;
    uetd_u16_t detail_flags;
    char detail[UAPP_CRASH_DETAIL_CAPACITY];
    uetd_u16_t architecture_context_length;
    uetd_u16_t architecture_context_schema;
    uetd_u32_t integrity_crc32;
    uapp_crash_record_state_t state;
} uapp_crash_record_t;

#define UAPP_CRASH_CONTEXT_CAPACITY 256u
#define UAPP_CRASH_MAGIC 0x55435231u
/* The provider stores the encoded bytes atomically, preserving the previous
 * valid value if commit fails or power is lost. expected_sequence=0 means empty.
 * A corrupt retained record is reported, never silently overwritten. The backend
 * must compare the expected sequence before commit/clear and retain a durable
 * sequence high-water mark across clear. A new record must never reuse an old
 * sequence; sequence exhaustion requires explicit storage re-provisioning.
 * No native C layout is
 * persisted. Bind once at startup. Default is NOT_READY, never pretend durability.
 * These calls are owner-context APIs. Fault/ISR use requires a separately
 * qualified nonblocking provider and platform serialization/emergency stack. */
typedef struct {
    uetd_status_t (*read)(void* context, uetd_u8_t* bytes, size_t capacity, size_t* length);
    uetd_status_t (*commit_atomic)(void* context, const uetd_u8_t* bytes, size_t length, uetd_u32_t expected_sequence);
    uetd_status_t (*clear_atomic)(void* context, uetd_u32_t expected_sequence);
    void* context;
} uapp_crash_backend_t;
uetd_status_t uapp_crash_history_bind(const uapp_crash_backend_t* backend);

/// Store a crash record and its architecture-specific context atomically.
uetd_status_t uapp_crash_record_commit(const uapp_crash_record_t* record, const uetd_u8_t* architecture_context_bytes,
                                      uetd_u16_t architecture_context_length);
/// Read a persisted crash record and its architecture-specific context.
uetd_status_t uapp_crash_record_read(uapp_crash_record_t* record, uetd_u8_t* architecture_context_bytes, uetd_u16_t capacity,
                                    uetd_u16_t* architecture_context_length);
/// Mark a crash record as exported using its sequence number.
uetd_status_t uapp_crash_record_mark_exported(uetd_u32_t sequence);
/// Acknowledge a crash record using its sequence number.
uetd_status_t uapp_crash_record_acknowledge(uetd_u32_t sequence);
/// Clear a crash record using its sequence number.
uetd_status_t uapp_crash_record_clear(uetd_u32_t sequence);

#ifdef __cplusplus
}
#endif
#endif /* UAPP_FAULT_HISTORY_H */
