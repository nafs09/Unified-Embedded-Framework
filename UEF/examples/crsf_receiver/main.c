/// @file examples/crsf_receiver/main.c
/// @brief Hardware example scaffold: crsf_receiver.
///
/// TODO: Configure the serial transport, validate CRSF framing/CRC, then publish RC state with
/// age/failsafe metadata. Do not update channels from partial frames.
#include "uef/uproto/uproto_crsf.h"

int main(void) {
    /* TODO(crsf-receiver main): Initialize the board UART at the selected CRSF settings,
     * bind the bounded DMA/ring producer, and initialize the decoder. Process a bounded
     * byte/frame budget outside the hard IRQ, publish only CRC-valid RC snapshots, and
     * assert the configured stale-link/failsafe state when freshness expires.
     */
    return 0;
}
