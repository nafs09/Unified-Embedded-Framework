/// @file src/upal/display/dsi_host.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "upal/display/dsi_host.h"

uetd_status_t upal_dsi_validate(const upal_dsi_config_t* config) {
    /* TODO: Check host/panel protocol and PHY revisions, lane versus trio rate units, timing, encoding and resource constraints. */
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_dsi_init(upal_dsi_host_t* host, const upal_dsi_config_t* config) {
    /* TODO: Bind qualified controller/PHY resources without pretending a missing transmitter is available. */
    (void)host;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_dsi_submit(upal_dsi_host_t* host, const upal_dsi_transfer_t* transfer, upal_transaction_token_t* token) {
    /* TODO: Bound command/read lengths and LP/HS mode; borrow buffers only after successful admission. */
    (void)host;
    (void)transfer;
    (void)token;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_dsi_poll(upal_dsi_host_t* host, uetd_u16_t work_budget) {
    /* TODO: Service a bounded number of events and enforce actual transfer deadlines. */
    (void)host;
    (void)work_budget;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_dsi_try_complete(upal_dsi_host_t* host, upal_transaction_completion_t* completion) {
    /* TODO: Report exact received/transmitted count and terminal quiescence; preserve output on no completion. */
    (void)host;
    (void)completion;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_dsi_cancel(upal_dsi_host_t* host, upal_transaction_token_t token) {
    /* TODO: Quiesce or drain hardware before terminal buffer release; cancellation admission alone is insufficient. */
    (void)host;
    (void)token;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_dsi_stop(upal_dsi_host_t* host) {
    /* TODO: Reject live transfers or use the explicit cancellation path before releasing clocks and PHY resources. */
    (void)host;
    return UETD_NOT_SUPPORTED;
}

