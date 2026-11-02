/// @file src/uproto/network/bluetooth.c
/// @brief Validated dispatch for Bluetooth host-stack providers.
#include "uproto/network/bluetooth.h"

static bool uproto_bt_address_valid(const uproto_bt_address_t* peer) {
    return peer != NULL && (peer->kind == UPROTO_BT_ADDRESS_PUBLIC || peer->kind == UPROTO_BT_ADDRESS_RANDOM);
}

static bool uproto_bt_uuid_valid(const uproto_bt_uuid_t* uuid) {
    return uuid != NULL && (uuid->length == 2u || uuid->length == 16u);
}

uetd_status_t uproto_bt_bind(uproto_bt_t* bluetooth, const uproto_bt_ops_t* ops, void* context, uetd_u32_t capabilities) {
    const uetd_u32_t known = UPROTO_BT_CAP_BLE_CENTRAL | UPROTO_BT_CAP_BLE_PERIPHERAL | UPROTO_BT_CAP_GATT_CLIENT |
                             UPROTO_BT_CAP_GATT_SERVER | UPROTO_BT_CAP_SECURITY | UPROTO_BT_CAP_BONDED_SECURITY | UPROTO_BT_CAP_CLASSIC_SPP |
                             UPROTO_BT_CAP_EXTENDED_ADVERTISING;
    if (bluetooth == NULL || ops == NULL || capabilities == 0u || (capabilities & ~known) != 0u ||
        ((capabilities & UPROTO_BT_CAP_GATT_CLIENT) != 0u && (capabilities & UPROTO_BT_CAP_BLE_CENTRAL) == 0u) ||
        ((capabilities & UPROTO_BT_CAP_GATT_SERVER) != 0u && (capabilities & UPROTO_BT_CAP_BLE_PERIPHERAL) == 0u) ||
        ((capabilities & UPROTO_BT_CAP_BONDED_SECURITY) != 0u && (capabilities & UPROTO_BT_CAP_SECURITY) == 0u) ||
        ((capabilities & UPROTO_BT_CAP_EXTENDED_ADVERTISING) != 0u && (capabilities & UPROTO_BT_CAP_BLE_PERIPHERAL) == 0u))
        return UETD_INVALID_ARG;
    bluetooth->ops = ops;
    bluetooth->context = context;
    bluetooth->capabilities = capabilities;
    bluetooth->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_bt_scan_start(uproto_bt_t* bluetooth, uetd_u16_t duration_ms, bool active_scan) {
    if (bluetooth == NULL || !bluetooth->bound || duration_ms == 0u) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_BLE_CENTRAL) == 0u || bluetooth->ops->scan_start == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->scan_start(bluetooth->context, duration_ms, active_scan);
}

uetd_status_t uproto_bt_scan_stop(uproto_bt_t* bluetooth) {
    if (bluetooth == NULL || !bluetooth->bound) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_BLE_CENTRAL) == 0u) return UETD_NOT_SUPPORTED;
    if (bluetooth->ops->scan_stop == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->scan_stop(bluetooth->context);
}

uetd_status_t uproto_bt_advertise_start(uproto_bt_t* bluetooth, const uproto_bt_advertising_t* advertising) {
    if (bluetooth == NULL || !bluetooth->bound || advertising == NULL || advertising->interval_min_1p25ms == 0u ||
        advertising->interval_max_1p25ms < advertising->interval_min_1p25ms || advertising->channel_map == 0u ||
        (advertising->channel_map & 0xF8u) != 0u || advertising->data_length > UPROTO_BT_ADV_DATA_MAX ||
        (!advertising->extended && advertising->data_length > UPROTO_BT_LEGACY_ADV_DATA_MAX) ||
        (advertising->data_length != 0u && advertising->data == NULL)) {
        return UETD_INVALID_ARG;
    }
    if (advertising->extended && (bluetooth->capabilities & UPROTO_BT_CAP_EXTENDED_ADVERTISING) == 0u)
        return UETD_NOT_SUPPORTED;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_BLE_PERIPHERAL) == 0u || bluetooth->ops->advertise_start == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->advertise_start(bluetooth->context, advertising);
}

uetd_status_t uproto_bt_advertise_stop(uproto_bt_t* bluetooth) {
    if (bluetooth == NULL || !bluetooth->bound) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_BLE_PERIPHERAL) == 0u) return UETD_NOT_SUPPORTED;
    if (bluetooth->ops->advertise_stop == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->advertise_stop(bluetooth->context);
}

uetd_status_t uproto_bt_connect(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t timeout_ms) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) || timeout_ms == 0u) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_BLE_CENTRAL) == 0u || bluetooth->ops->connect == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->connect(bluetooth->context, peer, timeout_ms);
}

uetd_status_t uproto_bt_disconnect(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer)) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & (UPROTO_BT_CAP_BLE_CENTRAL | UPROTO_BT_CAP_BLE_PERIPHERAL |
                                    UPROTO_BT_CAP_CLASSIC_SPP)) == 0u) return UETD_NOT_SUPPORTED;
    if (bluetooth->ops->disconnect == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->disconnect(bluetooth->context, peer);
}

uetd_status_t uproto_bt_gatt_discover_services(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                               const uproto_bt_uuid_t* uuid_filter) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) ||
        (uuid_filter != NULL && !uproto_bt_uuid_valid(uuid_filter))) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_CLIENT) == 0u ||
        bluetooth->ops->gatt_discover_services == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->gatt_discover_services(bluetooth->context, peer, uuid_filter);
}

uetd_status_t uproto_bt_gatt_discover_characteristics(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                                      uetd_u16_t service_start_handle, uetd_u16_t service_end_handle,
                                                      const uproto_bt_uuid_t* uuid_filter) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) || service_start_handle == 0u ||
        service_end_handle < service_start_handle || (uuid_filter != NULL && !uproto_bt_uuid_valid(uuid_filter)))
        return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_CLIENT) == 0u ||
        bluetooth->ops->gatt_discover_characteristics == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->gatt_discover_characteristics(bluetooth->context, peer, service_start_handle,
                                                          service_end_handle, uuid_filter);
}

uetd_status_t uproto_bt_gatt_discover_descriptors(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                                  uetd_u16_t characteristic_value_handle, uetd_u16_t end_handle) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) ||
        characteristic_value_handle == 0u || end_handle < characteristic_value_handle) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_CLIENT) == 0u ||
        bluetooth->ops->gatt_discover_descriptors == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->gatt_discover_descriptors(bluetooth->context, peer, characteristic_value_handle, end_handle);
}

uetd_status_t uproto_bt_gatt_read(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                                  uetd_u8_t* value, uetd_u16_t capacity, uetd_u16_t* length) {
    uetd_status_t status;
    uetd_u16_t actual_length = 0u;
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) || attribute_handle == 0u || value == NULL ||
        capacity == 0u || capacity > UPROTO_BT_GATT_VALUE_MAX || length == NULL) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_CLIENT) == 0u || bluetooth->ops->gatt_read == NULL) return UETD_NOT_SUPPORTED;
    status = bluetooth->ops->gatt_read(bluetooth->context, peer, attribute_handle, value, capacity, &actual_length);
    if (status != UETD_OK) return status;
    if (actual_length > capacity) return UETD_OVERFLOW;
    *length = actual_length;
    return UETD_OK;
}

uetd_status_t uproto_bt_gatt_write(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                                   const uetd_u8_t* value, uetd_u16_t length, bool with_response) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) || attribute_handle == 0u ||
        length > UPROTO_BT_GATT_VALUE_MAX || (length != 0u && value == NULL)) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_CLIENT) == 0u || bluetooth->ops->gatt_write == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->gatt_write(bluetooth->context, peer, attribute_handle, value, length, with_response);
}

uetd_status_t uproto_bt_gatt_notify(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t attribute_handle,
                                    const uetd_u8_t* value, uetd_u16_t length, bool indicate) {
    if (bluetooth == NULL || !bluetooth->bound || attribute_handle == 0u || length > UPROTO_BT_GATT_VALUE_MAX ||
        (length != 0u && value == NULL) || (peer != NULL && !uproto_bt_address_valid(peer))) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_SERVER) == 0u || bluetooth->ops->gatt_notify == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->gatt_notify(bluetooth->context, peer, attribute_handle, value, length, indicate);
}

uetd_status_t uproto_bt_gatt_register_service(uproto_bt_t* bluetooth, const uproto_bt_gatt_service_t* service,
                                              uetd_u16_t* service_handle,
                                              uetd_u16_t* characteristic_value_handles,
                                              uetd_u8_t handle_capacity) {
    uetd_u8_t i;
    uetd_u8_t handle_count = 0u;
    uetd_status_t status;
    if (bluetooth == NULL || !bluetooth->bound || service == NULL || !uproto_bt_uuid_valid(&service->uuid) ||
        service_handle == NULL || service->characteristic_count == 0u || service->characteristics == NULL ||
        characteristic_value_handles == NULL || handle_capacity < service->characteristic_count) return UETD_INVALID_ARG;
    for (i = 0u; i < service->characteristic_count; ++i) {
        const uproto_bt_gatt_characteristic_t* characteristic = &service->characteristics[i];
        const uetd_u8_t known_properties = UPROTO_BT_GATT_PROP_READ | UPROTO_BT_GATT_PROP_WRITE |
                                           UPROTO_BT_GATT_PROP_WRITE_NO_RESPONSE | UPROTO_BT_GATT_PROP_NOTIFY |
                                           UPROTO_BT_GATT_PROP_INDICATE;
        if (!uproto_bt_uuid_valid(&characteristic->uuid) || characteristic->maximum_value_length == 0u ||
            characteristic->maximum_value_length > UPROTO_BT_GATT_VALUE_MAX || characteristic->properties == 0u ||
            (characteristic->properties & ~known_properties) != 0u ||
            characteristic->minimum_security > UPROTO_BT_SECURITY_SECURE_CONNECTIONS) return UETD_INVALID_ARG;
        if (characteristic->minimum_security != UPROTO_BT_SECURITY_NONE &&
            (bluetooth->capabilities & UPROTO_BT_CAP_SECURITY) == 0u) return UETD_NOT_SUPPORTED;
    }
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_SERVER) == 0u || bluetooth->ops->gatt_register_service == NULL)
        return UETD_NOT_SUPPORTED;
    *service_handle = 0u;
    for (i = 0u; i < handle_capacity; ++i) characteristic_value_handles[i] = 0u;
    status = bluetooth->ops->gatt_register_service(bluetooth->context, service, service_handle,
                                                   characteristic_value_handles, handle_capacity, &handle_count);
    if (status != UETD_OK) return status;
    if (*service_handle == 0u || handle_count != service->characteristic_count) {
        if (*service_handle != 0u && bluetooth->ops->gatt_unregister_service != NULL)
            (void)bluetooth->ops->gatt_unregister_service(bluetooth->context, *service_handle);
        *service_handle = 0u;
        return UETD_ERROR;
    }
    for (i = 0u; i < handle_count; ++i) {
        if (characteristic_value_handles[i] == 0u) {
            if (bluetooth->ops->gatt_unregister_service != NULL)
                (void)bluetooth->ops->gatt_unregister_service(bluetooth->context, *service_handle);
            *service_handle = 0u;
            return UETD_ERROR;
        }
    }
    return UETD_OK;
}

uetd_status_t uproto_bt_gatt_unregister_service(uproto_bt_t* bluetooth, uetd_u16_t service_handle) {
    if (bluetooth == NULL || !bluetooth->bound || service_handle == 0u) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_SERVER) == 0u ||
        bluetooth->ops->gatt_unregister_service == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->gatt_unregister_service(bluetooth->context, service_handle);
}

uetd_status_t uproto_bt_gatt_subscribe(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                       uetd_u16_t characteristic_value_handle, bool enabled, bool indicate) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) || characteristic_value_handle == 0u)
        return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_CLIENT) == 0u || bluetooth->ops->gatt_subscribe == NULL)
        return UETD_NOT_SUPPORTED;
    return bluetooth->ops->gatt_subscribe(bluetooth->context, peer, characteristic_value_handle, enabled, indicate);
}

uetd_status_t uproto_bt_gatt_server_respond(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer,
                                            uetd_u32_t transaction_id, uetd_status_t result,
                                            const uetd_u8_t* value, uetd_u16_t length) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) || transaction_id == 0u ||
        length > UPROTO_BT_GATT_VALUE_MAX || (length != 0u && value == NULL)) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_GATT_SERVER) == 0u ||
        bluetooth->ops->gatt_server_respond == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->gatt_server_respond(bluetooth->context, peer, transaction_id, result, value, length);
}

uetd_status_t uproto_bt_set_security(uproto_bt_t* bluetooth, uproto_bt_security_t security, bool require_bonding) {
    if (bluetooth == NULL || !bluetooth->bound || security < UPROTO_BT_SECURITY_NONE ||
        security > UPROTO_BT_SECURITY_SECURE_CONNECTIONS || (require_bonding && security == UPROTO_BT_SECURITY_NONE))
        return UETD_INVALID_ARG;
    if (security != UPROTO_BT_SECURITY_NONE && (bluetooth->capabilities & UPROTO_BT_CAP_SECURITY) == 0u)
        return UETD_NOT_SUPPORTED;
    if (require_bonding && (bluetooth->capabilities & UPROTO_BT_CAP_BONDED_SECURITY) == 0u)
        return UETD_NOT_SUPPORTED;
    if (bluetooth->ops->set_security == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->set_security(bluetooth->context, security, require_bonding);
}

uetd_status_t uproto_bt_spp_open(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u16_t timeout_ms) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) || timeout_ms == 0u) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_CLASSIC_SPP) == 0u || bluetooth->ops->spp_open == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->spp_open(bluetooth->context, peer, timeout_ms);
}

uetd_status_t uproto_bt_spp_write(uproto_bt_t* bluetooth, const uetd_u8_t* bytes, uetd_u16_t length, uetd_u16_t* written) {
    uetd_u16_t actual_length = 0u;
    uetd_status_t status;
    if (bluetooth == NULL || !bluetooth->bound || bytes == NULL || length == 0u || written == NULL) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_CLASSIC_SPP) == 0u || bluetooth->ops->spp_write == NULL) return UETD_NOT_SUPPORTED;
    status = bluetooth->ops->spp_write(bluetooth->context, bytes, length, &actual_length);
    if (status != UETD_OK) return status;
    if (actual_length > length) return UETD_OVERFLOW;
    *written = actual_length;
    return UETD_OK;
}

uetd_status_t uproto_bt_spp_close(uproto_bt_t* bluetooth) {
    if (bluetooth == NULL || !bluetooth->bound) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_CLASSIC_SPP) == 0u) return UETD_NOT_SUPPORTED;
    if (bluetooth->ops->spp_close == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->spp_close(bluetooth->context);
}

uetd_status_t uproto_bt_poll(uproto_bt_t* bluetooth, uetd_u16_t work_budget) {
    if (bluetooth == NULL || !bluetooth->bound || work_budget == 0u) return UETD_INVALID_ARG;
    if (bluetooth->ops->poll == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->poll(bluetooth->context, work_budget);
}

uetd_status_t uproto_bt_next_event(uproto_bt_t* bluetooth, uproto_bt_event_t* event) {
    uetd_status_t status;
    if (bluetooth == NULL || !bluetooth->bound || event == NULL) return UETD_INVALID_ARG;
    if (bluetooth->ops->next_event == NULL) return UETD_NOT_SUPPORTED;
    status = bluetooth->ops->next_event(bluetooth->context, event);
    if (status != UETD_OK) return status;
    if (event->data_length > UPROTO_BT_EVENT_DATA_MAX) return UETD_OVERFLOW;
    if (event->kind < UPROTO_BT_EVENT_SCAN_RESULT || event->kind > UPROTO_BT_EVENT_SECURITY ||
        !uproto_bt_address_valid(&event->peer)) return UETD_ERROR;
    switch (event->kind) {
    case UPROTO_BT_EVENT_GATT_SERVICE:
        if (event->handle == 0u || event->end_handle < event->handle || !uproto_bt_uuid_valid(&event->uuid)) return UETD_ERROR;
        break;
    case UPROTO_BT_EVENT_GATT_CHARACTERISTIC:
        if (event->handle == 0u || event->parent_handle == 0u || !uproto_bt_uuid_valid(&event->uuid) ||
            event->maximum_value_length == 0u || event->maximum_value_length > UPROTO_BT_GATT_VALUE_MAX ||
            event->properties == 0u ||
            (event->properties & (uetd_u8_t)~(UPROTO_BT_GATT_PROP_READ | UPROTO_BT_GATT_PROP_WRITE |
                                               UPROTO_BT_GATT_PROP_WRITE_NO_RESPONSE | UPROTO_BT_GATT_PROP_NOTIFY |
                                               UPROTO_BT_GATT_PROP_INDICATE)) != 0u ||
            event->minimum_security > UPROTO_BT_SECURITY_SECURE_CONNECTIONS) return UETD_ERROR;
        break;
    case UPROTO_BT_EVENT_GATT_DESCRIPTOR:
        if (event->handle == 0u || event->parent_handle == 0u || !uproto_bt_uuid_valid(&event->uuid)) return UETD_ERROR;
        break;
    case UPROTO_BT_EVENT_GATT_READ_REQUEST:
    case UPROTO_BT_EVENT_GATT_WRITE_REQUEST:
        if (event->handle == 0u || event->transaction_id == 0u) return UETD_ERROR;
        break;
    case UPROTO_BT_EVENT_GATT_NOTIFICATION:
        if (event->handle == 0u) return UETD_ERROR;
        break;
    default:
        break;
    }
    return UETD_OK;
}

uetd_status_t uproto_bt_pair(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer, uetd_u32_t passkey) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer) || passkey > 999999u) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_BONDED_SECURITY) == 0u || bluetooth->ops->pair == NULL) return UETD_NOT_SUPPORTED;
    return bluetooth->ops->pair(bluetooth->context, peer, passkey);
}

uetd_status_t uproto_bt_remove_bond(uproto_bt_t* bluetooth, const uproto_bt_address_t* peer) {
    if (bluetooth == NULL || !bluetooth->bound || !uproto_bt_address_valid(peer)) return UETD_INVALID_ARG;
    if ((bluetooth->capabilities & UPROTO_BT_CAP_BONDED_SECURITY) == 0u || bluetooth->ops->remove_bond == NULL)
        return UETD_NOT_SUPPORTED;
    return bluetooth->ops->remove_bond(bluetooth->context, peer);
}
