# baremetal_minimal

V1.1 example slot for this hardware scenario. The selected board, pin/clock/DMA
routing, external device revision, and required UEF modules must be recorded
before the example is treated as runnable.

## Implementation notes

Set up the board clock, initialize one verified GPIO, then blink it from the application main loop. Use the pin mapping from the selected board, not a family-wide assumption.

The main.c file is a compile-only starting point, not a working hardware
example. It must not be used on hardware until the board-specific steps and
safety behavior have been implemented and reviewed.