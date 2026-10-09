/// @file include/uef/ucore/uef_types.h
/// @brief Foundational fixed-width and primitive aliases for portable UEF code.

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

/* UMATH scalar and accumulator aliases are configured in uef/umath/config.h. */

#ifdef __cplusplus
}
#endif

#endif /* UEF_TYPES_H */
