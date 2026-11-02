/// @file include/uproto/network/connection_manager.h
/// @brief First-pass scaffold: Bounded interface reconnection and session invalidation.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/CONNECTIVITY_SECURITY.md#network-and-connection-manager.
#ifndef UETD_UPROTO_CONNECTION_MANAGER_H
#define UETD_UPROTO_CONNECTION_MANAGER_H

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
typedef enum { UPROTO_CONNECTION_DOWN = 0, UPROTO_CONNECTION_LINK, UPROTO_CONNECTION_ADDRESS, UPROTO_CONNECTION_RESOLVING, UPROTO_CONNECTION_CONNECTING, UPROTO_CONNECTION_SECURED, UPROTO_CONNECTION_READY, UPROTO_CONNECTION_FAILED } uproto_connection_state_t;
typedef struct { uetd_u32_t maximum_attempts, backoff_initial_ms, backoff_maximum_ms; void* provider_context; } uproto_connection_config_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } uproto_connection_manager_t;
typedef struct { uproto_connection_state_t state; uetd_u64_t generation; uetd_u32_t interface_id, attempts; uetd_status_t last_error; } uproto_connection_status_t;

/// Validate fixed interface/session limits, finite retry/backoff and provider ownership.
uetd_status_t uproto_connection_init(uproto_connection_manager_t* manager, const uproto_connection_config_t* config);
/// Bound interface/address/DNS/security transitions and invalidate dependent generations on loss.
uetd_status_t uproto_connection_poll(uproto_connection_manager_t* manager, ucore_clock_stamp_t now, uetd_u16_t work_budget);
/// Distinguish link/address/authentication/service readiness and return coherent generation metadata.
uetd_status_t uproto_connection_get_status(const uproto_connection_manager_t* manager, uproto_connection_status_t* status);
/// Cancel/drain outstanding operations without releasing borrowed buffers prematurely or replaying commands.
uetd_status_t uproto_connection_stop(uproto_connection_manager_t* manager);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPROTO_CONNECTION_MANAGER_H */
