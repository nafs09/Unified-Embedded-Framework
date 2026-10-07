/// @file include/uef/upal/upal_timer.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UPAL_TIMER_H
#define UPAL_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_dma.h"
#include "uef/uhal/uhal_gpio.h"
#include <uef/uhal/target.h>

typedef struct {
    void*       instance;
    IRQn_Type   up_irqn;       /* update interrupt */
    IRQn_Type   cc_irqn;       /* capture/compare interrupt */
    upal_dma_t* dma_update;    /* for DMA waveform output */
} upal_timer_hw_t;

typedef struct {
    const upal_timer_hw_t* hw;
    uef_u32_t              period;     /* ARR value */
    uef_u32_t              prescaler;  /* PSC value */
    volatile uef_u32_t     state;
    void (*on_update)(void* ctx);
    void*  update_ctx;
} upal_timer_t;

/* Timebase — period_us sets the update event interval */
uef_status_t upal_timer_init_timebase(upal_timer_t* t,
                                        const upal_timer_hw_t* hw,
                                        uef_u32_t period_us);

/* PWM output on a compare channel */
uef_status_t upal_timer_pwm_init(upal_timer_t* t, uef_u8_t channel,
                                   uhal_gpio_pin_t pin, uef_u8_t af);
void         upal_timer_pwm_set_duty(upal_timer_t* t, uef_u8_t channel,
                                      uef_u32_t duty);  /* 0 to period */

/* Input capture */
uef_status_t upal_timer_capture_init(upal_timer_t* t, uef_u8_t channel,
                                       uhal_gpio_pin_t pin, uef_u8_t af);

/* DMA waveform output (DSHOT, custom pulses)
 * Loads compare values from a DMA buffer on each update event */
uef_status_t upal_timer_dma_output_start(upal_timer_t* t, uef_u8_t channel,
                                           const uef_u16_t* values,
                                           uef_u32_t count,
                                           upal_dma_callback_t cb, void* ctx);

void upal_timer_start(upal_timer_t* t);
void upal_timer_stop(upal_timer_t* t);
void upal_timer_up_irq_handler(upal_timer_t* t);
void upal_timer_cc_irq_handler(upal_timer_t* t);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_TIMER_H */
