/// @file src/upal/can/can.c
/// @brief Source scaffold for the V1.2 public contract in uef/upal/upal_can.h.
///
/// Implementation intent: Validate standard/extended identifiers and frame length; keep bxCAN
///   and FDCAN-specific differences behind the API boundary.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/upal/upal_can.h>

uef_status_t upal_can_init(
    upal_can_t* c,
    const upal_can_hw_t* hw,
    const upal_can_cfg_t* cfg,
    upal_can_rx_cb_t rx_cb,
    void* rx_ctx
) {
    /* TODO(UEF UPAL CAN):
     * Validate all descriptors and configuration, enable/reset the peripheral in the
     * required order, and publish a ready state only after setup succeeds. Validate
     * standard/extended identifiers, configure filters without disrupting existing entries,
     * and define callback execution context.
     */
    (void)c;
    (void)hw;
    (void)cfg;
    (void)rx_cb;
    (void)rx_ctx;
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_can_add_filter(
    upal_can_t* c,
    uef_u32_t id,
    uef_u32_t mask,
    bool extended
) {
    /* TODO(UEF UPAL CAN):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Validate
     * standard/extended identifiers, configure filters without disrupting existing entries,
     * and define callback execution context.
     */
    (void)c;
    (void)id;
    (void)mask;
    (void)extended;
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_can_transmit(
    upal_can_t* c,
    const upal_can_frame_t* f,
    uef_u32_t timeout_ms
) {
    /* TODO(UEF UPAL CAN):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Validate
     * standard/extended identifiers, configure filters without disrupting existing entries,
     * and define callback execution context.
     */
    (void)c;
    (void)f;
    (void)timeout_ms;
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_can_transmit_async(
    upal_can_t* c,
    const upal_can_frame_t* f
) {
    /* TODO(UEF UPAL CAN):
     * Wait until all queued/buffered hardware work is committed, bounded by the caller
     * deadline. Validate standard/extended identifiers, configure filters without
     * disrupting existing entries, and define callback execution context.
     */
    (void)c;
    (void)f;
    return UEF_NOT_SUPPORTED;
}

void upal_can_rx_irq_handler(
    upal_can_t* c
) {
    /* TODO(UEF UPAL CAN):
     * Read and clear the pending source flags, update only the owning module state, and
     * defer non-ISR-safe callbacks/work. Validate standard/extended identifiers, configure
     * filters without disrupting existing entries, and define callback execution context.
     */
    (void)c;
}

void upal_can_tx_irq_handler(
    upal_can_t* c
) {
    /* TODO(UEF UPAL CAN):
     * Read and clear the pending source flags, update only the owning module state, and
     * defer non-ISR-safe callbacks/work. Validate standard/extended identifiers, configure
     * filters without disrupting existing entries, and define callback execution context.
     */
    (void)c;
}

void upal_can_err_irq_handler(
    upal_can_t* c
) {
    /* TODO(UEF UPAL CAN):
     * Read and clear the pending source flags, update only the owning module state, and
     * defer non-ISR-safe callbacks/work. Validate standard/extended identifiers, configure
     * filters without disrupting existing entries, and define callback execution context.
     */
    (void)c;
}
