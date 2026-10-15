/// @file examples/full_motor_control/main.c
/// @brief Hardware example scaffold: full_motor_control.
///
/// TODO: Compose the selected sensing, control, actuator, communication, health, and fault
/// components. Confirm timing budget and safe output state before enabling the actuator path.
#include "uapp/lifecycle.h"
#include "umid/drivers/imu.h"
#include "uproto/crsf.h"
#include "uproto/dshot.h"

int main(void) {
    /* TODO(full-motor-control main): Initialize sensing, estimator, controller,
     * allocation, communications, health, and fault components in dependency order.
     * Check timing/workspace budgets and sensor validity before enabling any actuator;
     * keep outputs in the board-safe state until explicit arming, and define shutdown and
     * degraded-mode behavior for each startup/runtime failure.
     */
    return 0;
}
