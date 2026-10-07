/// @file include/uef/ucon/ucon_status.h
/// @brief Status values used by UCON control and estimation algorithms.

#ifndef UEF_UCON_STATUS_H
#define UEF_UCON_STATUS_H

#include <uef/ucon/ucon_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UCON_OK = 0,
    UCON_INVALID_ARGUMENT,
    UCON_NUMERIC_FAILURE,
    UCON_SOLVER_FAILURE,
    UCON_INFEASIBLE,
    UCON_NO_FEASIBLE_CANDIDATE,
    UCON_MEASUREMENT_REJECTED,
    UCON_ITERATION_LIMIT,
    /* Returned by development skeletons until their algorithm contract is implemented. */
    UCON_NOT_IMPLEMENTED
} ucon_status_t;

/// Return a stable, human-readable name for a UCON status value.
const char *ucon_status_string(ucon_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* UEF_UCON_STATUS_H */
