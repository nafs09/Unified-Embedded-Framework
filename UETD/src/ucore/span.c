/// @file src/ucore/span.c
#include "ucore/span.h"
#include <string.h>
uetd_status_t ucore_size_add(size_t a, size_t b, size_t* out) {
    if (out == NULL) { return UETD_INVALID_ARG; }
    if (b > SIZE_MAX - a) { return UETD_OVERFLOW; }
    *out = a + b; return UETD_OK;
}
uetd_status_t ucore_size_multiply(size_t a, size_t b, size_t* out) {
    if (out == NULL) { return UETD_INVALID_ARG; }
    if (a != 0u && b > SIZE_MAX / a) { return UETD_OVERFLOW; }
    *out = a * b; return UETD_OK;
}
static bool valid_span(const void* data, size_t size) {
    return (data != NULL || size == 0u) && size <= UINTPTR_MAX - (uintptr_t)data;
}
uetd_status_t ucore_span_slice(ucore_read_span_t source, size_t offset, size_t length, ucore_read_span_t* out) {
    if (out == NULL || !valid_span(source.data, source.size) || offset > source.size || length > source.size - offset) {
        return UETD_INVALID_ARG;
    }
    const ucore_read_span_t result = {source.data == NULL ? NULL : source.data + offset, length};
    *out = result; return UETD_OK;
}
uetd_status_t ucore_span_copy(ucore_write_span_t destination, ucore_read_span_t source) {
    if (!valid_span(destination.data, destination.size) || !valid_span(source.data, source.size)) { return UETD_INVALID_ARG; }
    if (source.size > destination.size) { return UETD_OVERFLOW; }
    if (source.size != 0u) { memmove(destination.data, source.data, source.size); }
    return UETD_OK;
}
