/// @file include/uapp/diagnostics/diagnostics.h
/// @brief Bounded structured records and synchronous authorized maintenance.
#ifndef UETD_UAPP_DIAGNOSTICS_H
#define UETD_UAPP_DIAGNOSTICS_H
#include "ucore/span.h"
#include "ucore/deadline.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifndef UAPP_DIAGNOSTIC_PAYLOAD_CAPACITY
#define UAPP_DIAGNOSTIC_PAYLOAD_CAPACITY 256u
#endif
typedef struct { uetd_u32_t event_id, schema_id, severity; ucore_clock_stamp_t captured; ucore_read_span_t fields; } uapp_diagnostic_record_t;
typedef struct { uetd_u64_t request_id, principal; uetd_u32_t operation, target; ucore_deadline_t deadline; ucore_read_span_t arguments; } uapp_maintenance_request_t;
typedef struct {
    uapp_diagnostic_record_t record;
    uapp_maintenance_request_t request;
    uetd_u8_t payload[UAPP_DIAGNOSTIC_PAYLOAD_CAPACITY];
    bool maintenance;
} uapp_diagnostics_slot_t;
typedef struct {
    uetd_status_t (*now)(void* context, ucore_clock_stamp_t* stamp);
    /* Reject unknown schemas and secret/unredacted fields; no side effects. */
    uetd_status_t (*validate_record)(void* context, const uapp_diagnostic_record_t* record);
    /* Authenticate principal and enforce operation allowlist, schema and output
     * inhibit. Called at admission and immediately before execution. */
    uetd_status_t (*authorize)(void* context, const uapp_maintenance_request_t* request);
    uetd_status_t (*write_record)(void* context, const uapp_diagnostic_record_t* record);
    /* Synchronous, bounded execution; never retried, including BUSY/error returns. */
    uetd_status_t (*execute)(void* context, const uapp_maintenance_request_t* request);
    void (*on_result)(void* context, uetd_u64_t request_id, uetd_status_t result);
} uapp_diagnostics_provider_t;
typedef struct {
    void* provider_context;
    size_t maximum_record_bytes, maximum_command_bytes;
    uetd_u16_t queue_depth;
    uapp_diagnostics_slot_t* slots;
    uapp_diagnostics_provider_t provider;
} uapp_diagnostics_config_t;
typedef struct {
    uapp_diagnostics_config_t config;
    uetd_u64_t generation, last_request_id;
    uetd_u32_t dropped_records, failed_records;
    uetd_u16_t head, tail, count;
    bool initialized, busy;
} uapp_diagnostics_t;
#define UAPP_DIAGNOSTICS_INITIALIZER {0}
/* Owner-only, nonreentrant; initialize with {0}. Payloads are copied. Slots and
 * provider contexts outlive the object. No implicit truncation, formatting or
 * ISR calls. Budget limits dequeued items, callbacks must bound their own cost.
 * Log sink errors drop one record and increment failed_records. Commands are
 * never replayed, and terminal results are delivered once through on_result. */
uetd_status_t uapp_diagnostics_init(uapp_diagnostics_t* diagnostics, const uapp_diagnostics_config_t* config);
uetd_status_t uapp_diagnostics_emit(uapp_diagnostics_t* diagnostics, const uapp_diagnostic_record_t* record);
uetd_status_t uapp_diagnostics_request(uapp_diagnostics_t* diagnostics, const uapp_maintenance_request_t* request);
uetd_status_t uapp_diagnostics_poll(uapp_diagnostics_t* diagnostics, uetd_u16_t work_budget);
#ifdef __cplusplus
}
#endif
#endif
