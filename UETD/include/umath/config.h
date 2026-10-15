/// @file include/umath/config.h
/// @brief Project-wide UMATH scalar and intermediate type configuration.
#ifndef UETD_UMATH_CONFIG_H
#define UETD_UMATH_CONFIG_H

#include <ucore/types.h>

/* Keep UMATH policy out of the foundational UCORE scalar aliases. These macros
 * must be identical in the UETD library and every translation unit that consumes
 * its public UMATH or UCON types. CMake publishes the selected definitions. */
#ifndef UMATH_SCALAR_TYPE
#define UMATH_SCALAR_TYPE uetd_f32_t
#endif
typedef UMATH_SCALAR_TYPE umath_scalar_t;

/* Accumulation defaults to the public scalar to avoid hidden software-double
 * work on single-precision targets. Projects can select uetd_f64_t when wider
 * intermediates are worth their execution cost. */
#ifndef UMATH_ACCUMULATOR_TYPE
#define UMATH_ACCUMULATOR_TYPE UMATH_SCALAR_TYPE
#endif
typedef UMATH_ACCUMULATOR_TYPE umath_accumulator_t;

/* Cast constants before arithmetic so a generic kernel uses the configured
 * precision instead of promoting float expressions through a double literal. */
#define UMATH_SCALAR_C(value) ((umath_scalar_t)(value))
#define UMATH_ACCUMULATOR_C(value) ((umath_accumulator_t)(value))

#endif /* UETD_UMATH_CONFIG_H */
