/// @file src/upal/spi/spi.c
/// @brief Source scaffold for the V1.1 public contract in uef/upal/upal_spi.h.
///
/// Implementation intent: Implement full-duplex transfers, chip-select timing, and DMA
///   completion without changing caller-owned buffer lifetime.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/upal/upal_spi.h>

uef_status_t upal_spi_init(
    upal_spi_t* s,
    const upal_spi_hw_t* hw,
    uef_u32_t clock_hz,
    upal_spi_mode_t mode
) {
    /* TODO(UEF UPAL SPI):
     * Validate all descriptors and configuration, enable/reset the peripheral in the
     * required order, and publish a ready state only after setup succeeds. Keep chip-select
     * asserted across each full transfer, validate full-duplex buffer rules, and coordinate
     * DMA completion and timeout.
     */
    (void)s;
    (void)hw;
    (void)clock_hz;
    (void)mode;
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_spi_transfer_dma(
    upal_spi_t* s,
    uhal_gpio_pin_t cs_pin,
    const uef_u8_t* tx,
    uef_u8_t* rx,
    uef_u32_t len,
    upal_dma_callback_t cb,
    void* ctx
) {
    /* TODO(UEF UPAL SPI):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Keep chip-select
     * asserted across each full transfer, validate full-duplex buffer rules, and coordinate
     * DMA completion and timeout.
     */
    (void)s;
    (void)cs_pin;
    (void)tx;
    (void)rx;
    (void)len;
    (void)cb;
    (void)ctx;
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_spi_transfer_blocking(
    upal_spi_t* s,
    uhal_gpio_pin_t cs_pin,
    const uef_u8_t* tx,
    uef_u8_t* rx,
    uef_u32_t len,
    uef_u32_t timeout_ms
) {
    /* TODO(UEF UPAL SPI):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Keep chip-select
     * asserted across each full transfer, validate full-duplex buffer rules, and coordinate
     * DMA completion and timeout.
     */
    (void)s;
    (void)cs_pin;
    (void)tx;
    (void)rx;
    (void)len;
    (void)timeout_ms;
    return UEF_NOT_SUPPORTED;
}

void upal_spi_dma_handler(
    upal_spi_t* s
) {
    /* TODO(UEF UPAL SPI):
     * Reconcile DMA and peripheral completion flags, perform cache maintenance where
     * required, and issue exactly one completion callback. Keep chip-select asserted across
     * each full transfer, validate full-duplex buffer rules, and coordinate DMA completion
     * and timeout.
     */
    (void)s;
}
