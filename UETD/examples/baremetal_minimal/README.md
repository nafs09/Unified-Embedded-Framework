# Bare-metal GPIO scaffold

This source scaffold outlines a minimal board bring-up path: initialize the
board clock, configure one GPIO, and toggle it from the application loop.

`main.c` does not yet configure hardware. A selected board profile must supply
the clock and pin mapping, bounded delay behavior, and a safe startup-error
path. The example is not built by default and is not ready to run on hardware.
