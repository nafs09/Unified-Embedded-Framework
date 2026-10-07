/// @file include/uef/uproto/uproto_sbus.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UPROTO_SBUS_H
#define UPROTO_SBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_uart.h"

#define UPROTO_SBUS_CHANNELS  16
#define UPROTO_SBUS_BAUD      100000  /* inverted UART, 100k 8E2 */

typedef struct {
    uef_u16_t channels[UPROTO_SBUS_CHANNELS];  /* 172–1811 range */
    bool      failsafe;
    bool      frame_lost;
    uef_u64_t timestamp_us;
} uproto_sbus_frame_t;

typedef struct {
    upal_uart_t*       uart;
    uef_u8_t           rx_buf[25];
    uef_u8_t           rx_pos;
    uproto_sbus_frame_t last_frame;
} uproto_sbus_t;

uef_status_t uproto_sbus_init(uproto_sbus_t* s, upal_uart_t* uart);
void         uproto_sbus_process(uproto_sbus_t* s); /* from IDLE ISR */
bool         uproto_sbus_available(const uproto_sbus_t* s);
void         uproto_sbus_get(uproto_sbus_t* s, uproto_sbus_frame_t* out);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_SBUS_H */
