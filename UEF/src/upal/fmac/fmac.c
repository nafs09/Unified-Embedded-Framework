/// @file src/upal/fmac/fmac.c
/// @brief Source scaffold for the V1.1 public contract in uef/upal/upal_fmac.h.
///
/// Implementation intent: Implement accelerator configuration and bounded input/output
///   transfer; define coefficient and scaling formats.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/upal/upal_fmac.h>

uef_status_t upal_fmac_init(
    const upal_fmac_cfg_t* cfg
) {
    /* TODO(UEF UPAL FMAC):
     * Validate all descriptors and configuration, enable/reset the peripheral in the
     * required order, and publish a ready state only after setup succeeds. Document
     * coefficient/sample ordering and fixed-point scaling, handle FIFO saturation, and keep
     * interrupt-side work bounded.
     */
    (void)cfg;
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_fmac_write(
    const uef_q15_t* input,
    uef_u32_t count
) {
    /* TODO(UEF UPAL FMAC):
     * Validate the full payload and peripheral state, then report completion only after the
     * hardware accepts or finishes the transfer as promised. Document coefficient/sample
     * ordering and fixed-point scaling, handle FIFO saturation, and keep interrupt-side
     * work bounded.
     */
    (void)input;
    (void)count;
    return UEF_NOT_SUPPORTED;
}

uef_u32_t upal_fmac_read(
    uef_q15_t* output,
    uef_u32_t max_count
) {
    /* TODO(UEF UPAL FMAC):
     * Check output capacity and readiness before touching hardware; return fresh data only
     * and preserve caller storage on failure. Document coefficient/sample ordering and
     * fixed-point scaling, handle FIFO saturation, and keep interrupt-side work bounded.
     */
    (void)output;
    (void)max_count;
    return 0;
}

void upal_fmac_irq_handler(void) {
    /* TODO(UEF UPAL FMAC):
     * Read and clear the pending source flags, update only the owning module state, and
     * defer non-ISR-safe callbacks/work. Document coefficient/sample ordering and
     * fixed-point scaling, handle FIFO saturation, and keep interrupt-side work bounded.
     */
}
