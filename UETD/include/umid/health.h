/// @file include/umid/health.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UMID_HEALTH_H
#define UMID_HEALTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

typedef enum {
    UMID_HEALTH_OK = 0,
    UMID_HEALTH_WARN = 1,
    UMID_HEALTH_FAULT = 2,
    UMID_HEALTH_FAIL = 3,
} umid_health_status_t;

typedef struct {
    const char* name;
    umid_health_status_t status;
    uetd_u32_t fault_code;
    uetd_u64_t last_update_us;
    bool reported; /* Registration is not evidence of a successful health check. */
} umid_health_entry_t;

/* Fixed capacity keeps registration deterministic and allocation-free. Override
 * at compile time for generated systems with more than 32 monitored services.
 * IDs are u8 with 0xff reserved as invalid, so the capacity cannot exceed 255.
 */
#ifndef UMID_HEALTH_MAX_ENTRIES
#define UMID_HEALTH_MAX_ENTRIES 32u
#endif
#if UMID_HEALTH_MAX_ENTRIES < 1 || UMID_HEALTH_MAX_ENTRIES > 255
#error "UMID_HEALTH_MAX_ENTRIES must be in [1, 255]"
#endif
#define UMID_HEALTH_INVALID_ID 0xffu

/* Register a subsystem for health monitoring */
/// Register a subsystem for health monitoring.
uetd_u8_t umid_health_register(const char* name);
/// Publish health status for a registered subsystem.
void umid_health_report(uetd_u8_t id, umid_health_status_t s, uetd_u32_t fault_code);

/// Return the aggregate health status of registered subsystems.
umid_health_status_t umid_health_system_status(void);
/* The borrowed pointer is only safe when updates are externally serialized. */
/// Return a borrowed pointer to the registered health entry.
const umid_health_entry_t* umid_health_entry(uetd_u8_t id);
/* Copy one coherent entry while holding the registry critical section. */
/// Copy a coherent health entry from the registry.
bool umid_health_entry_copy(uetd_u8_t id, umid_health_entry_t* out);
/// Return the number of registered health entries.
uetd_u8_t umid_health_count(void);

/* Return true only when the ID exists, has actually reported, and its report is no older than
 * maximum_age_us in the same monotonic-clock domain. A timestamp from the
 * future is treated as stale (for example after a clock-domain reset).
 */
/// Return whether a health entry has reported within its freshness interval.
bool umid_health_entry_is_fresh(uetd_u8_t id, uetd_u64_t now_us, uetd_u64_t maximum_age_us);

#ifdef __cplusplus
}
#endif

#endif /* UMID_HEALTH_H */
