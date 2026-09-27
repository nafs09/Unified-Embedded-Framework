/// @file include/uef/umid/umid_health.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UMID_HEALTH_H
#define UMID_HEALTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

typedef enum {
    UMID_HEALTH_OK      = 0,
    UMID_HEALTH_WARN    = 1,
    UMID_HEALTH_FAULT   = 2,
    UMID_HEALTH_FAIL    = 3,
} umid_health_status_t;

typedef struct {
    const char*          name;
    umid_health_status_t status;
    uef_u32_t            fault_code;
    uef_u64_t            last_update_us;
} umid_health_entry_t;

/* Fixed capacity keeps registration deterministic and allocation-free. */
#define UMID_HEALTH_MAX_ENTRIES 32u
#define UMID_HEALTH_INVALID_ID  0xffu

/* Register a subsystem for health monitoring */
uef_u8_t umid_health_register(const char* name);
void     umid_health_report(uef_u8_t id, umid_health_status_t s,
                              uef_u32_t fault_code);

umid_health_status_t umid_health_system_status(void);
const umid_health_entry_t* umid_health_entry(uef_u8_t id);
uef_u8_t umid_health_count(void);

#ifdef __cplusplus
}
#endif

#endif /* UMID_HEALTH_H */
