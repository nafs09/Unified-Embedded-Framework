/// @file include/upal/power.h
/// @brief First-pass scaffold: Power-state admission, wake and battery facts.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/FOUNDATIONS.md#power-and-clock-management.
#ifndef UETD_UPAL_POWER_H
#define UETD_UPAL_POWER_H

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
typedef enum { UPAL_POWER_RUN = 0, UPAL_POWER_IDLE, UPAL_POWER_SLEEP, UPAL_POWER_DEEP_SLEEP } upal_power_mode_t;
typedef struct { uetd_u32_t id; uetd_u32_t generation; } upal_power_lease_t;
typedef struct { uetd_u32_t maximum_leases; uetd_u32_t wake_latency_us; void* provider_context; } upal_power_config_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } upal_power_t;
typedef struct { ucore_clock_stamp_t captured; uetd_u32_t voltage_mv; uetd_i32_t current_ma; uetd_u16_t charge_permille; uetd_u32_t valid_fields; bool external_supply; bool charging; bool brownout; } upal_power_status_t;

/// Bind exact retention/wake/timer/rail facts and bounded lease storage.
uetd_status_t upal_power_init(upal_power_t* power, const upal_power_config_t* config);
/// Reserve a generation-tagged veto before a transfer/session starts; fail explicitly on exhaustion.
uetd_status_t upal_power_acquire(upal_power_t* power, upal_power_mode_t deepest_allowed, upal_power_lease_t* lease);
/// Reject stale/double release without dropping another owner power veto.
uetd_status_t upal_power_release(upal_power_t* power, upal_power_lease_t lease);
/// Validate outstanding DMA/session leases, wake latency, retained state and clock continuity before sleep.
uetd_status_t upal_power_enter(upal_power_t* power, upal_power_mode_t mode, ucore_deadline_t next_wake);
/// Report measured validity and timestamp; never fabricate battery charge or power-good values.
uetd_status_t upal_power_get_status(upal_power_t* power, upal_power_status_t* status);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPAL_POWER_H */
