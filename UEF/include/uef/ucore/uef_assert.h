/// @file include/uef/ucore/uef_assert.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UEF_ASSERT_H
#define UEF_ASSERT_H

#include "uef/ucore/uef_types.h"
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#  define UEF_ASSERT(cond)          ((void)0)
#  define UEF_ASSERT_MSG(cond, msg) ((void)0)
#else
UEF_NORETURN void uef_assert_fail(const char* expr, const char* file,
                                  int line, const char* msg);
#  define UEF_ASSERT(cond) \
     ((cond) ? (void)0 : uef_assert_fail(#cond, __FILE__, __LINE__, NULL))
#  define UEF_ASSERT_MSG(cond, msg) \
     ((cond) ? (void)0 : uef_assert_fail(#cond, __FILE__, __LINE__, msg))
#endif

/* Compile-time assertion */
#ifdef __cplusplus
#  define UEF_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
#  define UEF_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#ifdef __cplusplus
}
#endif

#endif /* UEF_ASSERT_H */
