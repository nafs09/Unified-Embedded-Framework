/// @file include/umath/matrix/core.h
/// @brief Row-major dense matrix and caller-owned vector kernels.
#ifndef UETD_UMATH_MATRIX_H
#define UETD_UMATH_MATRIX_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Matrix buffers are contiguous row-major arrays. UMATH never allocates memory.
/// Input, output, and workspace ranges must be disjoint unless an API says in-place. Some kernels may partially write on numeric failure.
/// Determinant, inverse, and exponential have no fixed dimension ceiling; their workspace count functions reject unrepresentable sizes.
/// Rank-one update modifies its matrix in place; its vector inputs must be disjoint from the matrix.
umath_status_t umath_matrix_vector(const umath_scalar_t* matrix, const umath_scalar_t* vector, size_t rows, size_t columns,
                                   umath_scalar_t* output);
umath_status_t umath_matrix_multiply(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t inner,
                                     size_t columns, umath_scalar_t* output);
umath_status_t umath_matrix_transpose(const umath_scalar_t* input, size_t rows, size_t columns, umath_scalar_t* output);
umath_status_t umath_matrix_add(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t columns,
                                umath_scalar_t* output);
umath_status_t umath_matrix_sub(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t columns,
                                umath_scalar_t* output);
umath_status_t umath_matrix_scale(const umath_scalar_t* input, size_t rows, size_t columns, umath_scalar_t scale,
                                  umath_scalar_t* output);
umath_status_t umath_matrix_symmetrize(umath_scalar_t* matrix, size_t dimension);
umath_status_t umath_matrix_identity(size_t dimension, umath_scalar_t* output);
umath_status_t umath_matrix_diagonal(const umath_scalar_t* diagonal, size_t dimension, umath_scalar_t* output);
umath_status_t umath_matrix_elementwise_mul(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t columns,
                                            umath_scalar_t* output);
umath_status_t umath_matrix_trace(const umath_scalar_t* matrix, size_t dimension, umath_scalar_t* trace);
umath_status_t umath_matrix_norm_frobenius(const umath_scalar_t* matrix, size_t rows, size_t columns, umath_scalar_t* norm);
umath_status_t umath_matrix_norm_one(const umath_scalar_t* matrix, size_t rows, size_t columns, umath_scalar_t* norm);
umath_status_t umath_matrix_norm_infinity(const umath_scalar_t* matrix, size_t rows, size_t columns, umath_scalar_t* norm);
/// Workspace count queries return umath_accumulator_t elements, not bytes. The caller owns allocation; the query guarantees
/// converting its result to bytes will not overflow size_t. Execution rejects undersized workspace; extra elements are accepted.
umath_status_t umath_matrix_determinant_workspace_count(size_t dimension, size_t* workspace_count);
umath_status_t umath_matrix_determinant(const umath_scalar_t* matrix, size_t dimension, umath_scalar_t* determinant,
                                        umath_accumulator_t* workspace, size_t workspace_count);
umath_status_t umath_matrix_inverse_workspace_count(size_t dimension, size_t* workspace_count);
umath_status_t umath_matrix_inverse(const umath_scalar_t* matrix, size_t dimension, umath_scalar_t* inverse,
                                    umath_accumulator_t* workspace, size_t workspace_count);
umath_status_t umath_matrix_outer_product(const umath_scalar_t* left, size_t rows, const umath_scalar_t* right, size_t columns,
                                          umath_scalar_t* output);
umath_status_t umath_matrix_rank_one_update(umath_scalar_t* matrix, size_t rows, size_t columns, umath_scalar_t alpha,
                                            const umath_scalar_t* left, const umath_scalar_t* right);
/// Matrix exponential uses scaling and squaring with a Taylor series and three dimension-by-dimension accumulator arrays.
umath_status_t umath_matrix_exponential_workspace_count(size_t dimension, size_t* workspace_count);
umath_status_t umath_matrix_exponential(const umath_scalar_t* matrix, size_t dimension, umath_scalar_t* exponential,
                                        umath_accumulator_t* workspace, size_t workspace_count);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_MATRIX_H */
