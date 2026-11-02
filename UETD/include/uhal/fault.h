/// @file include/uhal/fault.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UHAL_FAULT_H
#define UHAL_FAULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"
#include "uhal/target.h"

/* Fixed-width addresses also represent program addresses wider than a data
 * pointer on Harvard MCUs. The platform defines their address-space meaning. */
typedef struct {
    uetd_u64_t pc;
    uetd_u64_t sp;
    uetd_u64_t fault_address;
    uetd_u32_t core_index;
    uetd_u32_t architecture; /* Platform-defined architecture identifier. */
    const void* registers; /* Optional architecture-specific snapshot. */
    uetd_u32_t registers_size; /* Bytes; valid only during the hook. */
} uhal_fault_context_t;

#define UHAL_FAULT_HARD (1u << 0)
#define UHAL_FAULT_MEM (1u << 1)
#define UHAL_FAULT_BUS (1u << 2)
#define UHAL_FAULT_USAGE (1u << 3)

/* Fatal platform hook; must not return. Persistence/reset policy belongs to the
 * platform/application. SDK-owned panic handlers are not replaced implicitly. */
UETD_WEAK
/// Handle a processor fault using the captured fault context.
void uhal_fault_hook(const uhal_fault_context_t* ctx, uetd_u32_t fault_type);


#ifdef __cplusplus
}
#endif

#endif /* UHAL_FAULT_H */
