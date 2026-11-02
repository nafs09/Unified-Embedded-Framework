/// @file include/upal/network/ethernet.h
/// @brief Ethernet MAC/PHY profile layered over the generic UPAL netif.
///
/// The MAC provider owns descriptor/DMA/cache rules and frame delivery through
/// the bound netif. PHY status reports negotiated link facts. Exact PHY bus,
/// pins, clock, DMA, cache and interrupt ownership come from the selected board
/// profile; this header does not invent those target details.

#ifndef UETD_UPAL_ETHERNET_H
#define UETD_UPAL_ETHERNET_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/network/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UPAL_ETHERNET_SPEED_UNKNOWN = 0,
    UPAL_ETHERNET_SPEED_10_MBPS,
    UPAL_ETHERNET_SPEED_100_MBPS,
    UPAL_ETHERNET_SPEED_1_GBPS,
} upal_ethernet_speed_t;

typedef struct {
    bool link_up;
    bool full_duplex;
    bool autonegotiated;
    bool pause_rx;
    bool pause_tx;
    upal_ethernet_speed_t speed;
    uetd_u8_t phy_address;
    uetd_u32_t rx_errors;
    uetd_u32_t tx_errors;
} upal_ethernet_status_t;

typedef struct {
    uetd_status_t (*read_status)(void* context, upal_ethernet_status_t* status);
} upal_ethernet_ops_t;

typedef struct {
    upal_netif_t* netif;
    const upal_ethernet_ops_t* ops;
    void* context;
    bool bound;
} upal_ethernet_t;

uetd_status_t upal_ethernet_bind(upal_ethernet_t* ethernet, upal_netif_t* netif, const upal_ethernet_ops_t* ops, void* context);
uetd_status_t upal_ethernet_get_status(upal_ethernet_t* ethernet, upal_ethernet_status_t* status);
uetd_status_t upal_ethernet_start(upal_ethernet_t* ethernet);
uetd_status_t upal_ethernet_stop(upal_ethernet_t* ethernet);
uetd_status_t upal_ethernet_poll(upal_ethernet_t* ethernet, uetd_u16_t work_budget);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPAL_ETHERNET_H */
