/// @file include/uef/uhal/uhal_gpio.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UHAL_GPIO_H
#define UHAL_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

typedef enum {
    UHAL_GPIO_INPUT,
    UHAL_GPIO_OUTPUT_PP,    /* push-pull */
    UHAL_GPIO_OUTPUT_OD,    /* open-drain */
    UHAL_GPIO_ALTERNATE,    /* peripheral alternate function */
    UHAL_GPIO_ANALOG,       /* ADC / DAC / comparator */
} uhal_gpio_mode_t;

typedef enum {
    UHAL_GPIO_SPEED_LOW,
    UHAL_GPIO_SPEED_MEDIUM,
    UHAL_GPIO_SPEED_HIGH,
    UHAL_GPIO_SPEED_VERY_HIGH,
} uhal_gpio_speed_t;

typedef enum {
    UHAL_GPIO_PULL_NONE,
    UHAL_GPIO_PULL_UP,
    UHAL_GPIO_PULL_DOWN,
} uhal_gpio_pull_t;

typedef struct {
    void*    port;      /* GPIO port base address: GPIOA, GPIOB, etc. */
    uef_u8_t pin;       /* pin number 0-15 */
} uhal_gpio_pin_t;

typedef struct {
    uhal_gpio_mode_t  mode;
    uhal_gpio_speed_t speed;
    uhal_gpio_pull_t  pull;
    uef_u8_t          alternate;  /* AF number; ignored unless mode=ALTERNATE */
} uhal_gpio_cfg_t;

void uhal_gpio_init(uhal_gpio_pin_t pin, const uhal_gpio_cfg_t* cfg);
void uhal_gpio_write(uhal_gpio_pin_t pin, bool state);
bool uhal_gpio_read(uhal_gpio_pin_t pin);
void uhal_gpio_toggle(uhal_gpio_pin_t pin);
void uhal_gpio_set(uhal_gpio_pin_t pin);    /* fast set (BSRR) */
void uhal_gpio_clear(uhal_gpio_pin_t pin);  /* fast clear (BSRR) */

/* Timing debug markers — stripped in NDEBUG builds */
#ifdef NDEBUG
#  define UHAL_MARK_SET(pin)    ((void)0)
#  define UHAL_MARK_CLR(pin)    ((void)0)
#  define UHAL_MARK_PULSE(pin)  ((void)0)
#else
#  define UHAL_MARK_SET(pin)    uhal_gpio_set(pin)
#  define UHAL_MARK_CLR(pin)    uhal_gpio_clear(pin)
#  define UHAL_MARK_PULSE(pin)  do{uhal_gpio_set(pin);uhal_gpio_clear(pin);}while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* UHAL_GPIO_H */
