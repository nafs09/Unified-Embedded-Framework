/// @file include/umid/logging/log_uart.h
/// @brief Optional UART output adapter for the transport-neutral UMID logger.

#ifndef UETD_UMID_LOG_UART_H
#define UETD_UMID_LOG_UART_H

#include "umid/logging/log.h"
#include "upal/serial/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Configure UMID_LOG to drain complete records through the selected UPAL UART.
void umid_log_uart_init(upal_uart_t* uart, umid_log_level_t min_level);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UMID_LOG_UART_H */
