/// @file include/uef/umid/umid_range.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UMID_RANGE_H
#define UMID_RANGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

typedef enum {
    UMID_RANGE_ULTRASONIC,
    UMID_RANGE_LIDAR_I2C,
    UMID_RANGE_TOF_SPI,
} umid_range_type_t;

typedef struct {
    umid_range_type_t type;
    uef_f32_t         range_m;
    uef_f32_t         max_range_m;
    uef_f32_t         min_range_m;
    uef_u64_t         timestamp_us;
} umid_range_t;

uef_status_t umid_range_init(umid_range_t* r);
uef_status_t umid_range_read(umid_range_t* r);

#ifdef __cplusplus
}
#endif

#endif /* UMID_RANGE_H */
