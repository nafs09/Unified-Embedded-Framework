/// @file include/umid/drivers/current.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UMID_CURRENT_H
#define UMID_CURRENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/adc.h"

typedef struct {
    upal_adc_t* adc;
    uetd_u8_t channel_index; /* index into ADC DMA buffer */
    uetd_f32_t scale;        /* raw → amperes */
    uetd_f32_t offset;       /* zero-current offset in raw counts */
    uetd_f32_t shunt_ohm;
} umid_current_t;

uetd_status_t umid_current_init(umid_current_t* cs);
uetd_f32_t umid_current_read_a(const umid_current_t* cs);
uetd_status_t umid_current_calibrate_zero(umid_current_t* cs);

#ifdef __cplusplus
}
#endif

#endif /* UMID_CURRENT_H */
