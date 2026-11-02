/// @file include/uproto/network/bluetooth.h
/// @brief Bluetooth LE and selected Classic profile service boundary.
///
/// The UETD-owned API and capability validation are transport-neutral. A
/// provider may be an external HCI/AT module reached through UPAL
/// SPI/I2C/UART or a native SoC SDK. Controller, RF, coexistence, and any
/// provider-specific host implementation stay at that target boundary; this
/// interface does not claim a complete UETD-owned Bluetooth stack.
/// The service/characteristic description and callback context must remain
/// valid for the registered service lifetime unless the provider copies them.

#ifndef UETD_UPROTO_BLUETOOTH_H
#define UETD_UPROTO_BLUETOOTH_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPROTO_BT_ADDRESS_BYTES 6u
#define UPROTO_BT_ADV_DATA_MAX 255u
#define UPROTO_BT_LEGACY_ADV_DATA_MAX 31u
#define UPROTO_BT_GATT_VALUE_MAX 512u
#define UPROTO_BT_EVENT_DATA_MAX UPROTO_BT_GATT_VALUE_MAX

enum {
    UPROTO_BT_CAP_BLE_CENTRAL = 1u << 0,
    UPROTO_BT_CAP_BLE_PERIPHERAL = 1u << 1,
    UPROTO_BT_CAP_GATT_CLIENT = 1u << 2,
    UPROTO_BT_CAP_GATT_SERVER = 1u << 3,
    UPROTO_BT_CAP_SECURITY = 1u << 4,
    UPROTO_BT_CAP_BONDED_SECURITY = 1u << 5,
    UPROTO_BT_CAP_CLASSIC_SPP = 1u << 6,
    UPROTO_BT_CAP_EXTENDED_ADVERTISING = 1u << 7,
};

typedef enum {
    UPROTO_BT_ADDRESS_PUBLIC = 0,
    UPROTO_BT_ADDRESS_RANDOM,
} uproto_bt_address_kind_t;

typedef struct {
    uetd_u8_t bytes[UPROTO_BT_ADDRESS_BYTES];
    uproto_bt_address_kind_t kind;
} uproto_bt_address_t;

typedef struct {
    uetd_u16_t interval_min_1p25ms;
    uetd_u16_t interval_max_1p25ms;
    uetd_u8_t channel_map;
    bool connectable;
    bool scannable;
    bool extended;
    const uetd_u8_t* data;
    uetd_u16_t data_length;
} uproto_bt_advertising_t;

typedef enum {
    UPROTO_BT_SECURITY_NONE = 0,
    UPROTO_BT_SECURITY_ENCRYPTED,
    UPROTO_BT_SECURITY_AUTHENTICATED,
    UPROTO_BT_SECURITY_SECURE_CONNECTIONS,
} uproto_bt_security_t;

typedef struct {
    uetd_u8_t length; /* 2 or 16 bytes */
    uetd_u8_t bytes[16];
} uproto_bt_uuid_t;

typedef enum {
    UPROTO_BT_EVENT_SCAN_RESULT = 0,
    UPROTO_BT_EVENT_CONNECTED,
    UPROTO_BT_EVENT_DISCONNECTED,
    UPROTO_BT_EVENT_GATT_SERVICE,
    UPROTO_BT_EVENT_GATT_CHARACTERISTIC,
    UPROTO_BT_EVENT_GATT_DESCRIPTOR,
    UPROTO_BT_EVENT_GATT_READ_REQUEST,
    UPROTO_BT_EVENT_GATT_WRITE_REQUEST,
    UPROTO_BT_EVENT_GATT_NOTIFICATION,
    UPROTO_BT_EVENT_SPP_DATA,
    UPROTO_BT_EVENT_SECURITY,
    /* Compatibility spellings for the former server request event names. */
    UPROTO_BT_EVENT_GATT_READ = UPROTO_BT_EVENT_GATT_READ_REQUEST,
    UPROTO_BT_EVENT_GATT_WRITE = UPROTO_BT_EVENT_GATT_WRITE_REQUEST,
} uproto_bt_event_kind_t;

typedef struct {
    uproto_bt_event_kind_t kind;
    uproto_bt_address_t peer;
    uetd_i8_t rssi_dbm; /* meaningful for SCAN_RESULT */
    uetd_u16_t handle; /* service start, characteristic value, descriptor, or attribute handle */
    uetd_u16_t end_handle; /* service/characteristic range when discovered */
    uetd_u16_t parent_handle; /* containing service or characteristic when discovered */
    uetd_u32_t transaction_id; /* nonzero for server read/write requests */
    uproto_bt_uuid_t uuid; /* meaningful for GATT discovery events */
    uetd_u16_t maximum_value_length; /* meaningful for characteristic discovery */
    uetd_u8_t properties; /* UPROTO_BT_GATT_PROP_* for characteristic discovery */
    uproto_bt_security_t minimum_security; /* meaningful for characteristic discovery */
    bool primary; /* meaningful for service discovery */
    uetd_u16_t data_length;
    uetd_u8_t data[UPROTO_BT_EVENT_DATA_MAX]; /* inline advertisement, GATT value, or SPP chunk */
    uetd_u8_t status;
} uproto_bt_event_t;

enum {
    UPROTO_BT_GATT_PROP_READ = 1u << 0,
    UPROTO_BT_GATT_PROP_WRITE = 1u << 1,
    UPROTO_BT_GATT_PROP_WRITE_NO_RESPONSE = 1u << 2,
    UPROTO_BT_GATT_PROP_NOTIFY = 1u << 3,
    UPROTO_BT_GATT_PROP_INDICATE = 1u << 4,
};

typedef struct {
    uproto_bt_uuid_t uuid;
    uetd_u16_t maximum_value_length;
    uetd_u8_t properties;
    uproto_bt_security_t minimum_security;
} uproto_bt_gatt_characteristic_t;

typedef struct {
    uproto_bt_uuid_t uuid;
    const uproto_bt_gatt_characteristic_t* characteristics;
    uetd_u8_t characteristic_count;
    bool primary;
} uproto_bt_gatt_service_t;

typedef struct {
    uetd_status_t (*scan_start)(void* context, uetd_u16_t duration_ms, bool active_scan);
    uetd_status_t (*scan_stop)(void* context);
    uetd_status_t (*advertise_start)(void* context, const uproto_bt_advertising_t* advertising);
    uetd_status_t (*advertise_stop)(void* context);
    uetd_status_t (*connect)(void* context, const uproto_bt_address_t* peer, uetd_u16_t timeout_ms);
    uetd_status_t (*disconnect)(void* context, const uproto_bt_address_t* peer);
    /* Discovery is asynchronous; matching results arrive through next_event. A NULL UUID filter means all. */
    uetd_status_t (*gatt_discover_services)(void* context, const uproto_bt_address_t* peer,
                                            const uproto_bt_uuid_t* uuid_filter);
    uetd_status_t (*gatt_discover_characteristics)(void* context, const uproto_bt_address_t* peer,
                                                   uetd_u16_t service_start_handle, uetd_u16_t service_end_handle,
                                                   const uproto_bt_uuid_t* uuid_filter);
    uetd_status_t (*gatt_discover_descriptors)(void* context, const uproto_bt_address_t* peer,
                                               uetd_u16_t characteristic_value_handle, uetd_u16_t end_handle);
    uetd_status_t (*gatt_read)(void* context, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                               uetd_u8_t* value, uetd_u16_t capacity, uetd_u16_t* length);
    uetd_status_t (*gatt_write)(void* context, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                                const uetd_u8_t* value, uetd_u16_t length, bool with_response);
    uetd_status_t (*gatt_notify)(void* context, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                                 const uetd_u8_t* value, uetd_u16_t length, bool indicate);
    uetd_status_t (*gatt_subscribe)(void* context, const uproto_bt_address_t* peer,
                                    uetd_u16_t characteristic_value_handle, bool enabled, bool indicate);
    uetd_status_t (*gatt_server_respond)(void* context, const uproto_bt_address_t* peer,
                                         uetd_u32_t transaction_id, uetd_status_t result,
                                         const uetd_u8_t* value, uetd_u16_t length);
    uetd_status_t (*gatt_register_service)(void* context, const uproto_bt_gatt_service_t* service,
                                           uetd_u16_t* service_handle,
                                           uetd_u16_t* characteristic_value_handles,
                                           uetd_u8_t handle_capacity, uetd_u8_t* handle_count);
    uetd_status_t (*gatt_unregister_service)(void* context, uetd_u16_t service_handle);
    uetd_status_t (*set_security)(void* context, uproto_bt_security_t security, bool require_bonding);
    uetd_status_t (*pair)(void* context, const uproto_bt_address_t* peer, uetd_u32_t passkey);
    uetd_status_t (*remove_bond)(void* context, const uproto_bt_address_t* peer);
    uetd_status_t (*spp_open)(void* context, const uproto_bt_address_t* peer, uetd_u16_t timeout_ms);
    uetd_status_t (*spp_write)(void* context, const uetd_u8_t* bytes, uetd_u16_t length, uetd_u16_t* written);
    uetd_status_t (*spp_close)(void* context);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
    uetd_status_t (*next_event)(void* context, uproto_bt_event_t* event);
} uproto_bt_ops_t;

typedef struct {
    const uproto_bt_ops_t* ops;
    void* context;
    uetd_u32_t capabilities;
    bool bound;
} uproto_bt_t;

uetd_status_t uproto_bt_bind(uproto_bt_t* bluetooth, const uproto_bt_ops_t* ops, void* context, uetd_u32_t capabilities);
uetd_status_t uproto_bt_scan_start(uproto_bt_t* bluetooth, uetd_u16_t duration_ms, bool active_scan);
uetd_status_t uproto_bt_scan_stop(uproto_bt_t* bluetooth);
uetd_status_t uproto_bt_advertise_start(uproto_bt_t* bluetooth, const uproto_bt_advertising_t* advertising);
uetd_status_t uproto_bt_advertise_stop(uproto_bt_t* bluetooth);
uetd_status_t uproto_bt_connect(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t timeout_ms);
uetd_status_t uproto_bt_disconnect(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer);
uetd_status_t uproto_bt_gatt_discover_services(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                               const uproto_bt_uuid_t* uuid_filter);
uetd_status_t uproto_bt_gatt_discover_characteristics(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                                      uetd_u16_t service_start_handle, uetd_u16_t service_end_handle,
                                                      const uproto_bt_uuid_t* uuid_filter);
uetd_status_t uproto_bt_gatt_discover_descriptors(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                                  uetd_u16_t characteristic_value_handle, uetd_u16_t end_handle);
uetd_status_t uproto_bt_gatt_read(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                                  uetd_u8_t* value, uetd_u16_t capacity, uetd_u16_t* length);
uetd_status_t uproto_bt_gatt_write(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                                   const uetd_u8_t* value, uetd_u16_t length, bool with_response);
uetd_status_t uproto_bt_gatt_notify(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                                    const uetd_u8_t* value, uetd_u16_t length, bool indicate);
uetd_status_t uproto_bt_gatt_subscribe(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                       uetd_u16_t characteristic_value_handle, bool enabled, bool indicate);
uetd_status_t uproto_bt_gatt_server_respond(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                            uetd_u32_t transaction_id, uetd_status_t result,
                                            const uetd_u8_t* value, uetd_u16_t length);
uetd_status_t uproto_bt_gatt_register_service(uproto_bt_t* bluetooth, const uproto_bt_gatt_service_t* service,
                                              uetd_u16_t* service_handle,
                                              uetd_u16_t* characteristic_value_handles,
                                              uetd_u8_t handle_capacity);
uetd_status_t uproto_bt_gatt_unregister_service(uproto_bt_t* bluetooth, uetd_u16_t service_handle);
uetd_status_t uproto_bt_set_security(uproto_bt_t* bluetooth, uproto_bt_security_t security, bool require_bonding);
uetd_status_t uproto_bt_pair(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u32_t passkey);
uetd_status_t uproto_bt_remove_bond(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer);
uetd_status_t uproto_bt_spp_open(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t timeout_ms);
uetd_status_t uproto_bt_spp_write(uproto_bt_t* bluetooth, const uetd_u8_t* bytes, uetd_u16_t length, uetd_u16_t* written);
uetd_status_t uproto_bt_spp_close(uproto_bt_t* bluetooth);
uetd_status_t uproto_bt_poll(uproto_bt_t* bluetooth, uetd_u16_t work_budget);
uetd_status_t uproto_bt_next_event(uproto_bt_t* bluetooth, uproto_bt_event_t* event);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_BLUETOOTH_H */
