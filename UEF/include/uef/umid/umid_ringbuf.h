/// @file include/uef/umid/umid_ringbuf.h
/// @brief Power-of-two byte ring buffer for UMID, UPROTO, and generated UCON; SPSC memory ordering still needs review.

#ifndef UMID_RINGBUF_H
#define UMID_RINGBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"

typedef struct {
    uef_u8_t*          buf;
    uef_u32_t          size;         /* must be power of 2 */
    volatile uef_u32_t head;         /* producer's write index; concurrency semantics are TODO */
    volatile uef_u32_t tail;         /* consumer's read index; concurrency semantics are TODO */
} umid_ringbuf_t;

void      umid_ringbuf_init(umid_ringbuf_t* rb,
                              uef_u8_t* buf, uef_u32_t size);
uef_u32_t umid_ringbuf_available(const umid_ringbuf_t* rb);
uef_u32_t umid_ringbuf_free(const umid_ringbuf_t* rb);
uef_u32_t umid_ringbuf_write(umid_ringbuf_t* rb,
                               const uef_u8_t* src, uef_u32_t len);
uef_u32_t umid_ringbuf_read(umid_ringbuf_t* rb,
                              uef_u8_t* dst, uef_u32_t len);
uef_u32_t umid_ringbuf_peek(const umid_ringbuf_t* rb,
                              uef_u8_t* dst, uef_u32_t len);
void      umid_ringbuf_flush(umid_ringbuf_t* rb);

#ifdef __cplusplus
}
#endif

#endif /* UMID_RINGBUF_H */
