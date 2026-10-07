/// @file include/uef/ucon/ucon_types.h
/// @brief UCON-facing names for scalar values shared with UMATH.
#ifndef UEF_UCON_TYPES_H
#define UEF_UCON_TYPES_H

#include <uef/umath/scalar.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Compatibility spelling for algorithm signatures; storage and configuration belong to UMATH.
typedef umath_scalar_t ucon_scalar_t;

#ifdef __cplusplus
}
#endif
#endif /* UEF_UCON_TYPES_H */
