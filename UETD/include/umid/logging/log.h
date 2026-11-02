/// @file include/umid/logging/log.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UMID_LOG_H
#define UMID_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

#ifndef UMID_LOG_QUEUE_CAPACITY
#define UMID_LOG_QUEUE_CAPACITY 16u
#endif
#ifndef UMID_LOG_RECORD_BYTES
#define UMID_LOG_RECORD_BYTES 192u
#endif

typedef enum {
    UMID_LOG_DEBUG,
    UMID_LOG_INFO,
    UMID_LOG_WARN,
    UMID_LOG_ERROR,
    UMID_LOG_FATAL,
} umid_log_level_t;

typedef uetd_status_t (*umid_log_write_record_fn)(void* context, const uetd_u8_t* bytes,
                                                   uetd_u16_t length, uetd_u32_t timeout_ms);

typedef struct {
    /* A successful call consumes the entire record; errors consume none. */
    umid_log_write_record_fn write_record;
    void* context;
} umid_log_sink_t;

/* Configure a transport-neutral output sink and reset the bounded queue/drop
 * counter. Call once before concurrent producers; sink/context must outlive
 * the logger. A callback may bridge to UART, BLE, WiFi/IP, USB, or storage.
 */
/// Initialize the bounded logging queue.
void umid_log_init(const umid_log_sink_t* sink, umid_log_level_t min_level);

/* Task/main context only (vsnprintf is not required to be ISR-safe). Concurrent
 * producers on one core are serialized; cross-core producers need one logger per
 * core or a future cross-core queue. Full queues drop the newest record.
 */
/// Queue a formatted log message without blocking.
void umid_log(umid_log_level_t level, const char* tag, const char* fmt, ...);
/// Queue a byte buffer as a hexadecimal log message.
void umid_log_hex(umid_log_level_t level, const char* tag, const uetd_u8_t* data, uetd_u32_t len);

/* Call from a low-priority task. The queue head is retained on sink error so a
 * transient backend failure does not silently consume an unwritten record.
 */
/// Write queued log messages to the configured output sink.
uetd_status_t umid_log_drain(uetd_u32_t max_records, uetd_u32_t timeout_ms, uetd_u32_t* drained);
/// Return the number of log messages waiting in the queue.
uetd_u32_t umid_log_queued(void);
/// Return the number of log messages dropped because the queue was full.
uetd_u32_t umid_log_dropped(void);

/* Convenience macros */
#define UMID_LOGI(tag, ...) umid_log(UMID_LOG_INFO, tag, __VA_ARGS__)
#define UMID_LOGW(tag, ...) umid_log(UMID_LOG_WARN, tag, __VA_ARGS__)
#define UMID_LOGE(tag, ...) umid_log(UMID_LOG_ERROR, tag, __VA_ARGS__)
#define UMID_LOGD(tag, ...) umid_log(UMID_LOG_DEBUG, tag, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* UMID_LOG_H */
