/// @file include/umid/data/settings.h
/// @brief First-pass scaffold: Generation-checked staged configuration transactions.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/PRODUCT_SERVICES.md#settings-and-configuration-transactions.
#ifndef UETD_UMID_SETTINGS_H
#define UETD_UMID_SETTINGS_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/span.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Intended implementation: serialized task/main owner; no hidden heap.
 * Configurations, provider context and backing storage outlive the object.
 * Explicitly asynchronous operations retain buffers only after admission and
 * until terminal completion; synchronous spans are borrowed only for the call.
 * Schema IDs bind typed, versioned layouts/units; they do not permit arbitrary
 * native structs as a wire representation. Output/state commits and callback
 * context must be reviewed against the linked design before implementation.
 */
typedef enum { UMID_SETTINGS_BOOT = 0, UMID_SETTINGS_QUIESCENT, UMID_SETTINGS_LIVE } umid_settings_activation_t;
typedef struct { uetd_u32_t schema_id, schema_version; size_t maximum_bytes; umid_settings_activation_t activation; void* provider_context; } umid_settings_config_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } umid_settings_t;
typedef struct { uetd_u64_t id, expected_generation; uetd_u32_t schema_version; } umid_settings_transaction_t;

/// Resolve typed schema/defaults and validated persistent backend; do not conflate settings, calibration and secrets.
uetd_status_t umid_settings_init(umid_settings_t* settings, const umid_settings_config_t* config);
/// Copy bounded candidate data and guard against lost updates without changing active settings.
uetd_status_t umid_settings_stage(umid_settings_t* settings, uetd_u64_t expected_generation, ucore_read_span_t candidate, umid_settings_transaction_t* transaction);
/// Check cross-field/resource/authorization constraints and reserve dependent resources.
uetd_status_t umid_settings_validate(umid_settings_t* settings, umid_settings_transaction_t transaction);
/// Persist if required and activate at the declared boundary; preserve last valid state or enter explicit recovery.
uetd_status_t umid_settings_commit(umid_settings_t* settings, umid_settings_transaction_t transaction);
/// Release staged resources without corrupting the active generation.
uetd_status_t umid_settings_abort(umid_settings_t* settings, umid_settings_transaction_t transaction);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMID_SETTINGS_H */
