# adc_hrtim_triggered

V1.1 example slot for this hardware scenario. The selected board, pin/clock/DMA
routing, external device revision, and required UEF modules must be recorded
before the example is treated as runnable.

## Implementation notes

Configure an HRTIM-triggered ADC/DMA sample path. Record trigger phase, channel order, sample time, timer frequency, cache rules, and shutdown-on-fault behavior.

The main.c file is a compile-only starting point, not a working hardware
example. It must not be used on hardware until the board-specific steps and
safety behavior have been implemented and reviewed.