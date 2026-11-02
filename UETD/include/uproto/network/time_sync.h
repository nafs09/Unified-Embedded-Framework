/// @file include/uproto/network/time_sync.h
/// @brief First-pass scaffold: Explicit synchronized-clock mapping and uncertainty.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/CONNECTIVITY_SECURITY.md#first-pass-application-protocols.
#ifndef UETD_UPROTO_TIME_SYNC_H
#define UETD_UPROTO_TIME_SYNC_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/deadline.h"

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
typedef enum { UPROTO_TIME_SNTP = 0, UPROTO_TIME_PTP, UPROTO_TIME_GNSS, UPROTO_TIME_EXTERNAL } uproto_time_source_t;
typedef struct { uproto_time_source_t source; uetd_u32_t maximum_age_us; void* provider_context; bool authenticated_source_required; } uproto_time_sync_config_t;
typedef struct { ucore_clock_stamp_t local; uetd_i64_t remote_time_us; uetd_u64_t uncertainty_us; uetd_u32_t source_generation; bool valid, holdover; } uproto_time_mapping_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } uproto_time_sync_t;

/// Select one implemented provider profile, timestamp capability, trust and holdover policy.
uetd_status_t uproto_time_sync_init(uproto_time_sync_t* sync, const uproto_time_sync_config_t* config);
/// Bound exchange/filter work and track source changes, delay asymmetry and measurement uncertainty.
uetd_status_t uproto_time_sync_poll(uproto_time_sync_t* sync, uetd_u16_t work_budget);
/// Publish local-to-remote mapping quality/age without stepping the monotonic deadline clock.
uetd_status_t uproto_time_sync_mapping(const uproto_time_sync_t* sync, uproto_time_mapping_t* mapping);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPROTO_TIME_SYNC_H */
