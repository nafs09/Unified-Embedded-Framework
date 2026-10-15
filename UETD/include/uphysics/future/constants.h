/// @file include/uphysics/future/constants.h
/// @brief Future SI physical-constant catalogue; not available for generation.
#ifndef UETD_UPHYSICS_FUTURE_CONSTANTS_H
#define UETD_UPHYSICS_FUTURE_CONSTANTS_H

#include <umath/config.h>
#include <umath/constants.h>

/* Exact numerical values of the seven SI defining constants (BIPM SI).
 * Names state the associated SI units. These expressions use the configured
 * UMATH accumulator type; binary floating-point may round the exact values.
 * Source: https://www.bipm.org/en/measurement-units/si-defining-constants */
#define UPHYSICS_DELTA_NU_CS_HZ (UMATH_ACCUMULATOR_C(9192631770))
#define UPHYSICS_C_M_PER_S (UMATH_ACCUMULATOR_C(299792458))
#define UPHYSICS_H_J_S (UMATH_ACCUMULATOR_C(6.62607015e-34))
#define UPHYSICS_E_C (UMATH_ACCUMULATOR_C(1.602176634e-19))
#define UPHYSICS_K_B_J_PER_K (UMATH_ACCUMULATOR_C(1.380649e-23))
#define UPHYSICS_N_A_PER_MOL (UMATH_ACCUMULATOR_C(6.02214076e23))
#define UPHYSICS_K_CD_540_THZ_CD_SR_PER_W (UMATH_ACCUMULATOR_C(683))

/* Useful derived constants. Their values inherit the configured accumulator
 * precision; they are expressions so their definitions remain connected to
 * the SI defining constants and UMATH's canonical angular constants. */
#define UPHYSICS_HBAR_J_S (UPHYSICS_H_J_S / UMATH_TAU)
#define UPHYSICS_R_J_PER_MOL_K (UPHYSICS_N_A_PER_MOL * UPHYSICS_K_B_J_PER_K)
#define UPHYSICS_F_C_PER_MOL (UPHYSICS_N_A_PER_MOL * UPHYSICS_E_C)

/* Explicit scalar-precision forms for APIs that store umath_scalar_t. */
#define UPHYSICS_DELTA_NU_CS_HZ_SCALAR (UMATH_SCALAR_C(9192631770))
#define UPHYSICS_C_M_PER_S_SCALAR (UMATH_SCALAR_C(299792458))
#define UPHYSICS_H_J_S_SCALAR (UMATH_SCALAR_C(6.62607015e-34))
#define UPHYSICS_E_C_SCALAR (UMATH_SCALAR_C(1.602176634e-19))
#define UPHYSICS_K_B_J_PER_K_SCALAR (UMATH_SCALAR_C(1.380649e-23))
#define UPHYSICS_N_A_PER_MOL_SCALAR (UMATH_SCALAR_C(6.02214076e23))
#define UPHYSICS_K_CD_540_THZ_CD_SR_PER_W_SCALAR (UMATH_SCALAR_C(683))
#define UPHYSICS_HBAR_J_S_SCALAR (UPHYSICS_H_J_S_SCALAR / UMATH_TAU_SCALAR)
#define UPHYSICS_R_J_PER_MOL_K_SCALAR (UPHYSICS_N_A_PER_MOL_SCALAR * UPHYSICS_K_B_J_PER_K_SCALAR)
#define UPHYSICS_F_C_PER_MOL_SCALAR (UPHYSICS_N_A_PER_MOL_SCALAR * UPHYSICS_E_C_SCALAR)

#endif /* UETD_UPHYSICS_FUTURE_CONSTANTS_H */
