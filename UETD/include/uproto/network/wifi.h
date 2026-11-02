/// @file include/uproto/network/wifi.h
/// @brief Typed WiFi manager boundary for external radios and integrated SoC stacks.
///
/// UETD owns the fixed-size configuration, validation, capability, and call
/// contract, not the target's 802.11 radio/MAC or regulatory implementation.
/// A provider may translate these calls into an external
/// module protocol over UPAL SPI/I2C/UART, or call an integrated SDK such as
/// ESP-IDF directly. Credential bytes are borrowed for each call and must not
/// be logged by providers.

#ifndef UETD_UPROTO_WIFI_H
#define UETD_UPROTO_WIFI_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPROTO_WIFI_SSID_MAX 32u
#define UPROTO_WIFI_PASSPHRASE_MAX 64u
#define UPROTO_WIFI_SCAN_RESULT_MAX 16u

enum {
    UPROTO_WIFI_CAP_STATION = 1u << 0,
    UPROTO_WIFI_CAP_ACCESS_POINT = 1u << 1,
    UPROTO_WIFI_CAP_SCAN = 1u << 2,
    UPROTO_WIFI_CAP_IPV6 = 1u << 3,
    UPROTO_WIFI_CAP_SOCKET_OFFLOAD = 1u << 4,
};

typedef enum {
    UPROTO_WIFI_SECURITY_OPEN = 0,
    UPROTO_WIFI_SECURITY_WPA2_PSK,
    UPROTO_WIFI_SECURITY_WPA3_SAE,
    UPROTO_WIFI_SECURITY_WPA2_WPA3,
} uproto_wifi_security_t;

typedef enum {
    UPROTO_WIFI_STATE_STOPPED = 0,
    UPROTO_WIFI_STATE_DISCONNECTED,
    UPROTO_WIFI_STATE_CONNECTING,
    UPROTO_WIFI_STATE_CONNECTED,
    UPROTO_WIFI_STATE_ACCESS_POINT,
    UPROTO_WIFI_STATE_ERROR,
} uproto_wifi_state_t;

typedef struct {
    uetd_u8_t ssid[UPROTO_WIFI_SSID_MAX];
    uetd_u8_t ssid_length;
    uetd_u8_t passphrase[UPROTO_WIFI_PASSPHRASE_MAX];
    uetd_u8_t passphrase_length;
    uproto_wifi_security_t security;
    uetd_u8_t bssid[6];
    bool use_bssid;
    uetd_u8_t channel; /* zero lets the provider choose */
    char country_code[2]; /* ISO 3166-1 alpha-2, enforced by the provider */
    bool ipv6_requested;
} uproto_wifi_station_config_t;

typedef struct {
    uetd_u8_t ssid[UPROTO_WIFI_SSID_MAX];
    uetd_u8_t ssid_length;
    uetd_u8_t passphrase[UPROTO_WIFI_PASSPHRASE_MAX];
    uetd_u8_t passphrase_length;
    uproto_wifi_security_t security;
    uetd_u8_t channel;
    char country_code[2];
    uetd_u8_t maximum_clients;
    bool hidden;
} uproto_wifi_ap_config_t;

typedef struct {
    uetd_u8_t ssid[UPROTO_WIFI_SSID_MAX];
    uetd_u8_t ssid_length;
    uetd_u8_t bssid[6];
    uetd_i8_t rssi_dbm;
    uetd_u8_t channel;
    uproto_wifi_security_t security;
} uproto_wifi_scan_result_t;

typedef struct {
    uproto_wifi_state_t state;
    uetd_u8_t bssid[6];
    uetd_i8_t rssi_dbm;
    uetd_u8_t channel;
    uetd_u8_t reason_code;
    uetd_u32_t ipv4_address; /* network byte order; zero when not assigned */
    bool ipv6_available;
} uproto_wifi_link_t;

typedef struct {
    uetd_status_t (*scan)(void* context, uproto_wifi_scan_result_t* results, uetd_u8_t capacity, uetd_u8_t* count);
    uetd_status_t (*connect)(void* context, const uproto_wifi_station_config_t* config);
    uetd_status_t (*disconnect)(void* context);
    uetd_status_t (*start_ap)(void* context, const uproto_wifi_ap_config_t* config);
    uetd_status_t (*stop_ap)(void* context);
    uetd_status_t (*get_link)(void* context, uproto_wifi_link_t* link);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
} uproto_wifi_ops_t;

typedef struct {
    const uproto_wifi_ops_t* ops;
    void* context;
    uetd_u32_t capabilities;
    bool bound;
} uproto_wifi_t;

/// Bind a provider without allocating memory or selecting a specific bus/SDK.
uetd_status_t uproto_wifi_bind(uproto_wifi_t* wifi, const uproto_wifi_ops_t* ops, void* context, uetd_u32_t capabilities);
uetd_status_t uproto_wifi_scan(uproto_wifi_t* wifi, uproto_wifi_scan_result_t* results, uetd_u8_t capacity, uetd_u8_t* count);
uetd_status_t uproto_wifi_connect(uproto_wifi_t* wifi, const uproto_wifi_station_config_t* config);
uetd_status_t uproto_wifi_disconnect(uproto_wifi_t* wifi);
uetd_status_t uproto_wifi_start_ap(uproto_wifi_t* wifi, const uproto_wifi_ap_config_t* config);
uetd_status_t uproto_wifi_stop_ap(uproto_wifi_t* wifi);
uetd_status_t uproto_wifi_get_link(uproto_wifi_t* wifi, uproto_wifi_link_t* link);
uetd_status_t uproto_wifi_poll(uproto_wifi_t* wifi, uetd_u16_t work_budget);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_WIFI_H */
