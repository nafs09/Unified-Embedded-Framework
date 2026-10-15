/// @file src/umid/log.c
/// @brief Bounded multi-producer, single-consumer text logger for one MCU core.

#include "umid/log.h"
#include "ucore/time.h"
#include "uhal/irq.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if UMID_LOG_QUEUE_CAPACITY < 1 || UMID_LOG_QUEUE_CAPACITY > 255
#error "UMID_LOG_QUEUE_CAPACITY must be in [1, 255]"
#endif
#if UMID_LOG_RECORD_BYTES < 24u || UMID_LOG_RECORD_BYTES > 65535u
#error "UMID_LOG_RECORD_BYTES must be in [24, 65535]"
#endif

typedef struct {
    uetd_u16_t length;
    char text[UMID_LOG_RECORD_BYTES];
} umid_log_record_t;

static umid_log_record_t g_queue[UMID_LOG_QUEUE_CAPACITY];
static upal_uart_t* g_uart;
static umid_log_level_t g_min_level = UMID_LOG_INFO;
static uetd_u8_t g_head;
static uetd_u8_t g_tail;
static uetd_u8_t g_count;
static bool g_draining;
static uetd_u32_t g_dropped;

static const char* level_name(umid_log_level_t level) {
    switch (level) {
    case UMID_LOG_DEBUG: return "DEBUG";
    case UMID_LOG_INFO: return "INFO";
    case UMID_LOG_WARN: return "WARN";
    case UMID_LOG_ERROR: return "ERROR";
    case UMID_LOG_FATAL: return "FATAL";
    default: return "INVALID";
    }
}

static void increment_dropped_locked(void) {
    if (g_dropped < ~(uetd_u32_t)0u) { ++g_dropped; }
}

static void enqueue_record(const char* text, uetd_u32_t length) {
    const uhal_critical_t critical_state = uhal_critical_enter();
    if (g_count >= UMID_LOG_QUEUE_CAPACITY) {
        increment_dropped_locked();
        uhal_critical_exit(critical_state);
        return;
    }
    umid_log_record_t* const record = &g_queue[g_head];
    if (length >= UMID_LOG_RECORD_BYTES) { length = UMID_LOG_RECORD_BYTES - 1u; }
    memcpy(record->text, text, length);
    record->text[length] = '\0';
    record->length = (uetd_u16_t)length;
    g_head = (uetd_u8_t)((g_head + 1u) % UMID_LOG_QUEUE_CAPACITY);
    ++g_count;
    uhal_critical_exit(critical_state);
}

static bool level_enabled(umid_log_level_t level) {
    if (level < UMID_LOG_DEBUG || level > UMID_LOG_FATAL) { return false; }
    const uhal_critical_t critical_state = uhal_critical_enter();
    const bool enabled = level >= g_min_level;
    uhal_critical_exit(critical_state);
    return enabled;
}

static uetd_u32_t format_prefix(char* output, size_t capacity, umid_log_level_t level, const char* tag) {
    const int count = snprintf(output, capacity, "%llu %s %s: ", (unsigned long long)uetd_time_now_us(), level_name(level),
                               tag != NULL ? tag : "UETD");
    if (count < 0 || (size_t)count >= capacity) { return 0u; }
    return (uetd_u32_t)count;
}

void umid_log_init(upal_uart_t* uart, umid_log_level_t min_level) {
    if (min_level < UMID_LOG_DEBUG || min_level > UMID_LOG_FATAL) { min_level = UMID_LOG_INFO; }
    const uhal_critical_t critical_state = uhal_critical_enter();
    g_uart = uart;
    g_min_level = min_level;
    g_head = 0u;
    g_tail = 0u;
    g_count = 0u;
    g_draining = false;
    g_dropped = 0u;
    uhal_critical_exit(critical_state);
}

void umid_log(umid_log_level_t level, const char* tag, const char* fmt, ...) {
    if (!level_enabled(level) || fmt == NULL) { return; }
    char line[UMID_LOG_RECORD_BYTES];
    const uetd_u32_t prefix = format_prefix(line, sizeof(line), level, tag);
    if (prefix == 0u || prefix >= sizeof(line) - 2u) {
        const uhal_critical_t critical_state = uhal_critical_enter();
        increment_dropped_locked();
        uhal_critical_exit(critical_state);
        return;
    }
    const size_t available = sizeof(line) - (size_t)prefix - 2u;
    va_list args;
    va_start(args, fmt);
    const int formatted = vsnprintf(line + prefix, available + 1u, fmt, args);
    va_end(args);
    if (formatted < 0) {
        const uhal_critical_t critical_state = uhal_critical_enter();
        increment_dropped_locked();
        uhal_critical_exit(critical_state);
        return;
    }
    uetd_u32_t length;
    if ((size_t)formatted > available) {
        /* Preserve a visible truncation marker and one terminating newline. */
        const size_t end = sizeof(line) - 1u;
        line[end - 4u] = '.';
        line[end - 3u] = '.';
        line[end - 2u] = '.';
        line[end - 1u] = '\n';
        line[end] = '\0';
        length = (uetd_u32_t)end;
    } else {
        length = prefix + (uetd_u32_t)formatted;
        if (length == prefix || line[length - 1u] != '\n') { line[length++] = '\n'; }
        line[length] = '\0';
    }
    enqueue_record(line, length);
}

void umid_log_hex(umid_log_level_t level, const char* tag, const uetd_u8_t* data, uetd_u32_t len) {
    if (!level_enabled(level) || (len != 0u && data == NULL)) { return; }
    char line[UMID_LOG_RECORD_BYTES];
    const uetd_u32_t prefix = format_prefix(line, sizeof(line), level, tag);
    if (prefix == 0u || prefix >= sizeof(line) - 8u) {
        const uhal_critical_t critical_state = uhal_critical_enter();
        increment_dropped_locked();
        uhal_critical_exit(critical_state);
        return;
    }
    int used = snprintf(line + prefix, sizeof(line) - prefix, "HEX[%lu] ", (unsigned long)len);
    if (used < 0 || (size_t)used >= sizeof(line) - prefix) {
        const uhal_critical_t critical_state = uhal_critical_enter();
        increment_dropped_locked();
        uhal_critical_exit(critical_state);
        return;
    }
    uetd_u32_t length = prefix + (uetd_u32_t)used;
    const uetd_u32_t preview = len < 32u ? len : 32u;
    for (uetd_u32_t index = 0u; index < preview; ++index) {
        if (length + 4u >= sizeof(line)) { break; }
        used = snprintf(line + length, sizeof(line) - length, "%02X%s", (unsigned)data[index], index + 1u < preview ? " " : "");
        if (used < 0) { break; }
        length += (uetd_u32_t)used;
    }
    if (preview < len && length + 20u < sizeof(line)) {
        used = snprintf(line + length, sizeof(line) - length, " ...(+%lu)", (unsigned long)(len - preview));
        if (used > 0) { length += (uetd_u32_t)used; }
    }
    if (length + 2u > sizeof(line)) { length = sizeof(line) - 2u; }
    line[length++] = '\n';
    line[length] = '\0';
    enqueue_record(line, length);
}

uetd_status_t umid_log_drain(uetd_u32_t max_records, uetd_u32_t timeout_ms, uetd_u32_t* drained) {
    if (drained == NULL) { return UETD_INVALID_ARG; }
    *drained = 0u;
    const uhal_critical_t start_state = uhal_critical_enter();
    if (g_draining) {
        uhal_critical_exit(start_state);
        return UETD_BUSY;
    }
    g_draining = true;
    uhal_critical_exit(start_state);
    uetd_status_t result = UETD_OK;
    while (*drained < max_records) {
        umid_log_record_t record;
        upal_uart_t* uart;
        uetd_u8_t tail;
        const uhal_critical_t critical_state = uhal_critical_enter();
        if (g_count == 0u) {
            uhal_critical_exit(critical_state);
            break;
        }
        record = g_queue[g_tail];
        tail = g_tail;
        uart = g_uart;
        uhal_critical_exit(critical_state);
        if (uart == NULL) {
            result = UETD_NOT_READY;
            break;
        }
        result = upal_uart_tx_blocking(uart, (const uetd_u8_t*)record.text, record.length, timeout_ms);
        if (result != UETD_OK) { break; }
        const uhal_critical_t commit_state = uhal_critical_enter();
        /* A single consumer is part of this API contract. */
        if (g_count == 0u || g_tail != tail) {
            result = UETD_CONFLICT;
            uhal_critical_exit(commit_state);
            break;
        }
        g_tail = (uetd_u8_t)((g_tail + 1u) % UMID_LOG_QUEUE_CAPACITY);
        --g_count;
        uhal_critical_exit(commit_state);
        ++*drained;
    }
    const uhal_critical_t finish_state = uhal_critical_enter();
    g_draining = false;
    uhal_critical_exit(finish_state);
    return result;
}

uetd_u32_t umid_log_queued(void) {
    const uhal_critical_t critical_state = uhal_critical_enter();
    const uetd_u32_t count = g_count;
    uhal_critical_exit(critical_state);
    return count;
}

uetd_u32_t umid_log_dropped(void) {
    const uhal_critical_t critical_state = uhal_critical_enter();
    const uetd_u32_t count = g_dropped;
    uhal_critical_exit(critical_state);
    return count;
}
