/// @file include/umid/sensor.h
/// @brief First-pass scaffold: Shared sample quality, clock and calibration envelopes.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/PRODUCT_SERVICES.md#sensor-and-acquisition-services.
#ifndef UETD_UMID_SENSOR_H
#define UETD_UMID_SENSOR_H

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
typedef enum { UMID_SAMPLE_INVALID = 0, UMID_SAMPLE_VALID = 1u, UMID_SAMPLE_STALE = 2u, UMID_SAMPLE_OVERRUN = 4u, UMID_SAMPLE_UNCERTAIN_TIME = 8u } umid_sample_quality_t;
typedef struct { ucore_clock_stamp_t captured; uetd_u64_t sequence; uetd_u32_t quality, calibration_generation, schema_id, frame_id; uetd_u32_t uncertainty_us; } umid_sample_metadata_t;
typedef struct { uetd_u32_t schema_id, frame_id, maximum_age_us, calibration_generation; size_t value_bytes; void* provider_context; } umid_sensor_config_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } umid_sensor_t;

/// Bind device/provider identity, typed value schema, units/frame and sample/calibration contract.
uetd_status_t umid_sensor_init(umid_sensor_t* sensor, const umid_sensor_config_t* config);
/// Bound acquisition/recovery work and preserve capture time rather than replacing it with parse time.
uetd_status_t umid_sensor_poll(umid_sensor_t* sensor, uetd_u16_t work_budget);
/// Publish the complete value and metadata together; distinguish stale, missing, invalid and overrun samples.
uetd_status_t umid_sensor_read(umid_sensor_t* sensor, ucore_write_span_t value, umid_sample_metadata_t* metadata);
/// Validate compatible device/schema calibration and activate it only at the declared sample boundary.
uetd_status_t umid_sensor_activate_calibration(umid_sensor_t* sensor, uetd_u32_t generation);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMID_SENSOR_H */
