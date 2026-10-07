/// @file include/uef/umid/umid_current.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UMID_CURRENT_H
#define UMID_CURRENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_adc.h"

typedef struct {
    upal_adc_t*  adc;
    uef_u8_t     channel_index;  /* index into ADC DMA buffer */
    uef_f32_t    scale;          /* raw → amperes */
    uef_f32_t    offset;         /* zero-current offset in raw counts */
    uef_f32_t    shunt_ohm;
} umid_current_t;

uef_status_t umid_current_init(umid_current_t* cs);
uef_f32_t    umid_current_read_a(const umid_current_t* cs);
uef_status_t umid_current_calibrate_zero(umid_current_t* cs);

#ifdef __cplusplus
}
#endif

#endif /* UMID_CURRENT_H */
