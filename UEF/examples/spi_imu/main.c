/// @file examples/spi_imu/main.c
/// @brief Hardware example scaffold: spi_imu.
///
/// TODO: Bind one selected IMU to SPI, configure its sample rate, and publish calibrated,
/// timestamped data. Get the register map and calibration constants from the chosen device.
#include "uef/umid/umid_imu.h"

int main(void) {
    /* TODO(spi-imu main): Initialize the selected board SPI instance and one explicitly
     * supported IMU, verify device identity, configure sample rate/range, and establish
     * chip-select/transfer ownership. Read timestamped samples with bounded deadlines,
     * apply declared scale/bias calibration, and publish a coherent sample only on success.
     */
    return 0;
}
