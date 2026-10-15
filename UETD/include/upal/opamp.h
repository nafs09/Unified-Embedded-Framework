/// @file include/upal/opamp.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_OPAMP_H
#define UPAL_OPAMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/gpio.h"

typedef enum {
    UPAL_OPAMP_MODE_PGA,        /* programmable-gain amplifier */
    UPAL_OPAMP_MODE_FOLLOWER,   /* voltage follower */
    UPAL_OPAMP_MODE_STANDALONE, /* external resistors */
} upal_opamp_mode_t;

typedef struct {
    void* instance;
    uhal_gpio_pin_t inp_pin, inn_pin, out_pin;
} upal_opamp_hw_t;

typedef struct {
    const upal_opamp_hw_t* hw;
    upal_opamp_mode_t mode;
    uetd_u8_t pga_gain; /* 2, 4, 8, 16, 32, 64 */
} upal_opamp_t;

uetd_status_t upal_opamp_init(upal_opamp_t* o, const upal_opamp_hw_t* hw, upal_opamp_mode_t mode, uetd_u8_t pga_gain);
void upal_opamp_enable(upal_opamp_t* o);
void upal_opamp_disable(upal_opamp_t* o);

/* Offset calibration — call once at startup in normal operating conditions */
uetd_status_t upal_opamp_calibrate(upal_opamp_t* o);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_OPAMP_H */
