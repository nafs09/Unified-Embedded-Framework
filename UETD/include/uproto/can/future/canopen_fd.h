/// @file include/uproto/can/future/canopen_fd.h
/// @brief Future CANopen FD (CiA 1301) adapter with bounded USDO/PDO storage.
///
/// Design-only, fail-closed operations. Not included by uproto.h and not
/// selectable for generated firmware. This is separate from CiA 301 CANopen;
/// increasing a classic SDO/PDO buffer does not implement CANopen FD.
/// Select the specification revision, provider and device profile before
/// implementing wire behavior. MEDI CAN is a separate protocol extension with
/// its own scaffold in can/future/medi_can.h. See docs/design/can/MEDI_CAN.md.
#ifndef UETD_UPROTO_FUTURE_CANOPEN_FD_H
#define UETD_UPROTO_FUTURE_CANOPEN_FD_H

#include "uproto/industrial/industrial_contracts.h"
#include "upal/can/fdcan_fd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* UETD storage budgets, not limits imposed by CiA 1301. Larger transfers
 * require another reviewed storage/streaming contract before implementation. */
#define UPROTO_CANOPEN_FD_NODE_CAPACITY 128u
#define UPROTO_CANOPEN_FD_EVENT_CAPACITY 8u
#define UPROTO_CANOPEN_FD_USDO_CAPACITY 4u
#define UPROTO_CANOPEN_FD_USDO_MAX_BYTES 256u
#define UPROTO_CANOPEN_FD_PDO_MAX_BYTES 64u

typedef struct {
    uetd_u8_t node_id;
    uetd_u8_t maximum_nodes;
    uetd_u16_t maximum_usdo_bytes;
    uetd_u8_t maximum_pending_usdo;
    uetd_u32_t usdo_timeout_us;
    uetd_u16_t heartbeat_producer_ms;
    uetd_u16_t heartbeat_consumer_timeout_ms;
    uetd_u32_t sync_period_us;
    uetd_u16_t pdo_count;
    uetd_u8_t maximum_pdo_bytes;
    /* Immutable caller-owned dictionary, alive for the instance lifetime. */
    const uproto_canopen_object_t* object_dictionary;
    uetd_u16_t object_count;
} uproto_canopen_fd_config_t;

typedef enum {
    UPROTO_CANOPEN_FD_EVENT_NMT = 0,
    UPROTO_CANOPEN_FD_EVENT_BOOTUP,
    UPROTO_CANOPEN_FD_EVENT_HEARTBEAT_TIMEOUT,
    UPROTO_CANOPEN_FD_EVENT_PDO,
    UPROTO_CANOPEN_FD_EVENT_USDO_COMPLETE,
    UPROTO_CANOPEN_FD_EVENT_USDO_ABORT,
    UPROTO_CANOPEN_FD_EVENT_EMCY,
    UPROTO_CANOPEN_FD_EVENT_SYNC,
    UPROTO_CANOPEN_FD_EVENT_TIME,
} uproto_canopen_fd_event_kind_t;

typedef struct {
    uproto_canopen_fd_event_kind_t kind;
    uetd_u8_t node_id;
    uetd_u16_t index;
    uetd_u8_t subindex;
    uetd_u32_t abort_code;
    uetd_u16_t data_length;
    /* Complete bounded semantic data, not a serialized CAN FD frame. */
    uetd_u8_t data[UPROTO_CANOPEN_FD_USDO_MAX_BYTES];
    uetd_u64_t received_at_us;
} uproto_canopen_fd_event_t;

typedef struct {
    uetd_u8_t node_id;
    uetd_u16_t index;
    uetd_u8_t subindex;
    bool active;
    bool write;
    uetd_u64_t deadline_us;
    uetd_u16_t length;
    uetd_u8_t bytes[UPROTO_CANOPEN_FD_USDO_MAX_BYTES];
    /* Provider-owned wire session/segmentation state must be specified before
     * promotion. Extension-protocol transaction state belongs to its module. */
} uproto_canopen_fd_usdo_transaction_t;

typedef struct {
    uproto_canopen_fd_config_t config;
    uproto_canopen_nmt_state_t node_states[UPROTO_CANOPEN_FD_NODE_CAPACITY];
    uetd_u64_t heartbeat_deadlines_us[UPROTO_CANOPEN_FD_NODE_CAPACITY];
    uproto_canopen_fd_usdo_transaction_t usdo[UPROTO_CANOPEN_FD_USDO_CAPACITY];
    uproto_canopen_fd_event_t events[UPROTO_CANOPEN_FD_EVENT_CAPACITY];
    uetd_u8_t event_read_index;
    uetd_u8_t event_write_index;
    uetd_u32_t dropped_events;
} uproto_canopen_fd_t;

/// Future validation of dictionary/profile, resource budgets and FD transport.
/// All operations currently return UETD_NOT_SUPPORTED without changing caller
/// storage; they do not validate, transmit, decode or execute commands.
uetd_status_t uproto_canopen_fd_validate(const uproto_canopen_fd_config_t* config);

/// Future initialization; provider and transmit binding remain to be selected.
uetd_status_t uproto_canopen_fd_init(uproto_canopen_fd_t* protocol, const uproto_canopen_fd_config_t* config);

/// Future frame ingress retains FD/identifier/BRS/ESI flags and receive time.
uetd_status_t uproto_canopen_fd_process_frame(uproto_canopen_fd_t* protocol, const upal_fdcan_fd_frame_t* frame,
                                             uetd_u64_t received_at_us);

/// Future retrieval of a complete event; overflow must affect data quality.
uetd_status_t uproto_canopen_fd_next_event(uproto_canopen_fd_t* protocol, uproto_canopen_fd_event_t* event);

/// Future bounded NMT/heartbeat/USDO deadline processing.
uetd_status_t uproto_canopen_fd_poll(uproto_canopen_fd_t* protocol, uetd_u64_t now_us);

/// Future unicast USDO read; standard wire session matching belongs to provider.
uetd_status_t uproto_canopen_fd_usdo_read(uproto_canopen_fd_t* protocol, uetd_u8_t node_id, uetd_u16_t index,
                                         uetd_u8_t subindex);

/// Future unicast USDO write; enforce access/length and complete-transfer commit.
uetd_status_t uproto_canopen_fd_usdo_write(uproto_canopen_fd_t* protocol, uetd_u8_t node_id, uetd_u16_t index,
                                          uetd_u8_t subindex, const uetd_u8_t* data, uetd_u16_t length);

/// Future runtime reset; extension replay/session lifecycle belongs to its module.
uetd_status_t uproto_canopen_fd_reset(uproto_canopen_fd_t* protocol);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_FUTURE_CANOPEN_FD_H */
