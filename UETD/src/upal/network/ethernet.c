/// @file src/upal/network/ethernet.c
/// @brief Validated MAC/PHY service dispatch over UPAL netif.
#include "upal/network/ethernet.h"

uetd_status_t upal_ethernet_bind(upal_ethernet_t* ethernet, upal_netif_t* netif, const upal_ethernet_ops_t* ops, void* context) {
    if (ethernet == NULL || netif == NULL || !netif->initialized || netif->info.media != UPAL_NETIF_MEDIA_ETHERNET ||
        (netif->info.capabilities & UPAL_NETIF_CAP_RAW_L2) == 0u ||
        ops == NULL || ops->read_status == NULL) return UETD_INVALID_ARG;
    ethernet->netif = netif;
    ethernet->ops = ops;
    ethernet->context = context;
    ethernet->bound = true;
    return UETD_OK;
}

uetd_status_t upal_ethernet_get_status(upal_ethernet_t* ethernet, upal_ethernet_status_t* status) {
    if (ethernet == NULL || !ethernet->bound || status == NULL) return UETD_INVALID_ARG;
    return ethernet->ops->read_status(ethernet->context, status);
}

uetd_status_t upal_ethernet_start(upal_ethernet_t* ethernet) {
    if (ethernet == NULL || !ethernet->bound) return UETD_INVALID_ARG;
    return upal_netif_start(ethernet->netif);
}

uetd_status_t upal_ethernet_stop(upal_ethernet_t* ethernet) {
    if (ethernet == NULL || !ethernet->bound) return UETD_INVALID_ARG;
    return upal_netif_stop(ethernet->netif);
}

uetd_status_t upal_ethernet_poll(upal_ethernet_t* ethernet, uetd_u16_t work_budget) {
    if (ethernet == NULL || !ethernet->bound) return UETD_INVALID_ARG;
    return upal_netif_poll(ethernet->netif, work_budget);
}
