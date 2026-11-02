/// @file include/upal/network/ieee802154.h
/// @brief Fixed-frame IEEE 802.15.4 MAC/radio provider contract.
///
/// This boundary covers bounded frame configuration and delivery. A selected
/// Thread/Zigbee stack owns network commissioning, key lifecycle, routing,
/// replay protection, and higher-layer security. Providers may be native radio
/// drivers or external radio modules reached through SPI/I2C/UART.

#ifndef UETD_UPAL_FUTURE_IEEE802154_H
#define UETD_UPAL_FUTURE_IEEE802154_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The provider appends/verifies the two-octet FCS; bytes exclude that FCS. */
#define UPAL_IEEE802154_PSDU_MAX 127u
#define UPAL_IEEE802154_FRAME_MAX 125u
#define UPAL_IEEE802154_EXTENDED_ADDRESS_BYTES 8u

typedef enum {
    UPAL_IEEE802154_ADDRESS_NONE = 0,
    UPAL_IEEE802154_ADDRESS_SHORT,
    UPAL_IEEE802154_ADDRESS_EXTENDED,
} upal_ieee802154_address_mode_t;

typedef struct {
    upal_ieee802154_address_mode_t mode;
    uetd_u16_t short_address;
    uetd_u8_t extended_address[UPAL_IEEE802154_EXTENDED_ADDRESS_BYTES];
} upal_ieee802154_address_t;

typedef struct {
    uetd_u8_t channel; /* 11..26 in the common 2.4 GHz PHY profile */
    uetd_u16_t pan_id;
    upal_ieee802154_address_t local_address;
    uetd_u8_t maximum_retries;
    bool promiscuous;
    bool request_ack;
} upal_ieee802154_config_t;

typedef struct {
    uetd_u8_t bytes[UPAL_IEEE802154_FRAME_MAX];
    uetd_u8_t length;
    uetd_i8_t rssi_dbm;
    uetd_u8_t link_quality;
    uetd_u64_t timestamp_us;
} upal_ieee802154_frame_t;

typedef struct {
    bool running;
    bool channel_busy;
    uetd_u8_t channel;
    uetd_u32_t tx_failures;
    uetd_u32_t rx_dropped;
} upal_ieee802154_status_t;

typedef struct {
    uetd_status_t (*start)(void* context, const upal_ieee802154_config_t* config);
    uetd_status_t (*stop)(void* context);
    uetd_status_t (*set_channel)(void* context, uetd_u8_t channel);
    uetd_status_t (*transmit)(void* context, const upal_ieee802154_frame_t* frame);
    uetd_status_t (*receive)(void* context, upal_ieee802154_frame_t* frame);
    uetd_status_t (*get_status)(void* context, upal_ieee802154_status_t* status);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
} upal_ieee802154_ops_t;

typedef struct {
    const upal_ieee802154_ops_t* ops;
    void* context;
    bool initialized;
} upal_ieee802154_t;

uetd_status_t upal_ieee802154_init(upal_ieee802154_t* radio, const upal_ieee802154_ops_t* ops, void* context);
uetd_status_t upal_ieee802154_start(upal_ieee802154_t* radio, const upal_ieee802154_config_t* config);
uetd_status_t upal_ieee802154_stop(upal_ieee802154_t* radio);
uetd_status_t upal_ieee802154_set_channel(upal_ieee802154_t* radio, uetd_u8_t channel);
uetd_status_t upal_ieee802154_transmit(upal_ieee802154_t* radio, const upal_ieee802154_frame_t* frame);
uetd_status_t upal_ieee802154_receive(upal_ieee802154_t* radio, upal_ieee802154_frame_t* frame);
uetd_status_t upal_ieee802154_get_status(upal_ieee802154_t* radio, upal_ieee802154_status_t* status);
uetd_status_t upal_ieee802154_poll(upal_ieee802154_t* radio, uetd_u16_t work_budget);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPAL_FUTURE_IEEE802154_H */
