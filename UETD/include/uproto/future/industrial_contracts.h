/// @file include/uproto/future/industrial_contracts.h
/// @brief Design-only typed outlines for future industrial protocol adapters.
///
/// Not included by uproto.h and not backed by implementation or generation
/// manifests. The process-image contract remains transport-independent.

#ifndef UPROTO_FUTURE_INDUSTRIAL_CONTRACTS_H
#define UPROTO_FUTURE_INDUSTRIAL_CONTRACTS_H

#include "ucore/status.h"
#include "ucore/types.h"

typedef enum {
    UPROTO_MODBUS_RTU = 0,
    UPROTO_MODBUS_TCP,
} uproto_modbus_transport_t;

typedef enum {
    UPROTO_MODBUS_CLIENT = 0,
    UPROTO_MODBUS_SERVER,
} uproto_modbus_role_t;

typedef enum {
    UPROTO_MODBUS_FC_READ_COILS = 1,
    UPROTO_MODBUS_FC_READ_DISCRETE_INPUTS = 2,
    UPROTO_MODBUS_FC_READ_HOLDING_REGISTERS = 3,
    UPROTO_MODBUS_FC_READ_INPUT_REGISTERS = 4,
    UPROTO_MODBUS_FC_WRITE_SINGLE_COIL = 5,
    UPROTO_MODBUS_FC_WRITE_SINGLE_REGISTER = 6,
    UPROTO_MODBUS_FC_WRITE_MULTIPLE_COILS = 15,
    UPROTO_MODBUS_FC_WRITE_MULTIPLE_REGISTERS = 16,
} uproto_modbus_function_t;

typedef struct {
    uetd_u16_t first_address;
    uetd_u16_t quantity;
    uetd_u16_t process_point_id;
    uetd_u8_t function_code;
    bool writable;
} uproto_modbus_mapping_t;

typedef struct {
    uproto_modbus_transport_t transport;
    uproto_modbus_role_t role;
    uetd_u8_t unit_id;
    uetd_u16_t maximum_pdu_bytes;
    uetd_u16_t maximum_adu_bytes;
    uetd_u8_t maximum_connections;
    uetd_u8_t maximum_outstanding_transactions;
    uetd_u32_t request_timeout_ms;
    uetd_u32_t inter_byte_timeout_us;
    uetd_u32_t silent_interval_us;
    const uproto_modbus_mapping_t* mapping;
    uetd_u16_t mapping_count;
} uproto_modbus_config_t;

typedef struct {
    uetd_u16_t transaction_id;
    uetd_u8_t function_code;
    uetd_u8_t exception_code;
    uetd_u16_t payload_length;
    uetd_u8_t payload[253];
    uetd_u32_t flags; /* timeout, malformed, duplicate, truncated, transport */
} uproto_modbus_event_t;

#define UPROTO_MODBUS_RX_CAPACITY 260u
#define UPROTO_MODBUS_EVENT_CAPACITY 8u
#define UPROTO_MODBUS_TRANSACTION_CAPACITY 8u

typedef struct {
    uetd_u16_t transaction_id;
    uetd_u8_t function_code;
    bool active;
    uetd_u64_t deadline_us;
} uproto_modbus_transaction_t;

typedef struct {
    uproto_modbus_config_t config;
    uetd_u8_t rx_bytes[UPROTO_MODBUS_RX_CAPACITY];
    uetd_u16_t rx_length;
    uetd_u64_t last_byte_at_us;
    uetd_u16_t next_transaction_id;
    uproto_modbus_transaction_t transactions[UPROTO_MODBUS_TRANSACTION_CAPACITY];
    uproto_modbus_event_t events[UPROTO_MODBUS_EVENT_CAPACITY];
    uetd_u8_t event_read_index;
    uetd_u8_t event_write_index;
    uetd_u32_t dropped_events;
} uproto_modbus_t;

uetd_status_t uproto_modbus_validate(const uproto_modbus_config_t* config);
uetd_status_t uproto_modbus_init(uproto_modbus_t* protocol, const uproto_modbus_config_t* config);
uetd_status_t uproto_modbus_feed(uproto_modbus_t* protocol, const uetd_u8_t* bytes, uetd_u16_t length, uetd_u64_t received_at_us);
uetd_status_t uproto_modbus_next_event(uproto_modbus_t* protocol, uproto_modbus_event_t* event);
uetd_status_t uproto_modbus_build_request(uproto_modbus_t* protocol, uetd_u8_t function_code, uetd_u16_t first_address,
                                         uetd_u16_t quantity, const uetd_u8_t* payload, uetd_u16_t payload_length, uetd_u8_t* frame,
                                         uetd_u16_t capacity, uetd_u16_t* frame_length);
uetd_status_t uproto_modbus_poll(uproto_modbus_t* protocol, uetd_u64_t now_us);
uetd_status_t uproto_modbus_reset(uproto_modbus_t* protocol);

typedef enum {
    UPROTO_CANOPEN_NMT_INITIALIZING = 0,
    UPROTO_CANOPEN_NMT_PRE_OPERATIONAL,
    UPROTO_CANOPEN_NMT_OPERATIONAL,
    UPROTO_CANOPEN_NMT_STOPPED,
} uproto_canopen_nmt_state_t;

typedef enum {
    UPROTO_CANOPEN_OD_READ_ONLY = 0,
    UPROTO_CANOPEN_OD_WRITE_ONLY,
    UPROTO_CANOPEN_OD_READ_WRITE,
    UPROTO_CANOPEN_OD_CONSTANT,
} uproto_canopen_access_t;

typedef struct {
    uetd_u16_t index;
    uetd_u8_t subindex;
    uetd_u8_t type_id;
    uproto_canopen_access_t access;
    uetd_u16_t process_point_id;
    uetd_u16_t maximum_bytes;
} uproto_canopen_object_t;

typedef struct {
    uetd_u8_t node_id;
    uetd_u8_t maximum_nodes;
    uetd_u16_t maximum_sdo_bytes;
    uetd_u16_t maximum_pending_sdo;
    uetd_u16_t heartbeat_producer_ms;
    uetd_u16_t heartbeat_consumer_timeout_ms;
    uetd_u32_t sync_period_us;
    uetd_u8_t pdo_count;
    uetd_u8_t maximum_pdo_bytes;
    const uproto_canopen_object_t* object_dictionary;
    uetd_u16_t object_count;
} uproto_canopen_config_t;

typedef enum {
    UPROTO_CANOPEN_EVENT_NMT = 0,
    UPROTO_CANOPEN_EVENT_BOOTUP,
    UPROTO_CANOPEN_EVENT_HEARTBEAT_TIMEOUT,
    UPROTO_CANOPEN_EVENT_PDO,
    UPROTO_CANOPEN_EVENT_SDO_COMPLETE,
    UPROTO_CANOPEN_EVENT_SDO_ABORT,
    UPROTO_CANOPEN_EVENT_EMCY,
    UPROTO_CANOPEN_EVENT_SYNC,
} uproto_canopen_event_kind_t;

typedef struct {
    uproto_canopen_event_kind_t kind;
    uetd_u8_t node_id;
    uetd_u8_t flags;
    uetd_u16_t index;
    uetd_u8_t subindex;
    uetd_u8_t data_length;
    uetd_u8_t data[8];
    uetd_u64_t timestamp_us;
} uproto_canopen_event_t;

#define UPROTO_CANOPEN_NODE_CAPACITY 128u
#define UPROTO_CANOPEN_EVENT_CAPACITY 32u
#define UPROTO_CANOPEN_SDO_CAPACITY 4u

typedef struct {
    uetd_u16_t index;
    uetd_u8_t subindex;
    uetd_u8_t node_id;
    bool write;
    uetd_u64_t deadline_us;
    uetd_u16_t length;
    uetd_u8_t bytes[256];
} uproto_canopen_sdo_transaction_t;

typedef struct {
    uproto_canopen_config_t config;
    uproto_canopen_nmt_state_t node_states[UPROTO_CANOPEN_NODE_CAPACITY];
    uetd_u64_t heartbeat_deadlines_us[UPROTO_CANOPEN_NODE_CAPACITY];
    uproto_canopen_sdo_transaction_t sdo[UPROTO_CANOPEN_SDO_CAPACITY];
    uproto_canopen_event_t events[UPROTO_CANOPEN_EVENT_CAPACITY];
    uetd_u8_t event_read_index;
    uetd_u8_t event_write_index;
    uetd_u32_t dropped_events;
} uproto_canopen_t;

uetd_status_t uproto_canopen_validate(const uproto_canopen_config_t* config);
uetd_status_t uproto_canopen_init(uproto_canopen_t* protocol, const uproto_canopen_config_t* config);
uetd_status_t uproto_canopen_process_frame(uproto_canopen_t* protocol, uetd_u32_t can_id, const uetd_u8_t* data, uetd_u8_t length,
                                          uetd_u64_t received_at_us);
uetd_status_t uproto_canopen_next_event(uproto_canopen_t* protocol, uproto_canopen_event_t* event);
uetd_status_t uproto_canopen_poll(uproto_canopen_t* protocol, uetd_u64_t now_us);
uetd_status_t uproto_canopen_sdo_read(uproto_canopen_t* protocol, uetd_u8_t node_id, uetd_u16_t index, uetd_u8_t subindex);
uetd_status_t uproto_canopen_sdo_write(uproto_canopen_t* protocol, uetd_u8_t node_id, uetd_u16_t index, uetd_u8_t subindex,
                                      const uetd_u8_t* data, uetd_u16_t length);
uetd_status_t uproto_canopen_reset(uproto_canopen_t* protocol);

#endif /* UPROTO_FUTURE_INDUSTRIAL_CONTRACTS_H */
