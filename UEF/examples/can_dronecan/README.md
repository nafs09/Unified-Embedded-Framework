# can_dronecan

V1.1 example slot for this hardware scenario. The selected board, pin/clock/DMA
routing, external device revision, and required UEF modules must be recorded
before the example is treated as runnable.

## Implementation notes

Configure CAN filters and the selected DroneCAN/UAVCAN library binding. Record node identity, transfer-ID policy, memory limits, and bus-off recovery.

The main.c file is a compile-only starting point, not a working hardware
example. It must not be used on hardware until the board-specific steps and
safety behavior have been implemented and reviewed.