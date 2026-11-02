/// @file include/uactuator/actuator.h
/// @brief First-pass scaffold: Typed actuator authority, expiry and safe output.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/PRODUCT_SERVICES.md#actuator-services-and-local-safety.
#ifndef UETD_UACTUATOR_ACTUATOR_H
#define UETD_UACTUATOR_ACTUATOR_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/span.h"
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
typedef enum { UACTUATOR_DIGITAL = 0, UACTUATOR_PWM, UACTUATOR_SERVO, UACTUATOR_STEP_DIRECTION, UACTUATOR_MOTOR, UACTUATOR_PROTOCOL } uactuator_class_t;
typedef struct { uactuator_class_t kind; uetd_u32_t schema_id, maximum_age_us; void* provider_context; } uactuator_config_t;
typedef struct { uetd_u64_t request_id, authority_generation; ucore_deadline_t expires; uetd_u32_t schema_id; ucore_read_span_t value; } uactuator_command_t;
typedef struct { uetd_u64_t request_id; uetd_u32_t quality, inhibit_causes; ucore_clock_stamp_t observed; bool armed, accepted, executing, completed; } uactuator_feedback_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } uactuator_t;

/// Resolve the class-specific typed schema, provider limits, safe level and independent hardware inhibit.
uetd_status_t uactuator_init(uactuator_t* actuator, const uactuator_config_t* config);
/// Validate local permissives, fresh feedback and authorization before allowing output.
uetd_status_t uactuator_arm(uactuator_t* actuator, uetd_u64_t authority_generation);
/// Check units/range, expiry, authority and monotonic request identity; admission does not imply execution.
uetd_status_t uactuator_submit(uactuator_t* actuator, const uactuator_command_t* command);
/// Latch the cause and drive the qualified safe-output path with a bounded latency.
uetd_status_t uactuator_inhibit(uactuator_t* actuator, uetd_u32_t causes);
/// Publish measured/coherent outcome and freshness rather than echoing the requested command.
uetd_status_t uactuator_feedback(const uactuator_t* actuator, uactuator_feedback_t* feedback);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UACTUATOR_ACTUATOR_H */
