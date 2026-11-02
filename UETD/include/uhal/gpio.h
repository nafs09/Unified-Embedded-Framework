/// @file include/uhal/gpio.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UHAL_GPIO_H
#define UHAL_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

typedef enum {
    UHAL_GPIO_INPUT,
    UHAL_GPIO_OUTPUT_PP, /* push-pull */
    UHAL_GPIO_OUTPUT_OD, /* open-drain */
    UHAL_GPIO_ALTERNATE, /* peripheral alternate function */
    UHAL_GPIO_ANALOG,    /* ADC / DAC / comparator */
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
    void* port;   /* Platform token; applications use generated pin bindings. */
    uetd_u8_t pin; /* Platform pin/line index; not restricted to STM32's 0..15. */
} uhal_gpio_pin_t;

typedef struct {
    uhal_gpio_mode_t mode;
    uhal_gpio_speed_t speed;
    uhal_gpio_pull_t pull;
    uetd_u8_t alternate; /* AF number; ignored unless mode=ALTERNATE */
} uhal_gpio_cfg_t;

/// Configure a GPIO pin with the requested mode and electrical settings.
void uhal_gpio_init(uhal_gpio_pin_t pin, const uhal_gpio_cfg_t* cfg);
/// Set or clear a GPIO output pin.
void uhal_gpio_write(uhal_gpio_pin_t pin, bool state);
/// Read the current GPIO pin level.
bool uhal_gpio_read(uhal_gpio_pin_t pin);
/// Invert the current GPIO output level.
void uhal_gpio_toggle(uhal_gpio_pin_t pin);
/// Set a GPIO output pin using the hardware set register.
void uhal_gpio_set(uhal_gpio_pin_t pin);   /* fast set (BSRR) */
/// Clear a GPIO output pin using the hardware reset register.
void uhal_gpio_clear(uhal_gpio_pin_t pin); /* fast clear (BSRR) */

/* Timing debug markers — stripped in NDEBUG builds */
#ifdef NDEBUG
#define UHAL_MARK_SET(pin) ((void)0)
#define UHAL_MARK_CLR(pin) ((void)0)
#define UHAL_MARK_PULSE(pin) ((void)0)
#else
#define UHAL_MARK_SET(pin) uhal_gpio_set(pin)
#define UHAL_MARK_CLR(pin) uhal_gpio_clear(pin)
#define UHAL_MARK_PULSE(pin)                                                                                                     \
    do {                                                                                                                         \
        uhal_gpio_set(pin);                                                                                                      \
        uhal_gpio_clear(pin);                                                                                                    \
    } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* UHAL_GPIO_H */
