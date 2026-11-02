/// @file include/upal/display/dsi_host.h
/// @brief First-pass scaffold: MIPI DSI host with explicit D-PHY/C-PHY profile.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/MIPI_DISPLAY_LINK.md.
#ifndef UETD_UPAL_DSI_HOST_H
#define UETD_UPAL_DSI_HOST_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Intended implementation: serialized task/main owner; no hidden heap.
 * Configurations, provider context and backing storage outlive the object.
 * Explicitly asynchronous operations retain buffers only after admission and
 * until terminal completion; synchronous spans are borrowed only for the call.
 * Schema IDs bind typed, versioned layouts/units; they do not permit arbitrary
 * native structs as a wire representation. Output/state commits and callback
 * context must be reviewed against the linked design before implementation.
 */
typedef enum { UPAL_DSI_PROTOCOL_DSI = 0, UPAL_DSI_PROTOCOL_DSI2 } upal_dsi_protocol_t;
typedef enum { UPAL_DSI_PHY_D = 0, UPAL_DSI_PHY_C } upal_dsi_phy_t;
typedef enum { UPAL_DSI_COMMAND_MODE = 0, UPAL_DSI_VIDEO_MODE } upal_dsi_mode_t;
typedef enum { UPAL_DSI_FORWARDED_CLOCK = 0, UPAL_DSI_EMBEDDED_CLOCK } upal_dsi_clock_t;
typedef enum { UPAL_DSI_C_ENCODING_NONE = 0, UPAL_DSI_C_ENCODING_6_STATE, UPAL_DSI_C_ENCODING_18_STATE } upal_dsi_c_encoding_t;
typedef struct {
    upal_dsi_protocol_t protocol; upal_dsi_phy_t phy; upal_dsi_mode_t mode;
    upal_dsi_clock_t clock_mode; upal_dsi_c_encoding_t c_encoding;
    uetd_u16_t protocol_revision; uetd_u16_t phy_revision;
    uetd_u8_t lane_or_trio_count; uetd_u8_t lane_or_trio_map[4];
    uetd_u64_t d_lane_bits_per_second; uetd_u64_t c_trio_symbols_per_second;
    uetd_u32_t active_width, active_height, h_front, h_sync, h_back, v_front, v_sync, v_back;
    uetd_u64_t pixel_clock_hz; uetd_u8_t wire_bits_per_pixel; uetd_u8_t virtual_channel;
    bool compression_enabled; size_t maximum_command_bytes; void* provider_context;
} upal_dsi_config_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } upal_dsi_host_t;
typedef enum { UPAL_DSI_DCS_WRITE = 0, UPAL_DSI_GENERIC_WRITE, UPAL_DSI_DCS_READ, UPAL_DSI_GENERIC_READ } upal_dsi_transfer_kind_t;
typedef struct { upal_dsi_transfer_kind_t kind; const uetd_u8_t* tx; size_t tx_bytes; uetd_u8_t* rx; size_t rx_capacity; bool low_power; ucore_deadline_t deadline; } upal_dsi_transfer_t;

/// Check host/panel protocol and PHY revisions, lane versus trio rate units, timing, encoding and resource constraints.
uetd_status_t upal_dsi_validate(const upal_dsi_config_t* config);
/// Bind qualified controller/PHY resources without pretending a missing transmitter is available.
uetd_status_t upal_dsi_init(upal_dsi_host_t* host, const upal_dsi_config_t* config);
/// Bound command/read lengths and LP/HS mode; borrow buffers only after successful admission.
uetd_status_t upal_dsi_submit(upal_dsi_host_t* host, const upal_dsi_transfer_t* transfer, upal_transaction_token_t* token);
/// Service a bounded number of events and enforce actual transfer deadlines.
uetd_status_t upal_dsi_poll(upal_dsi_host_t* host, uetd_u16_t work_budget);
/// Report exact received/transmitted count and terminal quiescence; preserve output on no completion.
uetd_status_t upal_dsi_try_complete(upal_dsi_host_t* host, upal_transaction_completion_t* completion);
/// Quiesce or drain hardware before terminal buffer release; cancellation admission alone is insufficient.
uetd_status_t upal_dsi_cancel(upal_dsi_host_t* host, upal_transaction_token_t token);
/// Reject live transfers or use the explicit cancellation path before releasing clocks and PHY resources.
uetd_status_t upal_dsi_stop(upal_dsi_host_t* host);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPAL_DSI_HOST_H */
