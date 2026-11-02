/// @file src/upal/network/netif.c
/// @brief Validated dispatch for packet-mode link providers.
#include "upal/network/netif.h"

static bool upal_netif_media_valid(upal_netif_media_t media) {
    return media >= UPAL_NETIF_MEDIA_ETHERNET && media <= UPAL_NETIF_MEDIA_OTHER;
}

uetd_status_t upal_netif_init(upal_netif_t* netif, const upal_netif_info_t* info, const upal_netif_ops_t* ops, void* context) {
    if (netif == NULL || info == NULL || ops == NULL || !upal_netif_media_valid(info->media) || info->mtu == 0u ||
        info->maximum_tx_bytes == 0u || info->maximum_rx_bytes == 0u || info->address_length > UPAL_NETIF_ADDRESS_MAX ||
        (info->capabilities & ~(UPAL_NETIF_CAP_RAW_L2 | UPAL_NETIF_CAP_RAW_IP | UPAL_NETIF_CAP_TX_DMA |
                                UPAL_NETIF_CAP_RX_DMA | UPAL_NETIF_CAP_HARDWARE_CHECKSUM | UPAL_NETIF_CAP_HARDWARE_TIMESTAMP)) != 0u ||
        (info->capabilities & (UPAL_NETIF_CAP_RAW_L2 | UPAL_NETIF_CAP_RAW_IP)) == 0u ||
        (((info->capabilities & UPAL_NETIF_CAP_RAW_IP) != 0u) &&
         (info->maximum_tx_bytes < info->mtu || info->maximum_rx_bytes < info->mtu)) ||
        ops->start == NULL || ops->stop == NULL || ops->poll == NULL || ops->get_link == NULL || ops->transmit == NULL ||
        ops->receive == NULL) {
        return UETD_INVALID_ARG;
    }

    netif->info = *info;
    netif->ops = ops;
    netif->context = context;
    netif->initialized = true;
    return UETD_OK;
}

uetd_status_t upal_netif_start(upal_netif_t* netif) {
    if (netif == NULL || !netif->initialized) return UETD_INVALID_ARG;
    return netif->ops->start(netif->context);
}

uetd_status_t upal_netif_stop(upal_netif_t* netif) {
    if (netif == NULL || !netif->initialized) return UETD_INVALID_ARG;
    return netif->ops->stop(netif->context);
}

uetd_status_t upal_netif_poll(upal_netif_t* netif, uetd_u16_t work_budget) {
    if (netif == NULL || !netif->initialized || work_budget == 0u) return UETD_INVALID_ARG;
    return netif->ops->poll(netif->context, work_budget);
}

uetd_status_t upal_netif_get_link(upal_netif_t* netif, upal_netif_link_t* link) {
    if (netif == NULL || !netif->initialized || link == NULL) return UETD_INVALID_ARG;
    return netif->ops->get_link(netif->context, link);
}

uetd_status_t upal_netif_transmit(upal_netif_t* netif, const uetd_u8_t* packet, uetd_u16_t length) {
    if (netif == NULL || !netif->initialized || packet == NULL || length == 0u) return UETD_INVALID_ARG;
    if (length > netif->info.maximum_tx_bytes ||
        (((netif->info.capabilities & UPAL_NETIF_CAP_RAW_L2) == 0u) && length > netif->info.mtu)) return UETD_OVERFLOW;
    return netif->ops->transmit(netif->context, packet, length);
}

uetd_status_t upal_netif_receive(upal_netif_t* netif, uetd_u8_t* packet, uetd_u16_t capacity, uetd_u16_t* length) {
    uetd_status_t status;
    uetd_u16_t actual_length = 0u;

    if (netif == NULL || !netif->initialized || packet == NULL || capacity == 0u || length == NULL) return UETD_INVALID_ARG;
    if (capacity > netif->info.maximum_rx_bytes) return UETD_OVERFLOW;
    status = netif->ops->receive(netif->context, packet, capacity, &actual_length);
    if (status != UETD_OK) return status;
    if (actual_length == 0u) return UETD_NOT_READY;
    if (actual_length > capacity ||
        (((netif->info.capabilities & UPAL_NETIF_CAP_RAW_L2) == 0u) && actual_length > netif->info.mtu)) return UETD_OVERFLOW;
    *length = actual_length;
    return UETD_OK;
}
