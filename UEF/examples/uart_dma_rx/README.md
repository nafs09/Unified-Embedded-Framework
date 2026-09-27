# uart_dma_rx

V1.1 example slot for this hardware scenario. The selected board, pin/clock/DMA
routing, external device revision, and required UEF modules must be recorded
before the example is treated as runnable.

## Implementation notes

Configure a UART receive buffer and DMA/IDLE handling. Document buffer ownership, overflow reporting, IRQ priority, and how bytes are handed to a task or parser.

The main.c file is a compile-only starting point, not a working hardware
example. It must not be used on hardware until the board-specific steps and
safety behavior have been implemented and reviewed.