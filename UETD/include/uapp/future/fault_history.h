/// @file include/uapp/future/fault_history.h
/// @brief Design-only retained crash and reset history contract.
///
/// This is separate from secure update and is not included by uapp.h. Records
/// require an architecture fault-entry capture adapter and an atomic retained
/// storage backend before these declarations can be promoted.

#ifndef UAPP_FUTURE_FAULT_HISTORY_H
#define UAPP_FUTURE_FAULT_HISTORY_H

#include "ucore/status.h"
#include "ucore/types.h"

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

uetd_status_t uapp_crash_record_commit(const uapp_crash_record_t* record, const uetd_u8_t* architecture_context_bytes,
                                      uetd_u16_t architecture_context_length);
uetd_status_t uapp_crash_record_read(uapp_crash_record_t* record, uetd_u8_t* architecture_context_bytes, uetd_u16_t capacity,
                                    uetd_u16_t* architecture_context_length);
uetd_status_t uapp_crash_record_mark_exported(uetd_u32_t sequence);
uetd_status_t uapp_crash_record_acknowledge(uetd_u32_t sequence);
uetd_status_t uapp_crash_record_clear(uetd_u32_t sequence);

#endif /* UAPP_FUTURE_FAULT_HISTORY_H */
