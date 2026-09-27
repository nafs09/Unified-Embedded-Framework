/// @file include/uef/umid/umid_gnss.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UMID_GNSS_H
#define UMID_GNSS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_uart.h"

typedef struct {
    uef_f64_t latitude_deg;
    uef_f64_t longitude_deg;
    uef_f32_t altitude_m;
    uef_f32_t vel_north_mps;
    uef_f32_t vel_east_mps;
    uef_f32_t vel_down_mps;
    uef_f32_t horiz_accuracy_m;
    uef_f32_t vert_accuracy_m;
    uef_u8_t  n_sats;
    uef_u8_t  fix_type;   /* 0=no fix, 2=2D, 3=3D, 4=3D+DGPS, 5=3D+RTK */
    uef_u32_t tow_ms;     /* GPS time of week */
    uef_u64_t timestamp_us;
} umid_gnss_data_t;

typedef struct {
    upal_uart_t*  uart;
    uef_u8_t*     parse_buf;
    uef_u32_t     parse_buf_size;
    /* Protocol: NMEA, UBX, RTCM handled by device driver */
    void (*on_fix)(umid_gnss_data_t* data, void* ctx);
    void* on_fix_ctx;
} umid_gnss_t;

uef_status_t umid_gnss_init(umid_gnss_t* g);
void         umid_gnss_process(umid_gnss_t* g);  /* call from UART IDLE ISR */
bool         umid_gnss_has_fix(const umid_gnss_t* g);

#ifdef __cplusplus
}
#endif

#endif /* UMID_GNSS_H */
