/// @file examples/full_motor_control/main.c
/// @brief Hardware example scaffold: full_motor_control.
///
/// TODO: Compose the selected sensing, control, actuator, communication, health, and fault
/// components. Confirm timing budget and safe output state before enabling the actuator path.
#include "uef/uapp/uapp_lifecycle.h"
#include "uef/umid/umid_imu.h"
#include "uef/uproto/uproto_crsf.h"
#include "uef/uproto/uproto_dshot.h"

int main(void) {
    /* TODO(full-motor-control main): Initialize sensing, estimator, controller,
     * allocation, communications, health, and fault components in dependency order.
     * Check timing/workspace budgets and sensor validity before enabling any actuator;
     * keep outputs in the board-safe state until explicit arming, and define shutdown and
     * degraded-mode behavior for each startup/runtime failure.
     */
    return 0;
}
