/// @file include/uef/umid/umid_voltage.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UMID_VOLTAGE_H
#define UMID_VOLTAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_adc.h"

typedef struct {
    upal_adc_t* adc;
    uef_u8_t    channel_index;
    uef_f32_t   divider_ratio;   /* R_lower / (R_upper + R_lower) */
    uef_f32_t   vref_v;          /* ADC reference voltage */
} umid_voltage_t;

uef_status_t umid_voltage_init(umid_voltage_t* vs);
uef_f32_t    umid_voltage_read_v(const umid_voltage_t* vs);

#ifdef __cplusplus
}
#endif

#endif /* UMID_VOLTAGE_H */
