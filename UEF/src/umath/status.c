/// @file src/umath/status.c
/// @brief Human-readable names for UMATH return values.
#include <uef/umath/status.h>

const char *umath_status_string(umath_status_t status)
{
    switch (status) {
    case UMATH_OK: return "ok";
    case UMATH_INVALID_ARGUMENT: return "invalid argument";
    case UMATH_DIMENSION_MISMATCH: return "dimension mismatch";
    case UMATH_NUMERIC_FAILURE: return "numeric failure";
    case UMATH_SINGULAR: return "singular";
    case UMATH_NOT_POSITIVE_DEFINITE: return "not positive definite";
    case UMATH_NO_CONVERGENCE: return "no convergence";
    case UMATH_NOT_IMPLEMENTED: return "not implemented";
    default: return "unknown UMATH status";
    }
}