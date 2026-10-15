/// @file include/umid/ringbuf.h
/// @brief Fixed-capacity, power-of-two byte ring buffer with an SPSC contract.

#ifndef UMID_RINGBUF_H
#define UMID_RINGBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"

/* One producer and one consumer may operate concurrently. Multiple producers,
 * multiple consumers, init, and flush require external serialization. The
 * caller owns the backing storage and must finish init before publishing it.
 */
typedef struct {
    uetd_u8_t* buf;
    uetd_u32_t size;          /* nonzero power of two */
    volatile uetd_u32_t head; /* producer publication index */
    volatile uetd_u32_t tail; /* consumer release index */
} umid_ringbuf_t;

void umid_ringbuf_init(umid_ringbuf_t* rb, uetd_u8_t* buf, uetd_u32_t size);
uetd_u32_t umid_ringbuf_available(const umid_ringbuf_t* rb);
uetd_u32_t umid_ringbuf_free(const umid_ringbuf_t* rb);
uetd_u32_t umid_ringbuf_write(umid_ringbuf_t* rb, const uetd_u8_t* src, uetd_u32_t len);
uetd_u32_t umid_ringbuf_read(umid_ringbuf_t* rb, uetd_u8_t* dst, uetd_u32_t len);
uetd_u32_t umid_ringbuf_peek(const umid_ringbuf_t* rb, uetd_u8_t* dst, uetd_u32_t len);
void umid_ringbuf_flush(umid_ringbuf_t* rb);

#ifdef __cplusplus
}
#endif

#endif /* UMID_RINGBUF_H */
