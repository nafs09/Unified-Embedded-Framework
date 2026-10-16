# SPI IMU scaffold

This source scaffold outlines initialization and sampling for one selected
SPI-connected IMU, including identity checks, sample timing, calibration, and
timestamped data publication.

The device, register map, SPI instance, transfer ownership, scale and bias
calibration, and sample deadline remain target-specific inputs. The example is
not built by default and is not ready to run on hardware.
