/// @file src/uapp/diagnostics/fault_history.c
/// @brief Little-endian version 1 record codec and exact-sequence lifecycle.
#include "uapp/diagnostics/fault_history.h"
#include "ucore/checksum.h"
#include <string.h>
#define HEADER_BYTES 344u
#define RECORD_BYTES (HEADER_BYTES + UAPP_CRASH_CONTEXT_CAPACITY + 4u)
static uapp_crash_backend_t backend;
static bool bound, busy;
static void put(uetd_u8_t** p, uetd_u64_t value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) { *(*p)++ = (uetd_u8_t)value; value >>= 8; }
}
static uetd_u64_t get(const uetd_u8_t** p, unsigned bytes) {
    uetd_u64_t value = 0;
    for (unsigned i = 0; i < bytes; ++i) { value |= (uetd_u64_t)*(*p)++ << (8u * i); }
    return value;
}
static bool valid(const uapp_crash_record_t* r) {
    if (r->sequence == 0u || r->detail_length > UAPP_CRASH_DETAIL_CAPACITY ||
        (r->detail_flags & ~UAPP_CRASH_DETAIL_TRUNCATED) != 0u ||
        r->architecture_context_length > UAPP_CRASH_CONTEXT_CAPACITY ||
        r->state < UAPP_CRASH_RECORD_UNREAD || r->state > UAPP_CRASH_RECORD_ACKNOWLEDGED ||
        (r->capture_valid_mask & ~UINT64_C(63)) != 0u ||
        ((r->capture_valid_mask & UAPP_CRASH_CAPTURE_ARCH_CONTEXT_VALID) == 0u &&
          r->architecture_context_length != 0u) ||
        ((r->capture_valid_mask & UAPP_CRASH_CAPTURE_ARCH_CONTEXT_VALID) != 0u &&
         (r->architecture_context_length == 0u || r->architecture_context_schema == 0u))) { return false; }
    if (memchr(r->project_id, 0, sizeof(r->project_id)) == NULL) { return false; }
    if (memchr(r->firmware_version, 0, sizeof(r->firmware_version)) == NULL) { return false; }
    if (memchr(r->target_id, 0, sizeof(r->target_id)) == NULL) { return false; }
    if (memchr(r->board_id, 0, sizeof(r->board_id)) == NULL) { return false; }
    if (memchr(r->hardware_revision, 0, sizeof(r->hardware_revision)) == NULL) { return false; }
    return true;
}
static size_t encode(const uapp_crash_record_t* r, const uetd_u8_t* context, uetd_u8_t* bytes) {
    uetd_u8_t* p = bytes;
    memset(bytes, 0, RECORD_BYTES);
    put(&p, r->format_version, 2);
    put(&p, r->header_bytes, 2);
    put(&p, r->magic, 4);
    put(&p, r->sequence, 4);
    put(&p, r->fault_code, 4);
    put(&p, r->fault_type, 4);
    put(&p, r->reset_cause, 4);
    put(&p, r->core_id, 4);
    put(&p, r->task_or_irq_id, 4);
    put(&p, r->pc, 4);
    put(&p, r->lr, 4);
    put(&p, r->cfsr, 4);
    put(&p, r->hfsr, 4);
    put(&p, r->dfsr, 4);
    put(&p, r->mmfar, 4);
    put(&p, r->bfar, 4);
    put(&p, r->afsr, 4);
    put(&p, r->capture_valid_mask, 8);
    put(&p, r->clock_id, 4);
    put(&p, r->monotonic_us, 8);
    put(&p, r->rtc_time, 8);
    memcpy(p, r->project_id, strlen(r->project_id)); p += sizeof(r->project_id);
    memcpy(p, r->firmware_version, strlen(r->firmware_version)); p += sizeof(r->firmware_version);
    memcpy(p, r->target_id, strlen(r->target_id)); p += sizeof(r->target_id);
    memcpy(p, r->board_id, strlen(r->board_id)); p += sizeof(r->board_id);
    memcpy(p, r->hardware_revision, strlen(r->hardware_revision)); p += sizeof(r->hardware_revision);
    memcpy(p, r->artifact_sha256, 32); p += 32;
    put(&p, r->detail_length, 2);
    put(&p, r->detail_flags, 2);
    memcpy(p, r->detail, r->detail_length); p += UAPP_CRASH_DETAIL_CAPACITY;
    put(&p, r->architecture_context_length, 2);
    put(&p, r->architecture_context_schema, 2);
    put(&p, r->state, 4);
    if (r->architecture_context_length != 0u) { memcpy(p, context, r->architecture_context_length); }
    p += r->architecture_context_length;
    const uetd_u32_t crc = uetd_crc32(bytes, (uetd_u32_t)(p - bytes), UINT32_MAX) ^ UINT32_MAX;
    put(&p, crc, 4);
    return (size_t)(p - bytes);
}
static uetd_status_t decode(const uetd_u8_t* bytes, size_t length, uapp_crash_record_t* r, uetd_u8_t* context) {
    if (length < HEADER_BYTES + 4u || length > RECORD_BYTES) { return UETD_CRC_ERROR; }
    const uetd_u8_t* p = bytes + length - 4u;
    const uetd_u32_t crc = (uetd_u32_t)get(&p, 4);
    if ((uetd_crc32(bytes, (uetd_u32_t)(length - 4u), UINT32_MAX) ^ UINT32_MAX) != crc) { return UETD_CRC_ERROR; }
    p = bytes; memset(r, 0, sizeof(*r));
    r->format_version = (uetd_u16_t)get(&p, 2);
    r->header_bytes = (uetd_u16_t)get(&p, 2);
    r->magic = (uetd_u32_t)get(&p, 4);
    r->sequence = (uetd_u32_t)get(&p, 4);
    r->fault_code = (uetd_u32_t)get(&p, 4);
    r->fault_type = (uetd_u32_t)get(&p, 4);
    r->reset_cause = (uetd_u32_t)get(&p, 4);
    r->core_id = (uetd_u32_t)get(&p, 4);
    r->task_or_irq_id = (uetd_u32_t)get(&p, 4);
    r->pc = (uetd_u32_t)get(&p, 4);
    r->lr = (uetd_u32_t)get(&p, 4);
    r->cfsr = (uetd_u32_t)get(&p, 4);
    r->hfsr = (uetd_u32_t)get(&p, 4);
    r->dfsr = (uetd_u32_t)get(&p, 4);
    r->mmfar = (uetd_u32_t)get(&p, 4);
    r->bfar = (uetd_u32_t)get(&p, 4);
    r->afsr = (uetd_u32_t)get(&p, 4);
    r->capture_valid_mask = (uetd_u64_t)get(&p, 8);
    r->clock_id = (uetd_u32_t)get(&p, 4);
    r->monotonic_us = (uetd_u64_t)get(&p, 8);
    r->rtc_time = (uetd_u64_t)get(&p, 8);
    memcpy(r->project_id, p, sizeof(r->project_id)); p += sizeof(r->project_id);
    memcpy(r->firmware_version, p, sizeof(r->firmware_version)); p += sizeof(r->firmware_version);
    memcpy(r->target_id, p, sizeof(r->target_id)); p += sizeof(r->target_id);
    memcpy(r->board_id, p, sizeof(r->board_id)); p += sizeof(r->board_id);
    memcpy(r->hardware_revision, p, sizeof(r->hardware_revision)); p += sizeof(r->hardware_revision);
    memcpy(r->artifact_sha256, p, 32); p += 32;
    r->detail_length = (uetd_u16_t)get(&p, 2);
    r->detail_flags = (uetd_u16_t)get(&p, 2);
    memcpy(r->detail, p, UAPP_CRASH_DETAIL_CAPACITY); p += UAPP_CRASH_DETAIL_CAPACITY;
    r->architecture_context_length = (uetd_u16_t)get(&p, 2);
    r->architecture_context_schema = (uetd_u16_t)get(&p, 2);
    r->state = (uapp_crash_record_state_t)get(&p, 4);
    r->integrity_crc32 = crc;
    if (r->format_version != 1u || r->magic != UAPP_CRASH_MAGIC || r->header_bytes != HEADER_BYTES ||
        !valid(r) || length != HEADER_BYTES + r->architecture_context_length + 4u) { return UETD_CRC_ERROR; }
    if (r->architecture_context_length != 0u) { memcpy(context, p, r->architecture_context_length); }
    return UETD_OK;
}
static uetd_status_t read_record(uapp_crash_record_t* record, uetd_u8_t* context) {
    uetd_u8_t bytes[RECORD_BYTES]; size_t length = 0u;
    uetd_status_t status = backend.read(backend.context, bytes, sizeof(bytes), &length);
    return status == UETD_OK ? decode(bytes, length, record, context) : status;
}
static uetd_status_t enter(void) {
    if (!bound) { return UETD_NOT_READY; }
    if (busy) { return UETD_BUSY; }
    busy = true; return UETD_OK;
}
uetd_status_t uapp_crash_history_bind(const uapp_crash_backend_t* b) {
    if (b == NULL || b->read == NULL || b->commit_atomic == NULL || b->clear_atomic == NULL) { return UETD_INVALID_ARG; }
    if (bound || busy) { return UETD_BUSY; }
    backend = *b; bound = true; return UETD_OK;
}
uetd_status_t uapp_crash_record_commit(const uapp_crash_record_t* r, const uetd_u8_t* context, uetd_u16_t length) {
    if (r == NULL || !valid(r) || r->state != UAPP_CRASH_RECORD_UNREAD ||
        length != r->architecture_context_length || (length != 0u && context == NULL)) { return UETD_INVALID_ARG; }
    uetd_status_t status = enter();
    if (status != UETD_OK) { return status; }
    uapp_crash_record_t old;
    uetd_u8_t old_context[UAPP_CRASH_CONTEXT_CAPACITY];
    status = read_record(&old, old_context);
    uetd_u32_t expected = 0u;
    if (status == UETD_OK) {
        expected = old.sequence;
        if (r->sequence <= expected) { busy = false; return UETD_CONFLICT; }
    } else if (status != UETD_NOT_READY) { busy = false; return status; }
    uapp_crash_record_t normalized = *r;
    normalized.format_version = 1u; normalized.header_bytes = HEADER_BYTES; normalized.magic = UAPP_CRASH_MAGIC;
    uetd_u8_t bytes[RECORD_BYTES];
    size_t count = encode(&normalized, context, bytes);
    status = backend.commit_atomic(backend.context, bytes, count, expected);
    busy = false; return status;
}
uetd_status_t uapp_crash_record_read(uapp_crash_record_t* output, uetd_u8_t* context, uetd_u16_t capacity, uetd_u16_t* length) {
    if (output == NULL || length == NULL || (capacity != 0u && context == NULL)) { return UETD_INVALID_ARG; }
    uetd_status_t status = enter();
    if (status != UETD_OK) { return status; }
    uapp_crash_record_t record; uetd_u8_t data[UAPP_CRASH_CONTEXT_CAPACITY];
    status = read_record(&record, data);
    if (status == UETD_OK && record.architecture_context_length > capacity) { status = UETD_OVERFLOW; }
    if (status == UETD_OK) {
        if (record.architecture_context_length != 0u) { memcpy(context, data, record.architecture_context_length); }
        *output = record; *length = record.architecture_context_length;
    }
    busy = false; return status;
}
static uetd_status_t change_state(uetd_u32_t sequence, uapp_crash_record_state_t state, bool clear) {
    if (sequence == 0u) { return UETD_INVALID_ARG; }
    uetd_status_t status = enter();
    if (status != UETD_OK) { return status; }
    uapp_crash_record_t record; uetd_u8_t context[UAPP_CRASH_CONTEXT_CAPACITY];
    status = read_record(&record, context);
    if (status == UETD_OK && sequence != record.sequence) { status = UETD_CONFLICT; }
    if (status == UETD_OK) {
        if (clear) {
            status = record.state == UAPP_CRASH_RECORD_ACKNOWLEDGED ?
                backend.clear_atomic(backend.context, sequence) : UETD_NOT_READY;
        } else if (record.state < state) {
            if ((unsigned)state != (unsigned)record.state + 1u) { status = UETD_NOT_READY; }
            else {
                record.state = state; uetd_u8_t bytes[RECORD_BYTES];
                size_t count = encode(&record, context, bytes);
                status = backend.commit_atomic(backend.context, bytes, count, sequence);
            }
        } /* duplicate/export-after-ack is an idempotent no-op */
    }
    busy = false; return status;
}
uetd_status_t uapp_crash_record_mark_exported(uetd_u32_t sequence) { return change_state(sequence, UAPP_CRASH_RECORD_EXPORTED, false); }
uetd_status_t uapp_crash_record_acknowledge(uetd_u32_t sequence) { return change_state(sequence, UAPP_CRASH_RECORD_ACKNOWLEDGED, false); }
uetd_status_t uapp_crash_record_clear(uetd_u32_t sequence) { return change_state(sequence, UAPP_CRASH_RECORD_ACKNOWLEDGED, true); }
