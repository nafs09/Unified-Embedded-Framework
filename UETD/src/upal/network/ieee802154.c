/// @file src/upal/network/ieee802154.c
/// @brief Validated dispatch for an IEEE 802.15.4 radio/MAC provider.
#include "upal/network/ieee802154.h"

static bool upal_ieee802154_address_valid(const upal_ieee802154_address_t* address) {
    return address != NULL && address->mode >= UPAL_IEEE802154_ADDRESS_SHORT &&
           address->mode <= UPAL_IEEE802154_ADDRESS_EXTENDED;
}

uetd_status_t upal_ieee802154_init(upal_ieee802154_t* radio, const upal_ieee802154_ops_t* ops, void* context) {
    if (radio == NULL || ops == NULL || ops->start == NULL || ops->stop == NULL || ops->set_channel == NULL ||
        ops->transmit == NULL || ops->receive == NULL || ops->get_status == NULL || ops->poll == NULL) return UETD_INVALID_ARG;
    radio->ops = ops;
    radio->context = context;
    radio->initialized = true;
    return UETD_OK;
}

uetd_status_t upal_ieee802154_start(upal_ieee802154_t* radio, const upal_ieee802154_config_t* config) {
    if (radio == NULL || !radio->initialized || config == NULL || config->channel < 11u || config->channel > 26u ||
        !upal_ieee802154_address_valid(&config->local_address)) return UETD_INVALID_ARG;
    return radio->ops->start(radio->context, config);
}

uetd_status_t upal_ieee802154_stop(upal_ieee802154_t* radio) {
    if (radio == NULL || !radio->initialized) return UETD_INVALID_ARG;
    return radio->ops->stop(radio->context);
}

uetd_status_t upal_ieee802154_set_channel(upal_ieee802154_t* radio, uetd_u8_t channel) {
    if (radio == NULL || !radio->initialized || channel < 11u || channel > 26u) return UETD_INVALID_ARG;
    return radio->ops->set_channel(radio->context, channel);
}

uetd_status_t upal_ieee802154_transmit(upal_ieee802154_t* radio, const upal_ieee802154_frame_t* frame) {
    if (radio == NULL || !radio->initialized || frame == NULL || frame->length == 0u || frame->length > UPAL_IEEE802154_FRAME_MAX)
        return UETD_INVALID_ARG;
    return radio->ops->transmit(radio->context, frame);
}

uetd_status_t upal_ieee802154_receive(upal_ieee802154_t* radio, upal_ieee802154_frame_t* frame) {
    uetd_status_t status;
    if (radio == NULL || !radio->initialized || frame == NULL) return UETD_INVALID_ARG;
    status = radio->ops->receive(radio->context, frame);
    if (status != UETD_OK) return status;
    return frame->length == 0u || frame->length > UPAL_IEEE802154_FRAME_MAX ? UETD_OVERFLOW : UETD_OK;
}

uetd_status_t upal_ieee802154_get_status(upal_ieee802154_t* radio, upal_ieee802154_status_t* status) {
    if (radio == NULL || !radio->initialized || status == NULL) return UETD_INVALID_ARG;
    return radio->ops->get_status(radio->context, status);
}

uetd_status_t upal_ieee802154_poll(upal_ieee802154_t* radio, uetd_u16_t work_budget) {
    if (radio == NULL || !radio->initialized || work_budget == 0u) return UETD_INVALID_ARG;
    return radio->ops->poll(radio->context, work_budget);
}
