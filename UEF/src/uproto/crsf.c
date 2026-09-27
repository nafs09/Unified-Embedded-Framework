/// @file src/uproto/crsf.c
/// @brief Source scaffold for the V1.1 public contract in uef/uproto/uproto_crsf.h.
///
/// Implementation intent: Validate frame length/type/CRC before updating RC state; keep decode
///   work bounded in the receive path.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uproto/uproto_crsf.h>

uef_status_t uproto_crsf_init(
    uproto_crsf_t* c,
    upal_uart_t* uart
) {
    /* TODO(UEF UPROTO CRSF init):
     * Validate the decoder and UART, configure the required baud/framing, initialize the
     * bounded RX ring, and attach its producer to the UART DMA/IDLE path. Keep ISR and parser
     * ownership separate so frame decoding never races the byte producer.
     */
    (void)c;
    (void)uart;
    return UEF_NOT_SUPPORTED;
}

void uproto_crsf_process(
    uproto_crsf_t* c
) {
    /* TODO(UEF UPROTO CRSF process):
     * Consume a bounded number of bytes from the ring, resynchronize using the address/length
     * fields, validate frame length and CRC, and decode only supported frame types. Publish RC
     * and link metadata atomically after full validation; increment error/frame counters without
     * discarding a previously valid RC sample on malformed input.
     */
    (void)c;
}

bool uproto_crsf_rc_available(
    const uproto_crsf_t* c
) {
    /* TODO(UEF UPROTO CRSF availability):
     * Check that at least one validated RC frame exists and apply the documented link-stale
     * timeout using `last_rx_us`. Define whether each call consumes a new-frame marker.
     */
    (void)c;
    return false;
}

void uproto_crsf_get_rc(
    uproto_crsf_t* c,
    uproto_crsf_rc_t* out
) {
    /* TODO(UEF UPROTO CRSF get):
     * Validate the output pointer and copy one coherent last-valid RC sample, including link
     * metadata. Preserve caller storage when no valid sample exists and define freshness/reset
     * behavior consistently with `uproto_crsf_rc_available`.
     */
    (void)c;
    (void)out;
}

uef_status_t uproto_crsf_send_battery(
    uproto_crsf_t* c,
    uef_f32_t voltage_v,
    uef_f32_t current_a,
    uef_u32_t capacity_mah,
    uef_u8_t  remaining_pct
) {
    /* TODO(UEF UPROTO CRSF battery telemetry):
     * Validate finite/range-safe voltage and current plus the remaining percentage, encode the
     * CRSF battery sensor payload with the specified scaling/endianness, append CRC, and submit
     * through a bounded UART TX path. Return transport failure rather than claiming delivery.
     */
    (void)c;
    (void)voltage_v;
    (void)current_a;
    (void)capacity_mah;
    (void)remaining_pct;
    return UEF_NOT_SUPPORTED;
}

uef_status_t uproto_crsf_send_attitude(
    uproto_crsf_t* c,
    uef_f32_t roll_rad,
    uef_f32_t pitch_rad,
    uef_f32_t yaw_rad
) {
    /* TODO(UEF UPROTO CRSF attitude telemetry):
     * Validate finite roll/pitch/yaw radians, convert to the protocol's documented scaled
     * representation with defined rounding/saturation, build the telemetry frame and CRC, and
     * submit it through the same bounded TX ownership path as battery telemetry.
     */
    (void)c;
    (void)roll_rad;
    (void)pitch_rad;
    (void)yaw_rad;
    return UEF_NOT_SUPPORTED;
}
