/// @file include/upal/hrtim.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_HRTIM_H
#define UPAL_HRTIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/gpio.h"
#include "upal/dma.h"
#include <uhal/target.h>

typedef enum {
    UPAL_HRTIM_MASTER = 0,
    UPAL_HRTIM_TIMER_A,
    UPAL_HRTIM_TIMER_B,
    UPAL_HRTIM_TIMER_C,
    UPAL_HRTIM_TIMER_D,
    UPAL_HRTIM_TIMER_E,
} upal_hrtim_timer_t;

typedef struct {
    void* instance;
    IRQn_Type master_irqn;
    IRQn_Type timer_irqns[5];
    IRQn_Type fault_irqn;
    upal_dma_t* dma[5];
} upal_hrtim_hw_t;

typedef struct {
    upal_hrtim_timer_t timer;
    uhal_gpio_pin_t out1_pin, out2_pin;
    uetd_u8_t out1_af, out2_af; /* AF13 on STM32G4 */
    uetd_u32_t period_ns;
    uetd_u32_t dead_time_rise_ns;
    uetd_u32_t dead_time_fall_ns;
    bool blanking_enable;
    uetd_u32_t blanking_source;
} upal_hrtim_pwm_cfg_t;

typedef struct {
    const upal_hrtim_hw_t* hw;
    volatile uetd_u32_t state;
} upal_hrtim_t;

/* DLL calibration — mandatory before any HRTIM operation */
uetd_status_t upal_hrtim_calibrate(upal_hrtim_t* h, const upal_hrtim_hw_t* hw);

/* Complementary PWM with dead-time */
uetd_status_t upal_hrtim_pwm_init(upal_hrtim_t* h, const upal_hrtim_pwm_cfg_t* cfg);
void upal_hrtim_set_duty(upal_hrtim_t* h, upal_hrtim_timer_t timer, uetd_u32_t pulse_ticks);
void upal_hrtim_preload_enable(upal_hrtim_t* h, upal_hrtim_timer_t timer);

/* Hardware fault input — zero software latency shutdown */
uetd_status_t upal_hrtim_fault_init(upal_hrtim_t* h, uetd_u8_t fault_input, uhal_gpio_pin_t fault_pin, uetd_u8_t af,
                                   bool active_high);

/* ISR handlers */
void upal_hrtim_master_irq_handler(upal_hrtim_t* h);
void upal_hrtim_timer_irq_handler(upal_hrtim_t* h, upal_hrtim_timer_t t);
void upal_hrtim_fault_irq_handler(upal_hrtim_t* h);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_HRTIM_H */
