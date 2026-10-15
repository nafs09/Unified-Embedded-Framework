/// @file include/umath/vector/vector.h
/// @brief Allocation-free operations on caller-owned contiguous vectors.
#ifndef UETD_UMATH_VECTOR_H
#define UETD_UMATH_VECTOR_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Generic normalize, distance, and interpolation preflight their results before writing output.
/// Clamp each of count finite inputs. Input and output ranges must not overlap.
umath_status_t umath_vector_clamp(const umath_scalar_t* input, size_t count, umath_scalar_t minimum, umath_scalar_t maximum,
                                  umath_scalar_t* output);
/// Elementwise arithmetic. Input and output ranges must not overlap; output is unchanged on failure.
umath_status_t umath_vector_add(const umath_scalar_t* left, const umath_scalar_t* right, size_t count, umath_scalar_t* output);
umath_status_t umath_vector_sub(const umath_scalar_t* left, const umath_scalar_t* right, size_t count, umath_scalar_t* output);
umath_status_t umath_vector_mul(const umath_scalar_t* left, const umath_scalar_t* right, size_t count, umath_scalar_t* output);
umath_status_t umath_vector_div(const umath_scalar_t* numerator, const umath_scalar_t* denominator, size_t count,
                                umath_scalar_t* quotient);
umath_status_t umath_vector_scale(const umath_scalar_t* input, size_t count, umath_scalar_t scale, umath_scalar_t* output);
/// Dot product. Result must not alias either input range.
umath_status_t umath_vector_dot(const umath_scalar_t* left, const umath_scalar_t* right, size_t count, umath_scalar_t* result);
/// Euclidean norm. Result must not alias the input range.
umath_status_t umath_vector_norm(const umath_scalar_t* input, size_t count, umath_scalar_t* result);
/// Generic vector geometry operations. Input/output ranges may be exactly aliased, but partial overlap is unsupported.
umath_status_t umath_vector_normalize(const umath_scalar_t* input, size_t count, umath_scalar_t* output);
umath_status_t umath_vector_distance(const umath_scalar_t* left, const umath_scalar_t* right, size_t count,
                                     umath_scalar_t* distance);
umath_status_t umath_vector_lerp(const umath_scalar_t* start, const umath_scalar_t* end, size_t count, umath_scalar_t t,
                                 umath_scalar_t* output);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_VECTOR_H */
