/// @file src/umid/logging/log_uart.c
/// @brief Optional complete-record sink adapter from UMID logging to UPAL UART.
#include "umid/logging/log_uart.h"

static uetd_status_t umid_log_uart_write_record(void* context, const uetd_u8_t* bytes,
                                                uetd_u16_t length, uetd_u32_t timeout_ms) {
    if (context == NULL || bytes == NULL || length == 0u) return UETD_INVALID_ARG;
    return upal_uart_tx_blocking((upal_uart_t*)context, bytes, length, timeout_ms);
}

void umid_log_uart_init(upal_uart_t* uart, umid_log_level_t min_level) {
    const umid_log_sink_t sink = {umid_log_uart_write_record, uart};
    umid_log_init(&sink, min_level);
}
