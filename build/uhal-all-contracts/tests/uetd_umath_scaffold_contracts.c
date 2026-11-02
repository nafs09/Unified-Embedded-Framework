#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <umath/derivatives/jacobian.h>
#include <umath/functions.h>
#include <umath/lie/se2.h>
#include <umath/lie/se3.h>
#include <umath/lie/so2.h>
#include <umath/lie/so3.h>
#include <umath/matrix/ldlt.h>
#include <umath/matrix/lu.h>
#include <umath/matrix/qr.h>
#include <umath/matrix/sparse_csr.h>
#include <umath/matrix/svd.h>
#include <umath/matrix/symmetric_eigen.h>
#include <umath/numerical/integration.h>
#include <umath/numerical/least_squares.h>
#include <umath/numerical/splines.h>
#include <umath/polynomial/roots.h>
#include <umath/probability/distributions.h>
#include <umath/probability/multivariate_normal.h>
#include <umath/probability/prng.h>
#include <umath/probability/streaming.h>
#include <umath/signal/fft.h>
#include <umath/signal/windows.h>

static union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;
static unsigned char before[sizeof(workspace.bytes)];

int main(void) {
    size_t failures = 0u;
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_jacobian_workspace_count((size_t){0}, (size_t){0}, (umath_jacobian_scheme_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_jacobian_workspace_count: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_jacobian_workspace_count: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_jacobian_finite_difference((umath_vector_function_fn){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), (size_t){0}, (umath_scalar_t){0}, (umath_jacobian_scheme_t){0}, ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_jacobian_finite_difference: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_jacobian_finite_difference: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_erf((umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_erf: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_erf: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_log_gamma((umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_log_gamma: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_log_gamma: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_bessel_i0((umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_bessel_i0: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_bessel_i0: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_se2_exp(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_se2_exp: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_se2_exp: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_se2_log(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_se2_log: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_se2_log: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_se2_adjoint(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_se2_adjoint: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_se2_adjoint: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_se3_exp(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_se3_exp: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_se3_exp: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_se3_log(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_se3_log: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_se3_log: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_se3_adjoint(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_se3_adjoint: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_se3_adjoint: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_se3_left_jacobian(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_se3_left_jacobian: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_se3_left_jacobian: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_se3_left_jacobian_inverse(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_se3_left_jacobian_inverse: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_se3_left_jacobian_inverse: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so2_exp((umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so2_exp: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so2_exp: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so2_log(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so2_log: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so2_log: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so3_hat((umath_vec3_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so3_hat: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so3_hat: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so3_vee(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so3_vee: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so3_vee: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so3_exp((umath_vec3_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so3_exp: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so3_exp: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so3_log((umath_quaternion_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so3_log: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so3_log: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so3_left_jacobian((umath_vec3_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so3_left_jacobian: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so3_left_jacobian: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so3_left_jacobian_inverse((umath_vec3_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so3_left_jacobian_inverse: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so3_left_jacobian_inverse: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so3_right_jacobian((umath_vec3_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so3_right_jacobian: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so3_right_jacobian: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_so3_right_jacobian_inverse((umath_vec3_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_so3_right_jacobian_inverse: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_so3_right_jacobian_inverse: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_ldlt_factor(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_ldlt_factor: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_ldlt_factor: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_ldlt_solve(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_ldlt_solve: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_ldlt_solve: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_lu_factor(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_lu_factor: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_lu_factor: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_lu_solve(((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_lu_solve: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_lu_solve: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_lu_determinant(((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, (int){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_lu_determinant: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_lu_determinant: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_qr_factor(((void*)workspace.bytes), (size_t){0}, (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_qr_factor: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_qr_factor: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_qr_least_squares(((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, (size_t){0}, ((void*)workspace.bytes), (umath_scalar_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_qr_least_squares: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_qr_least_squares: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_csr_validate(((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_csr_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_csr_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_csr_matvec(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_csr_matvec: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_csr_matvec: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_csr_add(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_csr_add: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_csr_add: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_svd(((void*)workspace.bytes), (size_t){0}, (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_svd: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_svd: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_svd_solve(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, (size_t){0}, ((void*)workspace.bytes), (umath_scalar_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_svd_solve: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_svd_solve: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_matrix_symmetric_eigen(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_matrix_symmetric_eigen: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_matrix_symmetric_eigen: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_integrate_trapezoid(((void*)workspace.bytes), (size_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_integrate_trapezoid: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_integrate_trapezoid: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_integrate_simpson(((void*)workspace.bytes), (size_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_integrate_simpson: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_integrate_simpson: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_ode_rk4_step((umath_ode_rhs_fn){0}, ((void*)workspace.bytes), (umath_scalar_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_ode_rk4_step: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_ode_rk4_step: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_ode_dopri5_step((umath_ode_rhs_fn){0}, ((void*)workspace.bytes), (umath_scalar_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes), (size_t){0}, (umath_scalar_t){0}, (umath_scalar_t){0}, (size_t){0}, ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_ode_dopri5_step: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_ode_dopri5_step: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_least_squares_solve(((void*)workspace.bytes), (size_t){0}, (size_t){0}, ((void*)workspace.bytes), (umath_scalar_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_least_squares_solve: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_least_squares_solve: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_weighted_least_squares_solve(((void*)workspace.bytes), (size_t){0}, (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), (umath_scalar_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_weighted_least_squares_solve: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_weighted_least_squares_solve: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_residual_norm(((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_residual_norm: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_residual_norm: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_cubic_hermite((umath_scalar_t){0}, (umath_scalar_t){0}, (umath_scalar_t){0}, (umath_scalar_t){0}, (umath_scalar_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_cubic_hermite: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_cubic_hermite: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_bezier_evaluate(((void*)workspace.bytes), (size_t){0}, (size_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_bezier_evaluate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_bezier_evaluate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_bspline_basis((size_t){0}, (size_t){0}, ((void*)workspace.bytes), (size_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_bspline_basis: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_bspline_basis: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_bspline_derivative((size_t){0}, (size_t){0}, ((void*)workspace.bytes), (size_t){0}, (umath_scalar_t){0}, (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_bspline_derivative: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_bspline_derivative: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_poly_roots_real(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_poly_roots_real: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_poly_roots_real: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_poly_roots_complex(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_poly_roots_complex: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_poly_roots_complex: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_logsumexp(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_logsumexp: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_logsumexp: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_normal_logpdf((umath_scalar_t){0}, (umath_scalar_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_normal_logpdf: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_normal_logpdf: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_normal_cdf((umath_scalar_t){0}, (umath_scalar_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_normal_cdf: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_normal_cdf: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_chi_square_cdf((umath_scalar_t){0}, (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_chi_square_cdf: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_chi_square_cdf: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_softmax(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_softmax: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_softmax: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_log_softmax(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_log_softmax: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_log_softmax: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_multivariate_normal_logpdf_cholesky(((void*)workspace.bytes), ((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_multivariate_normal_logpdf_cholesky: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_multivariate_normal_logpdf_cholesky: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_prng_seed(((void*)workspace.bytes), (uetd_u64_t){0}, (uetd_u64_t){0}) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_prng_seed: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_prng_seed: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_prng_u32(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_prng_u32: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_prng_u32: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_prng_uniform(((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_prng_uniform: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_prng_uniform: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_prng_normal(((void*)workspace.bytes), (umath_scalar_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_prng_normal: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_prng_normal: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_stats_mean(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_stats_mean: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_stats_mean: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_stats_variance(((void*)workspace.bytes), (size_t){0}, (bool){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_stats_variance: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_stats_variance: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_stats_covariance(((void*)workspace.bytes), (size_t){0}, (size_t){0}, (bool){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_stats_covariance: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_stats_covariance: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_stats_quantile(((void*)workspace.bytes), (size_t){0}, (umath_scalar_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_stats_quantile: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_stats_quantile: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_stats_standard_deviation(((void*)workspace.bytes), (size_t){0}, (bool){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_stats_standard_deviation: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_stats_standard_deviation: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_stats_rms(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_stats_rms: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_stats_rms: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_fft_plan_init(((void*)workspace.bytes), (size_t){0}, ((void*)workspace.bytes), (size_t){0}) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_fft_plan_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_fft_plan_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_fft_execute(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (bool){0}) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_fft_execute: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_fft_execute: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_fft_real_forward(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_fft_real_forward: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_fft_real_forward: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_window_hann(((void*)workspace.bytes), (size_t){0}, (bool){0}) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_window_hann: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_window_hann: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_window_hamming(((void*)workspace.bytes), (size_t){0}, (bool){0}) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_window_hamming: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_window_hamming: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_window_blackman(((void*)workspace.bytes), (size_t){0}, (bool){0}) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_window_blackman: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_window_blackman: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umath_window_kaiser(((void*)workspace.bytes), (size_t){0}, (umath_scalar_t){0}, (bool){0}) != UMATH_NOT_IMPLEMENTED) {
        printf("FAIL umath_window_kaiser: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umath_window_kaiser: changed caller-owned storage\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu uetd_umath_scaffold_contracts check(s) failed across 73 operations\n", failures);
        return 1;
    }
    printf("Validated 73 uetd_umath_scaffold_contracts operations from 22 source files\n");
    return 0;
}
