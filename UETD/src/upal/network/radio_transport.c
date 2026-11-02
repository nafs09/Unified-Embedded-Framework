/// @file src/upal/network/radio_transport.c
/// @brief Validated dispatch for an external radio module host transport.
#include "upal/network/radio_transport.h"

static bool upal_radio_bus_valid(upal_radio_bus_t bus) {
    return bus == UPAL_RADIO_BUS_SPI || bus == UPAL_RADIO_BUS_I2C || bus == UPAL_RADIO_BUS_UART;
}

uetd_status_t upal_radio_transport_init(upal_radio_transport_t* transport, const upal_radio_transport_config_t* config) {
    if (transport == NULL || config == NULL || !upal_radio_bus_valid(config->bus) ||
        (config->exchange == NULL && (config->send == NULL || config->receive == NULL)) ||
        config->maximum_tx_bytes == 0u || config->maximum_rx_bytes == 0u) {
        return UETD_INVALID_ARG;
    }

    transport->config = *config;
    transport->initialized = true;
    return UETD_OK;
}

uetd_status_t upal_radio_transport_exchange(upal_radio_transport_t* transport, const uetd_u8_t* tx, uetd_u16_t tx_length,
                                            uetd_u8_t* rx, uetd_u16_t rx_capacity, uetd_u16_t* rx_length,
                                            uetd_u32_t timeout_ms) {
    uetd_status_t status;
    uetd_u16_t actual_length = 0u;

    if (transport == NULL || !transport->initialized || rx_length == NULL || timeout_ms == 0u ||
        (tx_length != 0u && tx == NULL) || (rx_capacity != 0u && rx == NULL)) {
        return UETD_INVALID_ARG;
    }
    if (transport->config.exchange == NULL) return UETD_NOT_SUPPORTED;
    if (tx_length > transport->config.maximum_tx_bytes || rx_capacity > transport->config.maximum_rx_bytes) {
        return UETD_OVERFLOW;
    }

    status = transport->config.exchange(transport->config.context, tx, tx_length, rx, rx_capacity, &actual_length, timeout_ms);
    if (status != UETD_OK) {
        return status;
    }
    if (actual_length > rx_capacity) {
        return UETD_OVERFLOW;
    }
    *rx_length = actual_length;
    return UETD_OK;
}

uetd_status_t upal_radio_transport_send(upal_radio_transport_t* transport, const uetd_u8_t* bytes, uetd_u16_t length,
                                        uetd_u16_t* sent) {
    uetd_u16_t actual_length = 0u;
    uetd_status_t status;
    if (transport == NULL || !transport->initialized || bytes == NULL || length == 0u || sent == NULL)
        return UETD_INVALID_ARG;
    if (length > transport->config.maximum_tx_bytes) return UETD_OVERFLOW;
    if (transport->config.send == NULL) return UETD_NOT_SUPPORTED;
    status = transport->config.send(transport->config.context, bytes, length, &actual_length);
    if (status != UETD_OK) return status;
    if (actual_length > length) return UETD_OVERFLOW;
    *sent = actual_length;
    return UETD_OK;
}

uetd_status_t upal_radio_transport_receive(upal_radio_transport_t* transport, uetd_u8_t* bytes, uetd_u16_t capacity,
                                           uetd_u16_t* received, uetd_u32_t timeout_ms) {
    uetd_u16_t actual_length = 0u;
    uetd_status_t status;
    if (transport == NULL || !transport->initialized || bytes == NULL || capacity == 0u || received == NULL || timeout_ms == 0u)
        return UETD_INVALID_ARG;
    if (capacity > transport->config.maximum_rx_bytes) return UETD_OVERFLOW;
    if (transport->config.receive == NULL) return UETD_NOT_SUPPORTED;
    status = transport->config.receive(transport->config.context, bytes, capacity, &actual_length, timeout_ms);
    if (status != UETD_OK) return status;
    if (actual_length > capacity) return UETD_OVERFLOW;
    if (actual_length == 0u) return UETD_NOT_READY;
    *received = actual_length;
    return UETD_OK;
}

uetd_status_t upal_radio_transport_poll(upal_radio_transport_t* transport, uetd_u16_t work_budget) {
    if (transport == NULL || !transport->initialized || work_budget == 0u) return UETD_INVALID_ARG;
    if (transport->config.poll == NULL) return UETD_NOT_SUPPORTED;
    return transport->config.poll(transport->config.context, work_budget);
}

uetd_status_t upal_radio_transport_set_reset(upal_radio_transport_t* transport, bool asserted) {
    if (transport == NULL || !transport->initialized) {
        return UETD_INVALID_ARG;
    }
    if (transport->config.set_reset == NULL) {
        return UETD_NOT_SUPPORTED;
    }
    return transport->config.set_reset(transport->config.context, asserted);
}

uetd_status_t upal_radio_transport_is_ready(upal_radio_transport_t* transport, bool* ready) {
    if (transport == NULL || !transport->initialized || ready == NULL) {
        return UETD_INVALID_ARG;
    }
    if (transport->config.ready == NULL) {
        return UETD_NOT_SUPPORTED;
    }
    return transport->config.ready(transport->config.context, ready);
}
