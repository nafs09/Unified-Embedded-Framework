/// @file src/umath/lie/so2.c
/// @brief Fail-closed outlines for SO(2) operations.
#include <uef/umath/lie/so2.h>

umath_status_t umath_so2_exp(umath_scalar_t angle_radians,
                             umath_scalar_t rotation_row_major[4])
{
    /* TODO(UMATH-SO2): Use configured scalar precision and document active
     * rotation, row-major storage, and angle wrapping at the branch cut. */
    (void)angle_radians; (void)rotation_row_major;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_so2_log(const umath_scalar_t rotation_row_major[4],
                             umath_scalar_t *angle_radians)
{
    /* TODO(UMATH-SO2): Reject non-rotations outside the named tolerance and
     * return the principal angle with deterministic boundary behavior. */
    (void)rotation_row_major; (void)angle_radians;
    return UMATH_NOT_IMPLEMENTED;
}