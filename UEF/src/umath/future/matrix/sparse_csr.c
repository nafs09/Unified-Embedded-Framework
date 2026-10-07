/// @file src/umath/future/matrix/sparse_csr.c
/// @brief Fail-closed outlines for CSR validation and kernels.
#include <uef/umath/future/matrix/sparse_csr.h>

umath_status_t umath_csr_validate(const umath_csr_matrix_t *matrix)
{
    /* TODO(UMATH-SPARSE): Check structural invariants before any index/value
     * access, then validate every column and finite scalar without allocation. */
    (void)matrix;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_csr_matvec(const umath_csr_matrix_t *matrix,
                                const umath_scalar_t *vector,
                                umath_scalar_t *output)
{
    /* TODO(UMATH-SPARSE): Validate first, then accumulate one bounded row at a
     * time with documented aliasing and overflow semantics. */
    (void)matrix; (void)vector; (void)output;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_csr_add(const umath_csr_matrix_t *left,
                             const umath_csr_matrix_t *right,
                             size_t *row_offsets, size_t row_offset_capacity,
                             size_t *column_indices, umath_scalar_t *values,
                             size_t nonzero_capacity, size_t *nonzeros_written)
{
    /* TODO(UMATH-SPARSE): Merge sorted row indices into caller storage, report
     * exact capacity before writing, and combine or reject duplicate entries. */
    (void)left; (void)right; (void)row_offsets; (void)row_offset_capacity;
    (void)column_indices; (void)values; (void)nonzero_capacity; (void)nonzeros_written;
    return UMATH_NOT_IMPLEMENTED;
}