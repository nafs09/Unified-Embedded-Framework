/// @file src/upal/flash/flash.c
/// @brief Source scaffold for the V1.1 public contract in uef/upal/upal_flash.h.
///
/// Implementation intent: Implement unlock, erase, program, verify, and relock sequences with
///   power-loss and alignment constraints documented.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/upal/upal_flash.h>

uef_status_t upal_flash_unlock(void) {
    /* TODO(UEF UPAL FLASH):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Check
     * address/length alignment and permitted flash range, honor erase/program timing, and
     * never claim success before verifying controller status.
     */
    return UEF_NOT_SUPPORTED;
}

void upal_flash_lock(void) {
    /* TODO(UEF UPAL FLASH):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Check
     * address/length alignment and permitted flash range, honor erase/program timing, and
     * never claim success before verifying controller status.
     */
}

uef_status_t upal_flash_erase_page(
    uef_u32_t page_addr,
    uef_u32_t timeout_ms
) {
    /* TODO(UEF UPAL FLASH):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Check
     * address/length alignment and permitted flash range, honor erase/program timing, and
     * never claim success before verifying controller status.
     */
    (void)page_addr;
    (void)timeout_ms;
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_flash_write_dword(
    uef_u32_t addr,
    uef_u64_t data
) {
    /* TODO(UEF UPAL FLASH):
     * Validate the full payload and peripheral state, then report completion only after the
     * hardware accepts or finishes the transfer as promised. Check address/length alignment
     * and permitted flash range, honor erase/program timing, and never claim success before
     * verifying controller status.
     */
    (void)addr;
    (void)data;
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_flash_write(
    uef_u32_t addr,
    const uef_u8_t* data,
    uef_u32_t len,
    uef_u32_t timeout_ms
) {
    /* TODO(UEF UPAL FLASH):
     * Validate the full payload and peripheral state, then report completion only after the
     * hardware accepts or finishes the transfer as promised. Check address/length alignment
     * and permitted flash range, honor erase/program timing, and never claim success before
     * verifying controller status.
     */
    (void)addr;
    (void)data;
    (void)len;
    (void)timeout_ms;
    return UEF_NOT_SUPPORTED;
}

bool upal_flash_verify(
    uef_u32_t addr,
    const uef_u8_t* expected,
    uef_u32_t len
) {
    /* TODO(UEF UPAL FLASH):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Check
     * address/length alignment and permitted flash range, honor erase/program timing, and
     * never claim success before verifying controller status.
     */
    (void)addr;
    (void)expected;
    (void)len;
    return false;
}
