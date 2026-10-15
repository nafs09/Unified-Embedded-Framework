/// @file include/ucore/assert.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UETD_ASSERT_H
#define UETD_ASSERT_H

#include "ucore/types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#define UETD_ASSERT(cond) ((void)0)
#define UETD_ASSERT_MSG(cond, msg) ((void)0)
#else
UETD_NORETURN void uetd_assert_fail(const char* expr, const char* file, int line, const char* msg);
#define UETD_ASSERT(cond) ((cond) ? (void)0 : uetd_assert_fail(#cond, __FILE__, __LINE__, NULL))
#define UETD_ASSERT_MSG(cond, msg) ((cond) ? (void)0 : uetd_assert_fail(#cond, __FILE__, __LINE__, msg))
#endif

/* Compile-time assertion */
#ifdef __cplusplus
#define UETD_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
#define UETD_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#ifdef __cplusplus
}
#endif

#endif /* UETD_ASSERT_H */
