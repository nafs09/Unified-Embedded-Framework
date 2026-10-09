/// @file examples/dshot_motor/main.c
/// @brief Hardware example scaffold: dshot_motor.
///
/// TODO: Select a DSHOT rate and timer/UART transport, then hand DMA-owned symbol buffers to the
/// board driver. Keep motor arming and failsafe policy in the application.
#include "uef/uproto/uproto_dshot.h"

int main(void) {
    /* TODO(dshot-motor main): Initialize the selected timer-DMA or UART transport and
     * verify its clock, symbol timing, pin polarity, and buffer ownership for the chosen
     * DSHOT rate. Keep the ESC disarmed during setup, send only commands authorized by the
     * application arming/failsafe policy, and stop output safely on transport failure.
     */
    return 0;
}
