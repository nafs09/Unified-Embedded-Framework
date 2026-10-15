/// @file include/uproto/future/scaffold_call.h
/// @brief Type-erased development envelope for unregistered UPROTO proposals.
///
/// This is a work-planning boundary only. It is not a stable protocol API and
/// dispatch manifest. Replace it with typed fixed-size state/configuration
/// after the protocol profile and transport contract are approved.
#ifndef UPROTO_FUTURE_SCAFFOLD_CALL_H
#define UPROTO_FUTURE_SCAFFOLD_CALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

typedef struct {
    void* instance;            /* future typed protocol state, caller-owned */
    const void* config;        /* selected profile/configuration */
    const void* input;         /* bytes, frame, event, or payload */
    uetd_u32_t input_size;      /* input span length in bytes/items */
    void* output;              /* caller-owned result/frame storage */
    uetd_u32_t output_capacity; /* output span capacity */
    uetd_u32_t output_size;     /* set only by a reviewed implementation */
    void* transport_context;   /* UPAL handle or selected external stack */
} uproto_future_call_t;

/* Fail closed without changing call storage until a typed contract is defined. */
static inline uetd_status_t uproto_future_scaffold_result(const uproto_future_call_t* call) {
    return call != NULL ? UETD_NOT_SUPPORTED : UETD_INVALID_ARG;
}

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_SCAFFOLD_CALL_H */
