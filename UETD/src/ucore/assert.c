/// @file src/ucore/assert.c
/// @brief Fail-fast default for debug assertions.
///
/// GCC/Clang firmware projects may replace this weak function to record the
/// expression and source location before entering their board fault path.
/// Toolchains without weak-function support must select a replacement source
/// at build time rather than relying on duplicate-symbol override behavior.

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
