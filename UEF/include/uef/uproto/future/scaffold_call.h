/// @file include/uef/uproto/future/scaffold_call.h
/// @brief Type-erased development envelope for unregistered UPROTO proposals.
///
/// This is a work-planning boundary only. It is not a stable protocol API and
/// is not included by uproto.h, uef_modules protocol_provides, or the ControlIR
/// dispatch manifest. Replace it with typed fixed-size state/configuration
/// after the protocol profile and transport contract are approved.
#ifndef UPROTO_FUTURE_SCAFFOLD_CALL_H
#define UPROTO_FUTURE_SCAFFOLD_CALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_status.h"
#include "uef/ucore/uef_types.h"

typedef struct {
    void* instance;                 /* future typed protocol state, caller-owned */
    const void* config;             /* selected profile/configuration */
    const void* input;              /* bytes, frame, event, or payload */
    uef_u32_t input_size;           /* input span length in bytes/items */
    void* output;                   /* caller-owned result/frame storage */
    uef_u32_t output_capacity;      /* output span capacity */
    uef_u32_t output_size;          /* set only by a reviewed implementation */
    void* transport_context;        /* UPAL handle or selected external stack */
} uproto_future_call_t;

/* Fail closed without changing call storage until a typed contract is defined. */
static inline uef_status_t uproto_future_scaffold_result(
    const uproto_future_call_t* call) {
    return call != NULL ? UEF_NOT_SUPPORTED : UEF_INVALID_ARG;
}

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_SCAFFOLD_CALL_H */
