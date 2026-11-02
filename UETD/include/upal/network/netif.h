/// @file include/upal/network/netif.h
/// @brief Fixed-buffer packet interface between a link provider and a network stack.
///
/// A provider may wrap an Ethernet MAC, an integrated WiFi driver, or an
/// external coprocessor that exposes packet mode. Native ESP-IDF/lwIP
/// integrations and external SPI/I2C/UART module adapters bind through the
/// same operations. Modules that expose only socket offload bind at UPROTO and
/// do not pretend to provide raw packets here.

#ifndef UETD_UPAL_NETIF_H
#define UETD_UPAL_NETIF_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPAL_NETIF_ADDRESS_MAX 8u

typedef enum {
    UPAL_NETIF_MEDIA_ETHERNET = 0,
    UPAL_NETIF_MEDIA_WIFI,
    UPAL_NETIF_MEDIA_IEEE802154,
    UPAL_NETIF_MEDIA_PPP,
    UPAL_NETIF_MEDIA_OTHER,
} upal_netif_media_t;

enum {
    UPAL_NETIF_CAP_RAW_L2 = 1u << 0,
    UPAL_NETIF_CAP_RAW_IP = 1u << 1,
    UPAL_NETIF_CAP_TX_DMA = 1u << 2,
    UPAL_NETIF_CAP_RX_DMA = 1u << 3,
    UPAL_NETIF_CAP_HARDWARE_CHECKSUM = 1u << 4,
    UPAL_NETIF_CAP_HARDWARE_TIMESTAMP = 1u << 5,
};

typedef struct {
    upal_netif_media_t media;
    uetd_u32_t capabilities;
    uetd_u16_t mtu;
    uetd_u16_t maximum_tx_bytes;
    uetd_u16_t maximum_rx_bytes;
    uetd_u8_t address_length;
    uetd_u8_t hardware_address[UPAL_NETIF_ADDRESS_MAX];
} upal_netif_info_t;

typedef struct {
    bool running;
    bool link_up;
    uetd_u32_t link_speed_bps;
    bool full_duplex;
    uetd_u32_t rx_dropped;
    uetd_u32_t tx_dropped;
} upal_netif_link_t;

typedef struct {
    uetd_status_t (*start)(void* context);
    uetd_status_t (*stop)(void* context);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
    uetd_status_t (*get_link)(void* context, upal_netif_link_t* link);
    uetd_status_t (*transmit)(void* context, const uetd_u8_t* packet, uetd_u16_t length);
    uetd_status_t (*receive)(void* context, uetd_u8_t* packet, uetd_u16_t capacity, uetd_u16_t* length);
} upal_netif_ops_t;

typedef struct {
    upal_netif_info_t info;
    const upal_netif_ops_t* ops;
    void* context;
    bool initialized;
} upal_netif_t;

/// Bind a packet-mode network interface to its target or coprocessor provider.
uetd_status_t upal_netif_init(upal_netif_t* netif, const upal_netif_info_t* info, const upal_netif_ops_t* ops, void* context);
uetd_status_t upal_netif_start(upal_netif_t* netif);
uetd_status_t upal_netif_stop(upal_netif_t* netif);
uetd_status_t upal_netif_poll(upal_netif_t* netif, uetd_u16_t work_budget);
uetd_status_t upal_netif_get_link(upal_netif_t* netif, upal_netif_link_t* link);
uetd_status_t upal_netif_transmit(upal_netif_t* netif, const uetd_u8_t* packet, uetd_u16_t length);
/// UETD_NOT_READY means no packet was available; length is written on success.
uetd_status_t upal_netif_receive(upal_netif_t* netif, uetd_u8_t* packet, uetd_u16_t capacity, uetd_u16_t* length);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPAL_NETIF_H */
