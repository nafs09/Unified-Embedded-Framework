/// @file include/umath/status.h
/// @brief Error values shared by allocation-free UMATH operations.
#ifndef UETD_UMATH_STATUS_H
#define UETD_UMATH_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UMATH_OK = 0,
    UMATH_INVALID_ARGUMENT,
    UMATH_DIMENSION_MISMATCH,
    UMATH_NUMERIC_FAILURE,
    UMATH_SINGULAR,
    UMATH_NOT_POSITIVE_DEFINITE,
    UMATH_NO_CONVERGENCE,
    UMATH_NOT_IMPLEMENTED
} umath_status_t;

/// Return the stable printable name for a UMATH status.
const char* umath_status_string(umath_status_t status);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_STATUS_H */
