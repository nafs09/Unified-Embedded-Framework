# spi_imu

V1.1 example slot for this hardware scenario. The selected board, pin/clock/DMA
routing, external device revision, and required UEF modules must be recorded
before the example is treated as runnable.

## Implementation notes

Bind one selected IMU to SPI, configure its sample rate, and publish calibrated timestamped data. The sensor register map and calibration constants must come from the chosen device.

The main.c file is a compile-only starting point, not a working hardware
example. It must not be used on hardware until the board-specific steps and
safety behavior have been implemented and reviewed.