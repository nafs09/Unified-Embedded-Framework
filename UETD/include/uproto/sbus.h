/// @file include/uproto/sbus.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPROTO_SBUS_H
#define UPROTO_SBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/uart.h"

#define UPROTO_SBUS_CHANNELS 16
#define UPROTO_SBUS_BAUD 100000 /* inverted UART, 100k 8E2 */

typedef struct {
    uetd_u16_t channels[UPROTO_SBUS_CHANNELS]; /* 172–1811 range */
    bool failsafe;
    bool frame_lost;
    uetd_u64_t timestamp_us;
} uproto_sbus_frame_t;

typedef struct {
    upal_uart_t* uart;
    uetd_u8_t rx_buf[25];
    uetd_u8_t rx_pos;
    uproto_sbus_frame_t last_frame;
} uproto_sbus_t;

uetd_status_t uproto_sbus_init(uproto_sbus_t* s, upal_uart_t* uart);
void uproto_sbus_process(uproto_sbus_t* s); /* from IDLE ISR */
bool uproto_sbus_available(const uproto_sbus_t* s);
void uproto_sbus_get(uproto_sbus_t* s, uproto_sbus_frame_t* out);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_SBUS_H */
