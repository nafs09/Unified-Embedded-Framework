/// @file include/upal/comp.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_COMP_H
#define UPAL_COMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/gpio.h"
#include <uhal/target.h>

typedef enum {
    UPAL_COMP_INM_VREF_1_4,
    UPAL_COMP_INM_VREF_1_2,
    UPAL_COMP_INM_VREF_3_4,
    UPAL_COMP_INM_VREF,
    UPAL_COMP_INM_DAC1_CH1,
    UPAL_COMP_INM_DAC1_CH2,
    UPAL_COMP_INM_PIN,
} upal_comp_inm_t;

typedef struct {
    void* instance;
    IRQn_Type irqn;
    uhal_gpio_pin_t inp_pin; /* non-inverting */
    uhal_gpio_pin_t inn_pin; /* inverting (if PIN mode) */
    uhal_gpio_pin_t out_pin;
    uetd_u8_t out_af;
} upal_comp_hw_t;

typedef void (*upal_comp_cb_t)(bool output_state, void* ctx);

typedef struct {
    const upal_comp_hw_t* hw;
    upal_comp_inm_t inm;
    uetd_u32_t hysteresis;
    bool inverted_out;
    upal_comp_cb_t callback;
    void* ctx;
} upal_comp_t;

uetd_status_t upal_comp_init(upal_comp_t* c, const upal_comp_hw_t* hw, upal_comp_inm_t inm, upal_comp_cb_t cb, void* ctx);
void upal_comp_enable(upal_comp_t* c);
void upal_comp_disable(upal_comp_t* c);
bool upal_comp_read(const upal_comp_t* c);
void upal_comp_irq_handler(upal_comp_t* c);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_COMP_H */
