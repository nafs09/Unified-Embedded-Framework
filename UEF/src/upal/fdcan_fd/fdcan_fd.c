/// @file src/upal/fdcan_fd/fdcan_fd.c
/// @brief Phase 1 FDCAN-FD API skeleton with fail-closed target hooks.

#include <uef/upal/upal_fdcan_fd.h>

static bool fdcan_length_valid(uef_u8_t length)
{
    /* CAN-FD DLCs above eight bytes have only the standardized payload sizes. */
    return length <= 8U || length == 12U || length == 16U || length == 20U ||
           length == 24U || length == 32U || length == 48U || length == 64U;
}

static bool fdcan_frame_valid(const upal_fdcan_fd_frame_t *frame)
{
    if (frame == NULL || frame->data_length > UPAL_FDCAN_FD_MAX_DATA_BYTES) {
        return false;
    }
    if (!fdcan_length_valid(frame->data_length) ||
        (!frame->flexible_data_rate && frame->data_length > 8U)) {
        return false;
    }
    if ((frame->bit_rate_switching || frame->error_state_indicator) &&
        !frame->flexible_data_rate) {
        return false;
    }
    if (frame->extended_identifier) {
        return frame->identifier <= 0x1FFFFFFFU;
    }
    return frame->identifier <= 0x7FFU;
}

uef_status_t upal_fdcan_fd_init(upal_fdcan_fd_t *driver,
                                const upal_fdcan_fd_config_t *config)
{
    if (driver == NULL || config == NULL || config->instance == NULL ||
        config->nominal_bitrate == 0U || config->data_bitrate == 0U ||
        config->tx_element_count == 0U || config->rx_element_count == 0U ||
        config->nominal_sample_point_percent == 0U ||
        config->nominal_sample_point_percent >= 100U ||
        config->data_sample_point_percent == 0U ||
        config->data_sample_point_percent >= 100U) {
        return UEF_INVALID_ARG;
    }
    /*
     * TODO(UPAL-FDCAN-FD): Select target message-RAM layout, validate nominal/data
     * timing against the peripheral clock, program filters/FIFO element sizes,
     * clear protocol state, and attach the IRQ/DMA hooks for the selected part.
     * Keep the generic frame contract independent of vendor HAL structures.
     */
    /* Preserve the caller's driver object until target initialization exists. */
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_fdcan_fd_deinit(upal_fdcan_fd_t *driver)
{
    if (driver == NULL || driver->config == NULL) {
        return UEF_INVALID_ARG;
    }
    /*
     * TODO(UPAL-FDCAN-FD): Disable the target interrupt sources, drain or abort
     * outstanding transmissions according to the documented policy, restore the
     * peripheral to the board-owned state, and clear initialized only on success.
     */
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_fdcan_fd_transmit(upal_fdcan_fd_t *driver,
                                    const upal_fdcan_fd_frame_t *frame)
{
    if (driver == NULL || !driver->initialized || !fdcan_frame_valid(frame)) {
        return UEF_INVALID_ARG;
    }
    /*
     * TODO(UPAL-FDCAN-FD): Convert the generic ID/DLC/FD/BRS/ESI fields to the
     * selected target's message-RAM element, preserve caller data, and report
     * queue-full separately from hardware faults. Define ISR/thread safety.
     */
    return UEF_NOT_SUPPORTED;
}

uef_status_t upal_fdcan_fd_receive(upal_fdcan_fd_t *driver,
                                   upal_fdcan_fd_frame_t *frame)
{
    if (driver == NULL || frame == NULL || !driver->initialized) {
        return UEF_INVALID_ARG;
    }
    /*
     * TODO(UPAL-FDCAN-FD): Poll or dequeue exactly one complete frame, translate
     * target DLC and error flags, reject malformed lengths without overrun, and
     * document whether an empty FIFO returns UEF_NOT_READY or UEF_BUSY.
     */
    return UEF_NOT_SUPPORTED;
}
