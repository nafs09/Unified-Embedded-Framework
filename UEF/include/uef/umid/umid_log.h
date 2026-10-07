/// @file include/uef/umid/umid_log.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UMID_LOG_H
#define UMID_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/upal/upal_uart.h"

typedef enum {
    UMID_LOG_DEBUG,
    UMID_LOG_INFO,
    UMID_LOG_WARN,
    UMID_LOG_ERROR,
    UMID_LOG_FATAL,
} umid_log_level_t;

/* Configure logging backend — call before uos_scheduler_start */
void umid_log_init(upal_uart_t* uart, umid_log_level_t min_level);

/* Thread-safe: enqueues to a ring buffer; drained by a low-priority task */
void umid_log(umid_log_level_t level, const char* tag, const char* fmt, ...);
void umid_log_hex(umid_log_level_t level, const char* tag,
                   const uef_u8_t* data, uef_u32_t len);

/* Convenience macros */
#define UMID_LOGI(tag, ...)  umid_log(UMID_LOG_INFO,  tag, __VA_ARGS__)
#define UMID_LOGW(tag, ...)  umid_log(UMID_LOG_WARN,  tag, __VA_ARGS__)
#define UMID_LOGE(tag, ...)  umid_log(UMID_LOG_ERROR, tag, __VA_ARGS__)
#define UMID_LOGD(tag, ...)  umid_log(UMID_LOG_DEBUG, tag, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* UMID_LOG_H */
