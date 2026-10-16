# HRTIM-triggered ADC scaffold

This source scaffold outlines an HRTIM-triggered ADC/DMA sampling chain. The
intended path records trigger phase, channel order, sample time, timer rate,
buffer ownership, and shutdown behavior.

No target-specific trigger, channel, or DMA configuration is implemented.
Outputs must remain disabled during setup, and transfer or initialization
failures require a defined safe stop. The example is not built by default and
is not ready to run on hardware.
