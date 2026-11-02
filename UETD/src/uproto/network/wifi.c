/// @file src/uproto/network/wifi.c
/// @brief Validated dispatch for WiFi stack and coprocessor providers.
#include "uproto/network/wifi.h"

static bool uproto_wifi_security_valid(uproto_wifi_security_t security) {
    return security >= UPROTO_WIFI_SECURITY_OPEN && security <= UPROTO_WIFI_SECURITY_WPA2_WPA3;
}

static bool uproto_wifi_passphrase_valid(uproto_wifi_security_t security, uetd_u8_t length) {
    return security == UPROTO_WIFI_SECURITY_OPEN ? length == 0u : length >= 8u && length <= UPROTO_WIFI_PASSPHRASE_MAX;
}

uetd_status_t uproto_wifi_bind(uproto_wifi_t* wifi, const uproto_wifi_ops_t* ops, void* context, uetd_u32_t capabilities) {
    const uetd_u32_t known = UPROTO_WIFI_CAP_STATION | UPROTO_WIFI_CAP_ACCESS_POINT | UPROTO_WIFI_CAP_SCAN |
                             UPROTO_WIFI_CAP_IPV6 | UPROTO_WIFI_CAP_SOCKET_OFFLOAD;
    if (wifi == NULL || ops == NULL || ops->get_link == NULL || capabilities == 0u || (capabilities & ~known) != 0u) {
        return UETD_INVALID_ARG;
    }
    wifi->ops = ops;
    wifi->context = context;
    wifi->capabilities = capabilities;
    wifi->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_wifi_scan(uproto_wifi_t* wifi, uproto_wifi_scan_result_t* results, uetd_u8_t capacity, uetd_u8_t* count) {
    uetd_u8_t i;
    uetd_u8_t actual_count = 0u;
    uetd_status_t status;
    if (wifi == NULL || !wifi->bound || count == NULL || (capacity != 0u && results == NULL) || capacity > UPROTO_WIFI_SCAN_RESULT_MAX) {
        return UETD_INVALID_ARG;
    }
    if ((wifi->capabilities & UPROTO_WIFI_CAP_SCAN) == 0u || wifi->ops->scan == NULL) return UETD_NOT_SUPPORTED;
    status = wifi->ops->scan(wifi->context, results, capacity, &actual_count);
    if (status != UETD_OK) return status;
    if (actual_count > capacity) return UETD_OVERFLOW;
    for (i = 0u; i < actual_count; ++i) {
        if (results[i].ssid_length > UPROTO_WIFI_SSID_MAX || !uproto_wifi_security_valid(results[i].security)) return UETD_ERROR;
    }
    *count = actual_count;
    return UETD_OK;
}

uetd_status_t uproto_wifi_connect(uproto_wifi_t* wifi, const uproto_wifi_station_config_t* config) {
    if (wifi == NULL || !wifi->bound || config == NULL || config->ssid_length == 0u || config->ssid_length > UPROTO_WIFI_SSID_MAX ||
        config->passphrase_length > UPROTO_WIFI_PASSPHRASE_MAX || !uproto_wifi_security_valid(config->security) ||
        !uproto_wifi_passphrase_valid(config->security, config->passphrase_length)) return UETD_INVALID_ARG;
    if (config->ipv6_requested && (wifi->capabilities & UPROTO_WIFI_CAP_IPV6) == 0u) return UETD_NOT_SUPPORTED;
    if ((wifi->capabilities & UPROTO_WIFI_CAP_STATION) == 0u || wifi->ops->connect == NULL) return UETD_NOT_SUPPORTED;
    return wifi->ops->connect(wifi->context, config);
}

uetd_status_t uproto_wifi_disconnect(uproto_wifi_t* wifi) {
    if (wifi == NULL || !wifi->bound) return UETD_INVALID_ARG;
    if ((wifi->capabilities & UPROTO_WIFI_CAP_STATION) == 0u || wifi->ops->disconnect == NULL) return UETD_NOT_SUPPORTED;
    return wifi->ops->disconnect(wifi->context);
}

uetd_status_t uproto_wifi_start_ap(uproto_wifi_t* wifi, const uproto_wifi_ap_config_t* config) {
    if (wifi == NULL || !wifi->bound || config == NULL || config->ssid_length == 0u || config->ssid_length > UPROTO_WIFI_SSID_MAX ||
        config->passphrase_length > UPROTO_WIFI_PASSPHRASE_MAX || !uproto_wifi_security_valid(config->security) ||
        !uproto_wifi_passphrase_valid(config->security, config->passphrase_length) || config->maximum_clients == 0u) {
        return UETD_INVALID_ARG;
    }
    if ((wifi->capabilities & UPROTO_WIFI_CAP_ACCESS_POINT) == 0u || wifi->ops->start_ap == NULL) return UETD_NOT_SUPPORTED;
    return wifi->ops->start_ap(wifi->context, config);
}

uetd_status_t uproto_wifi_stop_ap(uproto_wifi_t* wifi) {
    if (wifi == NULL || !wifi->bound) return UETD_INVALID_ARG;
    if ((wifi->capabilities & UPROTO_WIFI_CAP_ACCESS_POINT) == 0u || wifi->ops->stop_ap == NULL) return UETD_NOT_SUPPORTED;
    return wifi->ops->stop_ap(wifi->context);
}

uetd_status_t uproto_wifi_get_link(uproto_wifi_t* wifi, uproto_wifi_link_t* link) {
    uproto_wifi_link_t actual;
    uetd_status_t status;
    if (wifi == NULL || !wifi->bound || link == NULL) return UETD_INVALID_ARG;
    status = wifi->ops->get_link(wifi->context, &actual);
    if (status != UETD_OK) return status;
    if (actual.state < UPROTO_WIFI_STATE_STOPPED || actual.state > UPROTO_WIFI_STATE_ERROR ||
        (actual.ipv6_available && (wifi->capabilities & UPROTO_WIFI_CAP_IPV6) == 0u)) return UETD_ERROR;
    *link = actual;
    return UETD_OK;
}

uetd_status_t uproto_wifi_poll(uproto_wifi_t* wifi, uetd_u16_t work_budget) {
    if (wifi == NULL || !wifi->bound || work_budget == 0u) return UETD_INVALID_ARG;
    if (wifi->ops->poll == NULL) return UETD_NOT_SUPPORTED;
    return wifi->ops->poll(wifi->context, work_budget);
}
