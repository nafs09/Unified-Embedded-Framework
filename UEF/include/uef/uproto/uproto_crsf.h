/// @file include/uef/uproto/uproto_crsf.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UPROTO_CRSF_H
#define UPROTO_CRSF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_uart.h"
#include "uef/umid/umid_ringbuf.h"

#define UPROTO_CRSF_MAX_CHANNELS  16
#define UPROTO_CRSF_BAUD          420000

typedef struct {
    uef_i32_t channels[UPROTO_CRSF_MAX_CHANNELS];  /* ±1024 range */
    uef_i16_t rssi_dbm;
    uef_u8_t  link_quality;
    uef_u8_t  snr_db;
    uef_u8_t  active_antenna;
    uef_u8_t  rf_mode;
    uef_u8_t  tx_power;
} uproto_crsf_rc_t;

typedef struct {
    upal_uart_t*   uart;
    umid_ringbuf_t rx_ring;
    uef_u8_t       rx_ring_buf[256];
    uproto_crsf_rc_t last_rc;
    uef_u64_t      last_rx_us;
    uef_u32_t      frame_count;
    uef_u32_t      error_count;
} uproto_crsf_t;

uef_status_t uproto_crsf_init(uproto_crsf_t* c, upal_uart_t* uart);

/* Call from a bounded task or deferred interrupt path to parse queued bytes. */
void uproto_crsf_process(uproto_crsf_t* c);

bool uproto_crsf_rc_available(const uproto_crsf_t* c);
void uproto_crsf_get_rc(uproto_crsf_t* c, uproto_crsf_rc_t* out);

/* Send telemetry frame */
uef_status_t uproto_crsf_send_battery(uproto_crsf_t* c,
                                        uef_f32_t voltage_v,
                                        uef_f32_t current_a,
                                        uef_u32_t capacity_mah,
                                        uef_u8_t  remaining_pct);
uef_status_t uproto_crsf_send_attitude(uproto_crsf_t* c,
                                         uef_f32_t roll_rad,
                                         uef_f32_t pitch_rad,
                                         uef_f32_t yaw_rad);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_CRSF_H */
