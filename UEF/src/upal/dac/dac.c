/// @file src/upal/dac/dac.c
/// @brief Source scaffold for the V1.2 public contract in uef/upal/upal_dac.h.
///
/// Implementation intent: Validate channel and resolution before writes; make DMA waveform
///   start/stop state explicit.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/upal/upal_dac.h>

uef_status_t upal_dac_init(
    upal_dac_t* d,
    const upal_dac_hw_t* hw
) {
    /* TODO(UEF UPAL DAC):
     * Validate all descriptors and configuration, enable/reset the peripheral in the
     * required order, and publish a ready state only after setup succeeds. Validate channel
     * and 12-bit values, and define sample-buffer lifetime and DMA completion behavior.
     */
    (void)d;
    (void)hw;
    return UEF_NOT_SUPPORTED;
}

void upal_dac_write(
    upal_dac_t* d,
    uef_u8_t channel,
    uef_u16_t value_12bit
) {
    /* TODO(UEF UPAL DAC):
     * Validate the full payload and peripheral state, then report completion only after the
     * hardware accepts or finishes the transfer as promised. Validate channel and 12-bit
     * values, and define sample-buffer lifetime and DMA completion behavior.
     */
    (void)d;
    (void)channel;
    (void)value_12bit;
}

uef_status_t upal_dac_dma_start(
    upal_dac_t* d,
    uef_u8_t channel,
    const uef_u16_t* samples,
    uef_u32_t count,
    bool circular,
    upal_dma_callback_t cb,
    void* ctx
) {
    /* TODO(UEF UPAL DAC):
     * Reject invalid state, arm the peripheral and DMA in the documented order, and roll
     * back partially configured resources on failure. Validate channel and 12-bit values,
     * and define sample-buffer lifetime and DMA completion behavior.
     */
    (void)d;
    (void)channel;
    (void)samples;
    (void)count;
    (void)circular;
    (void)cb;
    (void)ctx;
    return UEF_NOT_SUPPORTED;
}

void upal_dac_dma_stop(
    upal_dac_t* d,
    uef_u8_t channel
) {
    /* TODO(UEF UPAL DAC):
     * Disable new requests first, stop/abort any active transfer, clear owned flags, and
     * leave the module in a documented restartable state. Validate channel and 12-bit
     * values, and define sample-buffer lifetime and DMA completion behavior.
     */
    (void)d;
    (void)channel;
}
