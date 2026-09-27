/// @file src/umid/log.c
/// @brief Source scaffold for the V1.1 public contract in uef/umid/umid_log.h.
///
/// Implementation intent: Format into bounded caller-owned storage, enqueue without heap
///   allocation, and drain through the UART in a low-priority context.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_log.h>

void umid_log_init(
    upal_uart_t* uart,
    umid_log_level_t min_level
) {
    /* TODO(UEF UMID logging init):
     * Store the UART and minimum level in bounded shared state, define who owns UART setup,
     * initialize the fixed-capacity queue, and document concurrency/ISR guarantees.
     */
    (void)uart;
    (void)min_level;
}

void umid_log(
    umid_log_level_t level,
    const char* tag,
    const char* fmt,
    ...
) {
    /* TODO(UEF UMID log message):
     * Filter by minimum level before formatting, format into a fixed-size record without heap
     * allocation, define truncation/newline behavior, and enqueue atomically. Do not block or
     * call a non-ISR-safe formatter from interrupt context; count dropped records explicitly.
     */
    (void)level;
    (void)tag;
    (void)fmt;
}

void umid_log_hex(
    umid_log_level_t level,
    const char* tag,
    const uef_u8_t* data,
    uef_u32_t len
) {
    /* TODO(UEF UMID hex logging):
     * Validate the byte pointer for nonzero length, filter by level, and encode a bounded hex
     * preview with a documented truncation marker. Reuse the same queue and concurrency contract
     * as umid_log rather than writing directly from the caller's context.
     */
    (void)level;
    (void)tag;
    (void)data;
    (void)len;
}
