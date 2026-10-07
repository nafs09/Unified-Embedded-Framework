/// @file include/uef/umath/vector.h
/// @brief Allocation-free operations on caller-owned contiguous vectors.
#ifndef UEF_UMATH_VECTOR_H
#define UEF_UMATH_VECTOR_H

#include <stddef.h>
#include <uef/umath/status.h>
#include <uef/ucore/uef_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Clamp each of count finite inputs. Input and output ranges must not overlap.
umath_status_t umath_vector_clamp(const umath_scalar_t *input,
                                  size_t count,
                                  umath_scalar_t minimum,
                                  umath_scalar_t maximum,
                                  umath_scalar_t *output);
/// Dot product. Result must not alias either input range.
umath_status_t umath_vector_dot(const umath_scalar_t *left,
                                const umath_scalar_t *right,
                                size_t count,
                                umath_scalar_t *result);
/// Euclidean norm. Result must not alias the input range.
umath_status_t umath_vector_norm(const umath_scalar_t *input,
                                 size_t count,
                                 umath_scalar_t *result);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_VECTOR_H */