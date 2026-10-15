/// @file src/ucore/assert.c
/// @brief Fail-fast default for debug assertions.
///
/// Firmware projects may replace this weak function to record the expression,
/// source location, and message before entering their board fault path.

#include "ucore/assert.h"
#include <stdlib.h>

UETD_WEAK UETD_NORETURN void uetd_assert_fail(const char* expression, const char* file, int line, const char* message) {
    /* Keep diagnostic inputs visible to a debugger even without a logger. */
    (void)expression;
    (void)file;
    (void)line;
    (void)message;
    abort();
}
