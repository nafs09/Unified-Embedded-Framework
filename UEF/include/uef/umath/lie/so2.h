/// @file include/uef/umath/lie/so2.h
/// @brief Planned SO(2) matrix exponential and principal logarithm.
#ifndef UEF_UMATH_LIE_SO2_H
#define UEF_UMATH_LIE_SO2_H
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-SO2): Emit the row-major active rotation matrix from a finite radian angle.
umath_status_t umath_so2_exp(umath_scalar_t angle_radians,
                             umath_scalar_t rotation_row_major[4]);
/// TODO(UMATH-SO2): Validate orthogonality/determinant tolerance and define the principal-angle branch interval.
umath_status_t umath_so2_log(const umath_scalar_t rotation_row_major[4],
                             umath_scalar_t *angle_radians);
#ifdef __cplusplus
}
#endif
#endif