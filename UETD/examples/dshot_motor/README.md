# DSHOT motor scaffold

This source scaffold outlines a DSHOT command path over timer/DMA or UART
transport. It is intended to keep the ESC disarmed during setup and leave
arming and failsafe decisions with the application.

The DSHOT rate, timer and pin timing, DMA buffer ownership, board transport,
and safe output behavior are not implemented. The example is not built by
default and is not ready to run on hardware.
