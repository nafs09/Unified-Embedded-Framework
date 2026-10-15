/// @file include/ucore/types.h
/// @brief Foundational fixed-width and primitive aliases for portable UETD code.

#ifndef UETD_TYPES_H
#define UETD_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Keep alignment spelling portable between the C11 core and C++ consumers. */
#ifdef __cplusplus
#define UETD_ALIGNAS(bytes) alignas(bytes)
#else
#define UETD_ALIGNAS(bytes) _Alignas(bytes)
#endif

/* Keep function attributes usable from the C core and C++ consumers. */
#if defined(__GNUC__) || defined(__clang__)
#define UETD_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define UETD_NORETURN __declspec(noreturn)
#else
#define UETD_NORETURN
#endif

/* Floating point */
typedef float uetd_f32_t;
typedef double uetd_f64_t;

/* Fixed point */
typedef int16_t uetd_q15_t; /* Q1.15 */
typedef int32_t uetd_q31_t; /* Q1.31 */
typedef int64_t uetd_q63_t; /* Q1.63 */
typedef int32_t uetd_q16_t; /* Q15.16 general-purpose */

/* Standard integers */
typedef uint8_t uetd_u8_t;
typedef uint16_t uetd_u16_t;
typedef uint32_t uetd_u32_t;
typedef uint64_t uetd_u64_t;
typedef int8_t uetd_i8_t;
typedef int16_t uetd_i16_t;
typedef int32_t uetd_i32_t;
typedef int64_t uetd_i64_t;

/* UMATH scalar and accumulator aliases are configured in umath/config.h. */

#ifdef __cplusplus
}
#endif

#endif /* UETD_TYPES_H */
