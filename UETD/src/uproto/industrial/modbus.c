/// @file src/uproto/industrial/modbus.c
/// @brief Fail-closed Modbus operations pending a reviewed wire profile.
#include "uproto/industrial/modbus.h"

uetd_status_t uproto_modbus_validate(const uproto_modbus_config_t* config) {
    /* TODO: define function subsets, exact RTU timing, transaction policy, and TCP bounds. */
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_modbus_init(uproto_modbus_t* protocol, const uproto_modbus_config_t* config) {
    /* TODO: initialize fixed parser/event/transaction storage after validation. */
    (void)protocol;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_modbus_feed(uproto_modbus_t* protocol, const uetd_u8_t* bytes, uetd_u16_t length, uetd_u64_t received_at_us) {
    /* TODO: implement bounded RTU gap and TCP stream-fragment parsing. */
    (void)protocol;
    (void)bytes;
    (void)length;
    (void)received_at_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_modbus_next_event(uproto_modbus_t* protocol, uproto_modbus_event_t* event) {
    /* TODO: publish only complete, validated, non-stale protocol events. */
    (void)protocol;
    (void)event;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_modbus_build_request(uproto_modbus_t* protocol, uetd_u8_t function_code, uetd_u16_t first_address,
                                         uetd_u16_t quantity, const uetd_u8_t* payload, uetd_u16_t payload_length,
                                         uetd_u8_t* frame, uetd_u16_t capacity, uetd_u16_t* frame_length) {
    /* TODO: encode only the reviewed function-code subset with CRC/MBAP bounds. */
    (void)protocol;
    (void)function_code;
    (void)first_address;
    (void)quantity;
    (void)payload;
    (void)payload_length;
    (void)frame;
    (void)capacity;
    (void)frame_length;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_modbus_poll(uproto_modbus_t* protocol, uetd_u64_t now_us) {
    /* TODO: expire bounded transactions and enforce configured RTU silence. */
    (void)protocol;
    (void)now_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_modbus_reset(uproto_modbus_t* protocol) {
    /* TODO: clear partial frames/events without modifying the immutable profile. */
    (void)protocol;
    return UETD_NOT_SUPPORTED;
}
