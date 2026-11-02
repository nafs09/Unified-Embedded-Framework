/// @file include/uproto/can/future/medi_can.h
/// @brief Myovena MEDI CAN protocol extension of CANopen FD: future scaffold.
///
/// MEDI owns its versioned protocol, delivery tracking and execution contract.
/// Wire framing, CANopen FD binding, integrity algorithms, durable recovery and
/// trusted execution provider are deliberately unresolved. These are semantic
/// storage types, not wire layouts. See docs/design/can/MEDI_CAN.md.
/// Every operation returns UETD_NOT_SUPPORTED and preserves caller storage.
/// No operation currently sends, acknowledges, validates or executes anything.
#ifndef UETD_UPROTO_FUTURE_MEDI_CAN_H
#define UETD_UPROTO_FUTURE_MEDI_CAN_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/can/fdcan_fd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initial UETD planning budgets, not MEDI wire-protocol or product limits. */
#define UPROTO_MEDI_CAN_PEER_CAPACITY 4u
#define UPROTO_MEDI_CAN_TRANSACTION_CAPACITY 8u
#define UPROTO_MEDI_CAN_RESULT_CAPACITY 16u
#define UPROTO_MEDI_CAN_EVENT_CAPACITY 16u
#define UPROTO_MEDI_CAN_PAYLOAD_CAPACITY 256u
#define UPROTO_MEDI_CAN_DIGEST_CAPACITY 32u

typedef struct {
    uetd_u16_t source_endpoint;
    uetd_u16_t destination_endpoint;
    uetd_u64_t session_epoch;
    uetd_u64_t transaction_id;
} uproto_medi_can_identity_t;

typedef struct {
    uproto_medi_can_identity_t identity;
    uetd_u16_t protocol_version;
    uetd_u16_t operation;
    uetd_u64_t sequence;
    uetd_u64_t freshness_lease;
    uetd_u16_t payload_length;
    uetd_u8_t payload[UPROTO_MEDI_CAN_PAYLOAD_CAPACITY];
    /* Integrity/framing metadata must be designed before codec implementation.
     * An endpoint identifier alone does not authenticate a sender. */
} uproto_medi_can_command_t;

/* Semantic progress only. These values do not allocate packet opcodes. */
typedef enum {
    UPROTO_MEDI_CAN_STAGE_UNUSED = 0,
    UPROTO_MEDI_CAN_STAGE_ADMITTED,
    UPROTO_MEDI_CAN_STAGE_RECEIVED,
    UPROTO_MEDI_CAN_STAGE_VALIDATED,
    UPROTO_MEDI_CAN_STAGE_EXECUTING,
    UPROTO_MEDI_CAN_STAGE_EXECUTED,
    UPROTO_MEDI_CAN_STAGE_REJECTED,
    UPROTO_MEDI_CAN_STAGE_DELIVERY_FAILED,
    UPROTO_MEDI_CAN_STAGE_OUTCOME_UNKNOWN,
} uproto_medi_can_stage_t;

typedef struct {
    uetd_u16_t protocol_version;
    uetd_u16_t local_endpoint;
    uetd_u8_t maximum_peers;
    uetd_u8_t maximum_inflight;
    uetd_u16_t maximum_payload_bytes;
    uetd_u8_t maximum_attempts;
    uetd_u32_t acknowledgement_timeout_us;
    uetd_u32_t delivery_timeout_us;
    uetd_u32_t maximum_command_age_us;
    uetd_u32_t validation_timeout_us;
    uetd_u32_t execution_timeout_us;
    uetd_u32_t result_retention_us;
    uetd_u16_t maximum_work_per_poll;
    bool require_authenticated_integrity;
    bool require_hardware_validation;
    bool require_durable_recovery;
    /* Trusted clock, CANopen FD transport, journal, validator and executor
     * provider contracts must be supplied before any successful init exists. */
} uproto_medi_can_config_t;

typedef struct {
    uproto_medi_can_command_t command;
    uproto_medi_can_stage_t stage;
    uetd_u64_t admitted_at_us;
    uetd_u64_t deadline_us;
    uetd_u64_t acknowledgement_deadline_us;
    uetd_u8_t attempts;
} uproto_medi_can_transaction_t;

typedef struct {
    uproto_medi_can_identity_t identity;
    uproto_medi_can_stage_t stage;
    /* Canonical command binding: algorithm, encoded length and provider
     * evidence are not selected. This storage is not an authorization token. */
    uetd_u8_t command_digest[UPROTO_MEDI_CAN_DIGEST_CAPACITY];
    uetd_u64_t policy_generation;
    uetd_u64_t recorded_at_us;
    uetd_u32_t reason;
} uproto_medi_can_result_t;

typedef struct {
    uetd_u16_t endpoint;
    uetd_u64_t session_epoch;
    uetd_u64_t next_receive_sequence;
    bool session_established;
    bool reconciliation_required;
} uproto_medi_can_peer_t;

typedef enum {
    UPROTO_MEDI_CAN_EVENT_PROGRESS = 0,
    UPROTO_MEDI_CAN_EVENT_DUPLICATE,
    UPROTO_MEDI_CAN_EVENT_SEQUENCE_GAP,
    UPROTO_MEDI_CAN_EVENT_INTEGRITY_FAILURE,
    UPROTO_MEDI_CAN_EVENT_STALE,
    UPROTO_MEDI_CAN_EVENT_RESOURCE_FAULT,
    UPROTO_MEDI_CAN_EVENT_LINK_FAULT,
    UPROTO_MEDI_CAN_EVENT_RECOVERY_REQUIRED,
} uproto_medi_can_event_kind_t;

typedef struct {
    uproto_medi_can_event_kind_t kind;
    uproto_medi_can_identity_t identity;
    uproto_medi_can_stage_t stage;
    uetd_u64_t timestamp_us;
    uetd_u32_t reason;
} uproto_medi_can_event_t;

typedef struct {
    uetd_u32_t retries;
    uetd_u32_t duplicate_packets;
    uetd_u32_t sequence_gaps;
    uetd_u32_t integrity_failures;
    uetd_u32_t stale_commands;
    uetd_u32_t resource_faults;
    uetd_u32_t unresolved_transactions;
    bool execution_inhibited;
    bool fault_latched;
} uproto_medi_can_snapshot_t;

typedef struct {
    uproto_medi_can_config_t config;
    uproto_medi_can_peer_t peers[UPROTO_MEDI_CAN_PEER_CAPACITY];
    uproto_medi_can_transaction_t transactions[UPROTO_MEDI_CAN_TRANSACTION_CAPACITY];
    uproto_medi_can_result_t results[UPROTO_MEDI_CAN_RESULT_CAPACITY];
    uproto_medi_can_event_t events[UPROTO_MEDI_CAN_EVENT_CAPACITY];
    uetd_u8_t event_read_index;
    uetd_u8_t event_write_index;
    uproto_medi_can_snapshot_t diagnostics;
    /* RAM arrays are not a durable journal. Never infer crash-safe execution
     * or recovery from the existence of these fields. */
} uproto_medi_can_t;

/// Future validation of bounded timing, resources and required provider policy.
uetd_status_t uproto_medi_can_validate(const uproto_medi_can_config_t* config);
/// Future provider binding, recovery reconciliation and session initialization.
uetd_status_t uproto_medi_can_init(uproto_medi_can_t* protocol, const uproto_medi_can_config_t* config);
/// Future admission: reserve tracking capacity before accepting a logical command.
uetd_status_t uproto_medi_can_submit(uproto_medi_can_t* protocol, const uproto_medi_can_command_t* command,
                                    uetd_u64_t now_us);
/// Future explicit dispatch/reassembly of MEDI extension frames; never imply
/// that arbitrary standard CANopen FD traffic is a MEDI-authorized command.
uetd_status_t uproto_medi_can_process_frame(uproto_medi_can_t* protocol, const upal_fdcan_fd_frame_t* frame,
                                           uetd_u64_t received_at_us);
/// Future bounded TX retrieval, including protocol ACKs/results and retransmits.
uetd_status_t uproto_medi_can_next_transmit(uproto_medi_can_t* protocol, upal_fdcan_fd_frame_t* frame, uetd_u64_t now_us);
/// Future retry/deadline processing; expiry cannot imply that a remote action
/// did not occur. Unknown execution outcomes require reconciliation.
uetd_status_t uproto_medi_can_poll(uproto_medi_can_t* protocol, uetd_u64_t now_us);
/// Future privileged executor gate: recheck integrity, freshness, authorization,
/// deduplication, journal and interlocks against the exact command at commit.
/// A generic caller or a boolean validation result cannot bypass that gate.
uetd_status_t uproto_medi_can_execute_ready(uproto_medi_can_t* protocol, uetd_u64_t now_us);
/// Future receipt of a bound result from the trusted execution provider.
uetd_status_t uproto_medi_can_record_result(uproto_medi_can_t* protocol, const uproto_medi_can_result_t* result,
                                           uetd_u64_t now_us);
/// Future progress/fault retrieval; event loss must latch a fault, never be silent.
uetd_status_t uproto_medi_can_next_event(uproto_medi_can_t* protocol, uproto_medi_can_event_t* event);
/// Future coherent delivery/execution diagnostics without clearing fault evidence.
uetd_status_t uproto_medi_can_snapshot(const uproto_medi_can_t* protocol, uproto_medi_can_snapshot_t* snapshot);
/// Future transport restart retaining unresolved outcomes and replay evidence;
/// reset is not permission to forget a command or execute it a second time.
uetd_status_t uproto_medi_can_reset(uproto_medi_can_t* protocol);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_FUTURE_MEDI_CAN_H */
