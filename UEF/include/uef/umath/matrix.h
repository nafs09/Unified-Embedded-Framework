/// @file include/uef/umath/matrix.h
/// @brief Row-major dense matrix and caller-owned vector kernels.
#ifndef UEF_UMATH_MATRIX_H
#define UEF_UMATH_MATRIX_H
#include <uef/umath/config.h>

#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// All matrix buffers are contiguous row-major arrays. Dimensions describe valid storage;
/// UMATH never allocates memory. Input/output ranges must not overlap unless an API says in-place.
/// On a numeric failure, an output buffer may contain a partial result and must be discarded.
umath_status_t umath_matrix_vector(const umath_scalar_t *matrix,
                                   const umath_scalar_t *vector,
                                   size_t rows, size_t columns,
                                   umath_scalar_t *output);
umath_status_t umath_matrix_multiply(const umath_scalar_t *left,
                                     const umath_scalar_t *right,
                                     size_t rows, size_t inner, size_t columns,
                                     umath_scalar_t *output);
umath_status_t umath_matrix_transpose(const umath_scalar_t *input,
                                      size_t rows, size_t columns,
                                      umath_scalar_t *output);
umath_status_t umath_matrix_add(const umath_scalar_t *left,
                                const umath_scalar_t *right,
                                size_t rows, size_t columns,
                                umath_scalar_t *output);
umath_status_t umath_matrix_sub(const umath_scalar_t *left,
                                const umath_scalar_t *right,
                                size_t rows, size_t columns,
                                umath_scalar_t *output);
umath_status_t umath_matrix_scale(const umath_scalar_t *input,
                                  size_t rows, size_t columns,
                                  umath_scalar_t scale,
                                  umath_scalar_t *output);
umath_status_t umath_matrix_symmetrize(umath_scalar_t *matrix,
                                       size_t dimension);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_MATRIX_H */