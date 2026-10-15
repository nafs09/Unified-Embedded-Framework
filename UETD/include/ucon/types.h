/// @file include/ucon/types.h
/// @brief UCON-facing names for scalar values shared with UMATH.
#ifndef UETD_UCON_TYPES_H
#define UETD_UCON_TYPES_H

#include <umath/scalar.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Compatibility spelling for algorithm signatures; storage and configuration belong to UMATH.
typedef umath_scalar_t ucon_scalar_t;

#ifdef __cplusplus
}
#endif
#endif /* UETD_UCON_TYPES_H */
