/// @file examples/baremetal_minimal/main.c
/// @brief Hardware example scaffold: baremetal_minimal.
///
/// TODO: Set up the board clock and initialize one verified GPIO, then blink it from the
/// application main loop. Use the selected board's pin mapping, not a family-wide assumption.
#include "uef/uhal/uhal_gpio.h"

int main(void) {
    /* TODO(baremetal-minimal main): Initialize the selected board clock and verify the
     * resulting frequency, configure one board-mapped GPIO as output, then set/clear it
     * using the HAL without changing adjacent pins. Use a bounded board delay between
     * transitions and route startup/configuration failure to the board's safe error path.
     */
    return 0;
}
