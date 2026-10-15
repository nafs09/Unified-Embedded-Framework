/// @file include/uproto/dshot.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPROTO_DSHOT_H
#define UPROTO_DSHOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/timer.h"
#include "upal/uart.h"

typedef enum {
    UPROTO_DSHOT150 = 150,
    UPROTO_DSHOT300 = 300,
    UPROTO_DSHOT600 = 600,
    UPROTO_DSHOT1200 = 1200,
} uproto_dshot_rate_t;

typedef enum {
    UPROTO_DSHOT_TRANSPORT_TIMER, /* timer DMA waveform — preferred */
    UPROTO_DSHOT_TRANSPORT_UART,  /* UART oversampling at 8× bitrate */
} uproto_dshot_transport_t;

typedef struct {
    uproto_dshot_rate_t rate;
    uproto_dshot_transport_t transport;
    union {
        struct {
            upal_timer_t* timer;
            uetd_u8_t channel;
        } timer;
        struct {
            upal_uart_t* uart;
        } uart;
    };
    uetd_u16_t symbol_buf[18]; /* 16 data + 2 reset symbols */
} uproto_dshot_t;

uetd_status_t uproto_dshot_init(uproto_dshot_t* d);

/* Encode and send one frame. Non-blocking (DMA). */
uetd_status_t uproto_dshot_send(uproto_dshot_t* d, uetd_u16_t throttle, /* 0–2047 */
                               bool telemetry);

/* Send special command (motor stop, direction change, etc.) */
uetd_status_t uproto_dshot_command(uproto_dshot_t* d, uetd_u8_t command);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_DSHOT_H */
