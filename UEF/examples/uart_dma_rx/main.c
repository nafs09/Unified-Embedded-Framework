/// @file examples/uart_dma_rx/main.c
/// @brief Hardware example scaffold: uart_dma_rx.
///
/// TODO: Configure a UART receive buffer and DMA/IDLE handling. Document buffer ownership,
/// overflow reporting, IRQ priority, and how bytes reach a task or parser.
#include "uef/upal/upal_uart.h"

int main(void) {
    /* TODO(uart-dma-rx main): Initialize UART framing/baud and a statically allocated,
     * DMA-aligned RX buffer; start DMA/IDLE handling only after cache and ownership rules
     * are configured. Consume completed spans outside the IRQ, detect ring overrun and
     * framing errors, and rearm without exposing bytes still owned by DMA.
     */
    return 0;
}
