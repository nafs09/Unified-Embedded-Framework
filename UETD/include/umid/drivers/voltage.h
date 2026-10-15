/// @file include/umid/drivers/voltage.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UMID_VOLTAGE_H
#define UMID_VOLTAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/adc.h"

typedef struct {
    upal_adc_t* adc;
    uetd_u8_t channel_index;
    uetd_f32_t divider_ratio; /* R_lower / (R_upper + R_lower) */
    uetd_f32_t vref_v;        /* ADC reference voltage */
} umid_voltage_t;

uetd_status_t umid_voltage_init(umid_voltage_t* vs);
uetd_f32_t umid_voltage_read_v(const umid_voltage_t* vs);

#ifdef __cplusplus
}
#endif

#endif /* UMID_VOLTAGE_H */
