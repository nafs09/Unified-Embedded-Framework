# freertos_tasks

V1.1 example slot for this hardware scenario. The selected board, pin/clock/DMA
routing, external device revision, and required UEF modules must be recorded
before the example is treated as runnable.

## Implementation notes

Create statically stored UOS tasks and synchronization objects, provide caller-owned stacks/queue buffers, and transfer control to the scheduler after all objects are ready.

The main.c file is a compile-only starting point, not a working hardware
example. It must not be used on hardware until the board-specific steps and
safety behavior have been implemented and reviewed.