/// @file include/uapp/product_session.h
/// @brief Bounded command admission and evidence-based peer outcomes.
#ifndef UETD_UAPP_PRODUCT_SESSION_H
#define UETD_UAPP_PRODUCT_SESSION_H
#include "ucore/span.h"
#include "ucore/deadline.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifndef UAPP_PRODUCT_PAYLOAD_CAPACITY
#define UAPP_PRODUCT_PAYLOAD_CAPACITY 256u
#endif
typedef enum { UAPP_COMMAND_REQUESTED = 0, UAPP_COMMAND_ADMITTED, UAPP_COMMAND_SENT,
    UAPP_COMMAND_PEER_ACCEPTED, UAPP_COMMAND_EXECUTING, UAPP_COMMAND_COMPLETED,
    UAPP_COMMAND_REJECTED, UAPP_COMMAND_EXPIRED, UAPP_COMMAND_UNKNOWN } uapp_command_state_t;
typedef struct { uetd_u8_t identity[32]; uetd_u32_t identity_schema; } uapp_product_identity_t;
typedef struct {
    uetd_u64_t id, session_generation, authority_generation;
    ucore_deadline_t expires;
    uetd_u32_t schema_id;
    ucore_read_span_t payload;
} uapp_product_command_t;
typedef struct {
    uetd_u64_t command_id, session_generation;
    uapp_command_state_t state;
    uetd_status_t result;
} uapp_product_outcome_t;
typedef struct {
    uapp_product_command_t command;
    uapp_product_outcome_t outcome;
    uetd_u8_t payload[UAPP_PRODUCT_PAYLOAD_CAPACITY];
    bool occupied;
} uapp_product_slot_t;
typedef struct {
    uetd_status_t (*now)(void* context, ucore_clock_stamp_t* now);
    /* Must verify authenticated identity, current authority, schema and output policy.
     * Called at admission AND immediately before sending. No mutation/actuation. */
    uetd_status_t (*authorize)(void* context, const uapp_product_identity_t* peer, const uapp_product_command_t* command);
    /* Synchronous borrowing. OK means sent, never physical completion. BUSY must
     * mean no bytes accepted; every other error has an uncertain outcome. */
    uetd_status_t (*send)(void* context, const uapp_product_identity_t* peer, const uapp_product_command_t* command);
    /* Nonblocking evidence lookup. NOT_READY means no new evidence. Never replay.
     * Evidence must match peer, command ID and session generation. */
    uetd_status_t (*observe)(void* context, const uapp_product_identity_t* peer, uapp_product_outcome_t* outcome);
} uapp_product_provider_t;
typedef struct {
    uapp_product_identity_t peer;
    size_t maximum_pending_commands;
    void* provider_context;
    uapp_product_slot_t* slots;
    uapp_product_provider_t provider;
} uapp_product_session_config_t;
typedef struct {
    uapp_product_session_config_t config;
    uetd_u64_t generation, last_command_id;
    size_t cursor;
    bool initialized, busy;
} uapp_product_session_t;
#define UAPP_PRODUCT_SESSION_INITIALIZER {0}
/* Owner-only, no ISR calls, no reentrancy; initialize with {0}. Provider callbacks
 * are bounded and borrow their arguments only during the call. Slots outlive the
 * session. Command payloads are copied. Terminal results occupy their slot until
 * next_outcome consumes it. IDs increase for the lifetime of the object.
 * Reconnect/handover must invalidate; it never resends admitted/sent old commands. */
uetd_status_t uapp_product_session_init(uapp_product_session_t* session, const uapp_product_session_config_t* config);
uetd_status_t uapp_product_session_submit(uapp_product_session_t* session, const uapp_product_command_t* command);
uetd_status_t uapp_product_session_poll(uapp_product_session_t* session, uetd_u16_t work_budget);
uetd_status_t uapp_product_session_next_outcome(uapp_product_session_t* session, uapp_product_outcome_t* outcome);
uetd_status_t uapp_product_session_invalidate(uapp_product_session_t* session);
#ifdef __cplusplus
}
#endif
#endif
