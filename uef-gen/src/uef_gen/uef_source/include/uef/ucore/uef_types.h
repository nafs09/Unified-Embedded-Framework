/// @file include/uef/ucore/uef_types.h
/// @brief Scalar type aliases. All UEF code uses these; never float or int directly in portable code.

#ifndef UEF_TYPES_H
#define UEF_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

/* Keep alignment spelling portable between the C11 core and C++ consumers. */
#ifdef __cplusplus
#  define UEF_ALIGNAS(bytes) alignas(bytes)
#else
#  define UEF_ALIGNAS(bytes) _Alignas(bytes)
#endif

/* Keep function attributes usable from the C core and C++ consumers. */
#if defined(__GNUC__) || defined(__clang__)
#  define UEF_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#  define UEF_NORETURN __declspec(noreturn)
#else
#  define UEF_NORETURN
#endif

/* Floating point */
typedef float    uef_f32_t;
typedef double   uef_f64_t;

/* Fixed point */
typedef int16_t  uef_q15_t;   /* Q1.15 */
typedef int32_t  uef_q31_t;   /* Q1.31 */
typedef int64_t  uef_q63_t;   /* Q1.63 */
typedef int32_t  uef_q16_t;   /* Q15.16 general-purpose */

/* Standard integers */
typedef uint8_t  uef_u8_t;
typedef uint16_t uef_u16_t;
typedef uint32_t uef_u32_t;
typedef uint64_t uef_u64_t;
typedef int8_t   uef_i8_t;
typedef int16_t  uef_i16_t;
typedef int32_t  uef_i32_t;
typedef int64_t  uef_i64_t;

/* UCON scalar type — set per project by uef-gen */
#ifndef UCON_SCALAR_TYPE
#  define UCON_SCALAR_TYPE  uef_f32_t
#endif
typedef UCON_SCALAR_TYPE ucon_scalar_t;

/* Q16.16 arithmetic */
#define UEF_Q16_FRAC_BITS  16
#define UEF_Q16_ONE        (1 << UEF_Q16_FRAC_BITS)
#define UEF_Q16_FROM_F(x)  ((uef_q16_t)((x) * UEF_Q16_ONE))
#define UEF_Q16_TO_F(x)    ((uef_f32_t)(x) / UEF_Q16_ONE)

static inline uef_q16_t uef_q16_mul(uef_q16_t a, uef_q16_t b) {
    return (uef_q16_t)(((int64_t)a * b) >> UEF_Q16_FRAC_BITS);
}
static inline uef_q15_t uef_q15_mul(uef_q15_t a, uef_q15_t b) {
    return (uef_q15_t)(((int32_t)a * b) >> 15);
}
static inline uef_q31_t uef_q31_mul(uef_q31_t a, uef_q31_t b) {
    return (uef_q31_t)(((int64_t)a * b) >> 31);
}

#ifdef __cplusplus
}
#endif

#endif /* UEF_TYPES_H */
