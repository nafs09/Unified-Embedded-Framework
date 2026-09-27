/// @file src/uhal/x86/gpio.c
/// @brief In-memory GPIO state for host simulation only.

#include "uef/uhal/uhal_gpio.h"

/* Keep a fixed-capacity registry so the simulator never needs heap allocation. */
/* TODO(host-gpio-registry): define a host assertion/diagnostic for table exhaustion and add
 * synchronization before tests drive simulated pins from more than one thread. */
typedef struct {
    void* port;
    uef_u8_t pin;
    bool state;
    bool used;
} simulated_pin_t;

#define UEF_SIMULATED_PIN_COUNT 256u
static simulated_pin_t g_pins[UEF_SIMULATED_PIN_COUNT];

/* Resolve a simulated pin by its opaque port token and numeric pin; optionally claim a free slot. */
static simulated_pin_t* find_pin(uhal_gpio_pin_t pin, bool create) {
    simulated_pin_t* free_slot = NULL;
    for (uef_u32_t index = 0u; index < UEF_SIMULATED_PIN_COUNT; ++index) {
        simulated_pin_t* candidate = &g_pins[index];
        if (candidate->used && candidate->port == pin.port && candidate->pin == pin.pin) {
            return candidate;
        }
        if (!candidate->used && free_slot == NULL) {
            free_slot = candidate;
        }
    }
    if (create && free_slot != NULL) {
        free_slot->port = pin.port;
        free_slot->pin = pin.pin;
        free_slot->state = false;
        free_slot->used = true;
        return free_slot;
    }
    return NULL;
}

void uhal_gpio_init(uhal_gpio_pin_t pin, const uhal_gpio_cfg_t* config) {
    if (config != NULL) {
        /* Direction, pull, and electrical mode are intentionally not modeled on the host. */
        (void)find_pin(pin, true);
    }
}

void uhal_gpio_write(uhal_gpio_pin_t pin, bool state) {
    simulated_pin_t* simulated = find_pin(pin, true);
    if (simulated != NULL) {
        simulated->state = state;
    }
}

bool uhal_gpio_read(uhal_gpio_pin_t pin) {
    simulated_pin_t* simulated = find_pin(pin, false);
    /* An uninitialized simulated input defaults low rather than consuming a registry slot. */
    return simulated != NULL && simulated->state;
}

void uhal_gpio_toggle(uhal_gpio_pin_t pin) {
    simulated_pin_t* simulated = find_pin(pin, true);
    if (simulated != NULL) {
        simulated->state = !simulated->state;
    }
}

void uhal_gpio_set(uhal_gpio_pin_t pin) { uhal_gpio_write(pin, true); }
void uhal_gpio_clear(uhal_gpio_pin_t pin) { uhal_gpio_write(pin, false); }
