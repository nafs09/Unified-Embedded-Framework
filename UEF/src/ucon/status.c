/// @file src/ucon/status.c
/// @brief Stable status-name mapping for diagnostics and generated wrappers.

#include <uef/ucon/ucon_status.h>

const char *ucon_status_string(ucon_status_t status)
{
    switch (status) {
    case UCON_OK: return "ok";
    case UCON_INVALID_ARGUMENT: return "invalid_argument";
    case UCON_NUMERIC_FAILURE: return "numeric_failure";
    case UCON_SOLVER_FAILURE: return "solver_failure";
    case UCON_INFEASIBLE: return "infeasible";
    case UCON_NO_FEASIBLE_CANDIDATE: return "no_feasible_candidate";
    case UCON_MEASUREMENT_REJECTED: return "measurement_rejected";
    case UCON_ITERATION_LIMIT: return "iteration_limit";
    case UCON_NOT_IMPLEMENTED: return "not_implemented";
    default: return "unknown_status";
    }
}
