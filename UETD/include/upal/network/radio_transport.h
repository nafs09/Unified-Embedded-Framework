/// @file include/upal/network/radio_transport.h
/// @brief Bounded host transport for external radio/network coprocessors.
///
/// The transport provides either a bounded command/response exchange, an
/// asynchronous byte stream, or both. Framing, wake/ready sequencing,
/// HCI-vs-AT semantics, and unsolicited-event parsing belong to the selected
/// WiFi/Bluetooth/802.15.4 provider. Callbacks can use UPAL SPI, I2C, or UART.
/// Integrated radios bind through their target SDK and bypass this host link.

#ifndef UETD_UPAL_RADIO_TRANSPORT_H
#define UETD_UPAL_RADIO_TRANSPORT_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UPAL_RADIO_BUS_SPI = 0,
    UPAL_RADIO_BUS_I2C,
    UPAL_RADIO_BUS_UART,
} upal_radio_bus_t;

/// Exchange one complete, module-defined command/response transaction.
/// Implementations own bus-specific framing and must bound the operation by
/// timeout_ms. tx/rx and rx_length are caller-owned; rx_length is published
/// only on success.
typedef uetd_status_t (*upal_radio_exchange_fn)(void* context, const uetd_u8_t* tx, uetd_u16_t tx_length,
                                                uetd_u8_t* rx, uetd_u16_t rx_capacity, uetd_u16_t* rx_length,
                                                uetd_u32_t timeout_ms);
typedef uetd_status_t (*upal_radio_send_fn)(void* context, const uetd_u8_t* bytes, uetd_u16_t length,
                                            uetd_u16_t* sent);
typedef uetd_status_t (*upal_radio_receive_fn)(void* context, uetd_u8_t* bytes, uetd_u16_t capacity,
                                               uetd_u16_t* received, uetd_u32_t timeout_ms);
typedef uetd_status_t (*upal_radio_poll_fn)(void* context, uetd_u16_t work_budget);
typedef uetd_status_t (*upal_radio_reset_fn)(void* context, bool asserted);
typedef uetd_status_t (*upal_radio_ready_fn)(void* context, bool* ready);

typedef struct {
    upal_radio_bus_t bus;
    upal_radio_exchange_fn exchange;
    upal_radio_send_fn send;
    upal_radio_receive_fn receive;
    upal_radio_poll_fn poll;
    upal_radio_reset_fn set_reset;
    upal_radio_ready_fn ready;
    void* context;
    uetd_u16_t maximum_tx_bytes;
    uetd_u16_t maximum_rx_bytes;
} upal_radio_transport_config_t;

typedef struct {
    upal_radio_transport_config_t config;
    bool initialized;
} upal_radio_transport_t;

/// Validate and bind a physical host transport. No bus registers are touched.
uetd_status_t upal_radio_transport_init(upal_radio_transport_t* transport, const upal_radio_transport_config_t* config);
/// Exchange a bounded command/response transaction over the selected bus.
uetd_status_t upal_radio_transport_exchange(upal_radio_transport_t* transport, const uetd_u8_t* tx, uetd_u16_t tx_length,
                                            uetd_u8_t* rx, uetd_u16_t rx_capacity, uetd_u16_t* rx_length,
                                            uetd_u32_t timeout_ms);
/// Send bytes on an asynchronous HCI/AT/vendor stream; *sent is published on success.
uetd_status_t upal_radio_transport_send(upal_radio_transport_t* transport, const uetd_u8_t* bytes, uetd_u16_t length,
                                        uetd_u16_t* sent);
/// Receive available stream bytes; UETD_NOT_READY means no bytes arrived before timeout_ms.
uetd_status_t upal_radio_transport_receive(upal_radio_transport_t* transport, uetd_u8_t* bytes, uetd_u16_t capacity,
                                           uetd_u16_t* received, uetd_u32_t timeout_ms);
/// Advance a provider's deferred bus/event work with a finite work budget.
uetd_status_t upal_radio_transport_poll(upal_radio_transport_t* transport, uetd_u16_t work_budget);
/// Assert or release the optional module reset line.
uetd_status_t upal_radio_transport_set_reset(upal_radio_transport_t* transport, bool asserted);
/// Read the optional module-ready indication.
uetd_status_t upal_radio_transport_is_ready(upal_radio_transport_t* transport, bool* ready);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPAL_RADIO_TRANSPORT_H */
