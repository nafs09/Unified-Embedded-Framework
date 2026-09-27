/// @file include/uef/umid/umid_temperature.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UMID_TEMPERATURE_H
#define UMID_TEMPERATURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

typedef enum {
    UMID_TEMP_THERMOCOUPLE_K,
    UMID_TEMP_THERMOCOUPLE_J,
    UMID_TEMP_NTC,
    UMID_TEMP_PT100,
    UMID_TEMP_INTERNAL_MCU,
} umid_temp_type_t;

typedef struct {
    umid_temp_type_t type;
    upal_spi_t*      spi;    /* for thermocouple ICs (MAX31856, etc.) */
    uhal_gpio_pin_t  cs;
    uef_f32_t        ntc_r25_ohm;
    uef_f32_t        ntc_beta;
} umid_temperature_t;

uef_status_t umid_temperature_init(umid_temperature_t* ts);
uef_f32_t    umid_temperature_read_degC(umid_temperature_t* ts);

#ifdef __cplusplus
}
#endif

#endif /* UMID_TEMPERATURE_H */
