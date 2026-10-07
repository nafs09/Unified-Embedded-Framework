/// @file include/uef/umath/future/matrix/sparse_csr.h
/// @brief Planned allocation-free compressed-sparse-row operations.
#ifndef UEF_UMATH_FUTURE_MATRIX_SPARSE_CSR_H
#define UEF_UMATH_FUTURE_MATRIX_SPARSE_CSR_H
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    size_t rows, columns, nonzeros;
    const size_t *row_offsets;
    const size_t *column_indices;
    const umath_scalar_t *values;
} umath_csr_matrix_t;
/// TODO(UMATH-SPARSE): Validate all pointer lengths, monotonic row offsets, terminal nonzero count, in-range columns, and finite values.
umath_status_t umath_csr_validate(const umath_csr_matrix_t *matrix);
/// TODO(UMATH-SPARSE): Define aliasing, zero-row behavior, and output behavior on invalid sparse structure or numeric overflow.
umath_status_t umath_csr_matvec(const umath_csr_matrix_t *matrix,
                                const umath_scalar_t *vector,
                                umath_scalar_t *output);
/// TODO(UMATH-SPARSE): Define sorted-index/duplicate policy, caller workspace sizing, accumulation order, and deterministic complexity.
umath_status_t umath_csr_add(const umath_csr_matrix_t *left,
                             const umath_csr_matrix_t *right,
                             size_t *row_offsets, size_t row_offset_capacity,
                             size_t *column_indices, umath_scalar_t *values,
                             size_t nonzero_capacity, size_t *nonzeros_written);
#ifdef __cplusplus
}
#endif
#endif