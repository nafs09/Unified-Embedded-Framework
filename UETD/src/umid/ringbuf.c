/// @file src/umid/ringbuf.c
/// @brief Bounded single-producer/single-consumer byte ring buffer.

#include "umid/ringbuf.h"
#include "uhal/atomic.h"
#include <stddef.h>

static bool is_power_of_two(uetd_u32_t value) { return value != 0u && (value & (value - 1u)) == 0u; }

static bool ring_is_valid(const umid_ringbuf_t* ring) {
    return ring != NULL && ring->buf != NULL && ring->size != 0u && is_power_of_two(ring->size);
}

/* A double read avoids treating an index pair observed across a simultaneous
 * wrap/refill as valid occupancy. A failed snapshot is handled as empty/full,
 * preserving memory safety if a caller violates the SPSC contract.
 */
static bool occupancy_snapshot(const umid_ringbuf_t* ring, uetd_u32_t* head_out, uetd_u32_t* tail_out, uetd_u32_t* used_out) {
    if (!ring_is_valid(ring) || head_out == NULL || tail_out == NULL || used_out == NULL) { return false; }
    for (uetd_u8_t attempt = 0u; attempt < 2u; ++attempt) {
        const uetd_u32_t head = uhal_atomic_load_u32(&ring->head);
        const uetd_u32_t tail = uhal_atomic_load_u32(&ring->tail);
        const uetd_u32_t used = head - tail;
        if (used <= ring->size) {
            *head_out = head;
            *tail_out = tail;
            *used_out = used;
            return true;
        }
    }
    return false;
}

void umid_ringbuf_init(umid_ringbuf_t* ring, uetd_u8_t* storage, uetd_u32_t capacity) {
    if (ring == NULL) { return; }
    ring->buf = (storage != NULL && is_power_of_two(capacity)) ? storage : NULL;
    ring->size = ring->buf != NULL ? capacity : 0u;
    ring->head = 0u;
    ring->tail = 0u;
}

uetd_u32_t umid_ringbuf_available(const umid_ringbuf_t* ring) {
    uetd_u32_t head;
    uetd_u32_t tail;
    uetd_u32_t used;
    if (!occupancy_snapshot(ring, &head, &tail, &used)) { return 0u; }
    (void)head;
    (void)tail;
    return used;
}

uetd_u32_t umid_ringbuf_free(const umid_ringbuf_t* ring) {
    uetd_u32_t head;
    uetd_u32_t tail;
    uetd_u32_t used;
    if (!occupancy_snapshot(ring, &head, &tail, &used)) { return 0u; }
    (void)head;
    (void)tail;
    return ring->size - used;
}

uetd_u32_t umid_ringbuf_write(umid_ringbuf_t* ring, const uetd_u8_t* source, uetd_u32_t length) {
    if (!ring_is_valid(ring) || source == NULL) { return 0u; }
    /* The producer owns head; acquiring tail prevents reuse before consumption. */
    const uetd_u32_t head = uhal_atomic_load_u32(&ring->head);
    const uetd_u32_t tail = uhal_atomic_load_u32(&ring->tail);
    const uetd_u32_t used = head - tail;
    if (used > ring->size) { return 0u; }
    const uetd_u32_t free_bytes = ring->size - used;
    const uetd_u32_t count = length < free_bytes ? length : free_bytes;
    for (uetd_u32_t index = 0u; index < count; ++index) { ring->buf[(head + index) & (ring->size - 1u)] = source[index]; }
    /* Publish only after all payload bytes have been copied into free slots. */
    uhal_atomic_store_u32(&ring->head, head + count);
    return count;
}

uetd_u32_t umid_ringbuf_read(umid_ringbuf_t* ring, uetd_u8_t* destination, uetd_u32_t length) {
    if (!ring_is_valid(ring) || destination == NULL) { return 0u; }
    /* The consumer owns tail; acquire head before reading published payload. */
    const uetd_u32_t tail = uhal_atomic_load_u32(&ring->tail);
    const uetd_u32_t head = uhal_atomic_load_u32(&ring->head);
    const uetd_u32_t available = head - tail;
    if (available > ring->size) { return 0u; }
    const uetd_u32_t count = length < available ? length : available;
    for (uetd_u32_t index = 0u; index < count; ++index) { destination[index] = ring->buf[(tail + index) & (ring->size - 1u)]; }
    /* Release storage only after the consumer has copied the requested bytes. */
    uhal_atomic_store_u32(&ring->tail, tail + count);
    return count;
}

uetd_u32_t umid_ringbuf_peek(const umid_ringbuf_t* ring, uetd_u8_t* destination, uetd_u32_t length) {
    if (!ring_is_valid(ring) || destination == NULL) { return 0u; }
    const uetd_u32_t tail = uhal_atomic_load_u32(&ring->tail);
    const uetd_u32_t head = uhal_atomic_load_u32(&ring->head);
    const uetd_u32_t available = head - tail;
    if (available > ring->size) { return 0u; }
    const uetd_u32_t count = length < available ? length : available;
    for (uetd_u32_t index = 0u; index < count; ++index) { destination[index] = ring->buf[(tail + index) & (ring->size - 1u)]; }
    return count;
}

void umid_ringbuf_flush(umid_ringbuf_t* ring) {
    if (!ring_is_valid(ring)) { return; }
    /* Flush is a quiescent-state operation; callers stop both sides first. */
    uhal_atomic_store_u32(&ring->tail, uhal_atomic_load_u32(&ring->head));
}
