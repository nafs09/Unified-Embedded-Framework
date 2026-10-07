/// @file src/upal/iwdg/iwdg.c
/// @brief Source scaffold for the V1.2 public contract in uef/upal/upal_iwdg.h.
///
/// Implementation intent: Configure the independent watchdog from the verified clock source and
///   reject unrepresentable timeout requests.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/upal/upal_iwdg.h>

uef_status_t upal_iwdg_init(
    uef_u32_t timeout_ms
) {
    /* TODO(UEF UPAL IWDG):
     * Validate all descriptors and configuration, enable/reset the peripheral in the
     * required order, and publish a ready state only after setup succeeds. Convert timeout
     * to the hardware prescaler/reload range and reject values the selected clock cannot
     * represent.
     */
    (void)timeout_ms;
    return UEF_NOT_SUPPORTED;
}

void upal_iwdg_refresh(void) {
    /* TODO(UEF UPAL IWDG):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Convert timeout to
     * the hardware prescaler/reload range and reject values the selected clock cannot
     * represent.
     */
}
