/// @file src/uapp/fault.c
/// @brief Persist minimal fault metadata and enter the platform reset path.

#include "uapp/fault.h"
#include "ucore/status.h"
#include "ucore/time.h"
#include "uhal/core.h"
#include "uhal/irq.h"
#include "uhal/memory.h"
#include <stdlib.h>

#if UAPP_FAULT_QUEUE_CAPACITY < 1 || UAPP_FAULT_QUEUE_CAPACITY > 255
#error "UAPP_FAULT_QUEUE_CAPACITY must be in [1, 255]"
#endif
#if UAPP_FAULT_DETAIL_CAPACITY < 1
#error "UAPP_FAULT_DETAIL_CAPACITY must be at least 1"
#endif

static uapp_fault_record_t g_fault_queue[UAPP_FAULT_QUEUE_CAPACITY];
static uetd_u8_t g_fault_head;
static uetd_u8_t g_fault_tail;
static uetd_u8_t g_fault_count;
static uetd_u32_t g_fault_sequence;
static uetd_u32_t g_fault_dropped;
static uapp_fault_recovery_fn g_recovery_handler;
static void* g_recovery_context;

static size_t bounded_length(const char* text, size_t maximum) {
    size_t length = 0u;
    if (text == NULL) { return 0u; }
    while (length < maximum && text[length] != '\0') { ++length; }
    return length;
}

void uapp_fault_report(uetd_u32_t fault_code, const char* detail) {
    uapp_fault_record_t record = {0};
    record.fault_code = fault_code;
    record.occurred_at_us = uetd_time_now_us();
    const size_t copied = bounded_length(detail, UAPP_FAULT_DETAIL_CAPACITY - 1u);
    if (copied > 0u) {
        for (size_t index = 0u; index < copied; ++index) { record.detail[index] = detail[index]; }
    }
    record.detail[copied] = '\0';
    if (detail != NULL && copied == UAPP_FAULT_DETAIL_CAPACITY - 1u && detail[copied] != '\0') {
        record.flags |= UAPP_FAULT_RECORD_DETAIL_TRUNCATED;
    }
    const uhal_critical_t critical_state = uhal_critical_enter();
    record.sequence = ++g_fault_sequence;
    if (g_fault_count >= UAPP_FAULT_QUEUE_CAPACITY) {
        if (g_fault_dropped < ~(uetd_u32_t)0u) { ++g_fault_dropped; }
        uhal_critical_exit(critical_state);
        return;
    }
    g_fault_queue[g_fault_head] = record;
    g_fault_head = (uetd_u8_t)((g_fault_head + 1u) % UAPP_FAULT_QUEUE_CAPACITY);
    ++g_fault_count;
    uhal_critical_exit(critical_state);
}

bool uapp_fault_try_pop(uapp_fault_record_t* out) {
    if (out == NULL) { return false; }
    const uhal_critical_t critical_state = uhal_critical_enter();
    if (g_fault_count == 0u) {
        uhal_critical_exit(critical_state);
        return false;
    }
    *out = g_fault_queue[g_fault_tail];
    g_fault_tail = (uetd_u8_t)((g_fault_tail + 1u) % UAPP_FAULT_QUEUE_CAPACITY);
    --g_fault_count;
    uhal_critical_exit(critical_state);
    return true;
}

uetd_u32_t uapp_fault_dropped_count(void) {
    const uhal_critical_t critical_state = uhal_critical_enter();
    const uetd_u32_t dropped = g_fault_dropped;
    uhal_critical_exit(critical_state);
    return dropped;
}

void uapp_fault_clear_recoverable(void) {
    const uhal_critical_t critical_state = uhal_critical_enter();
    g_fault_head = 0u;
    g_fault_tail = 0u;
    g_fault_count = 0u;
    uhal_critical_exit(critical_state);
}

void uapp_fault_set_recovery_handler(uapp_fault_recovery_fn handler, void* context) {
    const uhal_critical_t critical_state = uhal_critical_enter();
    g_recovery_handler = handler;
    g_recovery_context = context;
    uhal_critical_exit(critical_state);
}

uetd_u32_t uapp_fault_dispatch(uetd_u32_t max_records) {
    uetd_u32_t dispatched = 0u;
    while (dispatched < max_records) {
        uapp_fault_record_t record;
        if (!uapp_fault_try_pop(&record)) { break; }
        const uhal_critical_t critical_state = uhal_critical_enter();
        const uapp_fault_recovery_fn handler = g_recovery_handler;
        void* const context = g_recovery_context;
        uhal_critical_exit(critical_state);
        if (handler != NULL) { handler(&record, context); }
        ++dispatched;
    }
    return dispatched;
}

UETD_NORETURN void uapp_fault_fatal(uetd_u32_t fault_code, const char* detail) {
    /* Detail text is deliberately not persisted in the small backup register set. */
    (void)detail;
    uhal_backup_write(UHAL_BACKUP_FAULT_CODE, fault_code);
    uhal_backup_write(UHAL_BACKUP_FAULT_PC, 0u);
    uhal_backup_write(UHAL_BACKUP_FAULT_LR, 0u);
    uhal_backup_write(UHAL_BACKUP_RESET_CAUSE, UHAL_RESET_FAULT);
    uhal_system_reset();
    /* A real target reset does not return. Keep host/debug fallback fail-fast. */
    abort();
}

bool uapp_fault_log_available(void) { return uhal_backup_read(UHAL_BACKUP_FAULT_CODE) != 0u; }

uetd_u32_t uapp_fault_log_code(void) { return uhal_backup_read(UHAL_BACKUP_FAULT_CODE); }

uetd_u32_t uapp_fault_log_pc(void) { return uhal_backup_read(UHAL_BACKUP_FAULT_PC); }

uetd_u32_t uapp_fault_log_lr(void) { return uhal_backup_read(UHAL_BACKUP_FAULT_LR); }
