/// @file include/uproto/crsf.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPROTO_CRSF_H
#define UPROTO_CRSF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "umid/ringbuf.h"
#include "upal/uart.h"

#define UPROTO_CRSF_MAX_CHANNELS 16
#define UPROTO_CRSF_BAUD 420000

typedef struct {
    uetd_i32_t channels[UPROTO_CRSF_MAX_CHANNELS]; /* ±1024 range */
    uetd_i16_t rssi_dbm;
    uetd_u8_t link_quality;
    uetd_u8_t snr_db;
    uetd_u8_t active_antenna;
    uetd_u8_t rf_mode;
    uetd_u8_t tx_power;
} uproto_crsf_rc_t;

typedef struct {
    upal_uart_t* uart;
    umid_ringbuf_t rx_ring;
    uetd_u8_t rx_ring_buf[256];
    uproto_crsf_rc_t last_rc;
    uetd_u64_t last_rx_us;
    uetd_u32_t frame_count;
    uetd_u32_t error_count;
} uproto_crsf_t;

uetd_status_t uproto_crsf_init(uproto_crsf_t* c, upal_uart_t* uart);

/* Call from a bounded task or deferred interrupt path to parse queued bytes. */
void uproto_crsf_process(uproto_crsf_t* c);

bool uproto_crsf_rc_available(const uproto_crsf_t* c);
void uproto_crsf_get_rc(uproto_crsf_t* c, uproto_crsf_rc_t* out);

/* Send telemetry frame */
uetd_status_t uproto_crsf_send_battery(uproto_crsf_t* c, uetd_f32_t voltage_v, uetd_f32_t current_a, uetd_u32_t capacity_mah,
                                      uetd_u8_t remaining_pct);
uetd_status_t uproto_crsf_send_attitude(uproto_crsf_t* c, uetd_f32_t roll_rad, uetd_f32_t pitch_rad, uetd_f32_t yaw_rad);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_CRSF_H */
