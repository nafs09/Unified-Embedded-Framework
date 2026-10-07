/// @file include/uef/upal/upal_fdcan_fd.h
/// @brief Phase 1 CAN-FD frame contract, separate from the classic CAN API.

#ifndef UEF_UPAL_FDCAN_FD_H
#define UEF_UPAL_FDCAN_FD_H

#include <uef/ucore/uef_status.h>
#include <uef/ucore/uef_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UPAL_FDCAN_FD_MAX_DATA_BYTES 64U

typedef struct {
    uef_u32_t identifier;
    uef_u8_t data_length;
    bool extended_identifier;
    bool flexible_data_rate;
    bool bit_rate_switching;
    bool error_state_indicator;
    uef_u8_t data[UPAL_FDCAN_FD_MAX_DATA_BYTES];
} upal_fdcan_fd_frame_t;

typedef struct {
    void *instance;                    /* Vendor FDCAN handle owned by the board layer. */
    uef_u32_t nominal_bitrate;
    uef_u32_t data_bitrate;
    uef_u16_t tx_element_count;
    uef_u16_t rx_element_count;
    uef_u8_t nominal_sample_point_percent;
    uef_u8_t data_sample_point_percent;
} upal_fdcan_fd_config_t;

typedef struct {
    void *instance;
    /* Caller-owned configuration; it must outlive a successfully initialized driver. */
    const upal_fdcan_fd_config_t *config;
    bool initialized;
} upal_fdcan_fd_t;

/// Validate configuration and initialize one target-selected FDCAN-FD instance.
uef_status_t upal_fdcan_fd_init(upal_fdcan_fd_t *driver,
                                const upal_fdcan_fd_config_t *config);

/// Stop an initialized FDCAN-FD instance and release target-owned resources.
uef_status_t upal_fdcan_fd_deinit(upal_fdcan_fd_t *driver);

/// Queue one classic CAN or CAN-FD frame for transmission. CAN-FD payload sizes
/// must match a legal DLC length (0-8, 12, 16, 20, 24, 32, 48, or 64 bytes);
/// BRS and ESI flags are valid only when flexible_data_rate is true.
uef_status_t upal_fdcan_fd_transmit(upal_fdcan_fd_t *driver,
                                    const upal_fdcan_fd_frame_t *frame);

/// Read one received frame into caller-owned storage.
uef_status_t upal_fdcan_fd_receive(upal_fdcan_fd_t *driver,
                                   upal_fdcan_fd_frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* UEF_UPAL_FDCAN_FD_H */
