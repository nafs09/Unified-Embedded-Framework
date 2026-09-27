/// @file src/umid/ringbuf.c
/// @brief Bounded single-producer/single-consumer byte ring buffer.

#include "uef/umid/umid_ringbuf.h"

#include <stdatomic.h>

/* TODO(UEF ring buffer concurrency): `head` and `tail` are volatile, not C11 atomic objects.
 * Verify the intended ISR/task target memory model and replace these fences/indices with a
 * portable atomic publication scheme or a documented critical-section implementation before
 * claiming this is data-race-free across C execution contexts.
 */
static bool is_power_of_two(uef_u32_t value) {
    /* A power-of-two capacity permits wrap with a mask instead of division/modulo. */
    return value != 0u && (value & (value - 1u)) == 0u;
}

void umid_ringbuf_init(umid_ringbuf_t* ring, uef_u8_t* storage,
                      uef_u32_t capacity) {
    if (ring == NULL) {
        return;
    }
    ring->buf = (storage != NULL && is_power_of_two(capacity)) ? storage : NULL;
    ring->size = ring->buf != NULL ? capacity : 0u;
    ring->head = 0u;
    ring->tail = 0u;
}

uef_u32_t umid_ringbuf_available(const umid_ringbuf_t* ring) {
    if (ring == NULL || ring->buf == NULL || ring->size == 0u) {
        return 0u;
    }
    /* Intended ordering boundary: consume payload only after reading the producer's head. */
    atomic_thread_fence(memory_order_acquire);
    return ring->head - ring->tail;
}

uef_u32_t umid_ringbuf_free(const umid_ringbuf_t* ring) {
    if (ring == NULL || ring->buf == NULL || ring->size == 0u) {
        return 0u;
    }
    const uef_u32_t used = umid_ringbuf_available(ring);
    return used <= ring->size ? ring->size - used : 0u;
}

uef_u32_t umid_ringbuf_write(umid_ringbuf_t* ring, const uef_u8_t* source,
                             uef_u32_t length) {
    if (ring == NULL || ring->buf == NULL || source == NULL) {
        return 0u;
    }

    /* Writes are partial when full; the returned byte count is the caller's loss signal. */
    const uef_u32_t free_bytes = umid_ringbuf_free(ring);
    const uef_u32_t count = length < free_bytes ? length : free_bytes;
    for (uef_u32_t index = 0u; index < count; ++index) {
        ring->buf[(ring->head + index) & (ring->size - 1u)] = source[index];
    }
    /* Publish the copied payload before the updated producer index, subject to the TODO above. */
    atomic_thread_fence(memory_order_release);
    ring->head += count;
    return count;
}

uef_u32_t umid_ringbuf_read(umid_ringbuf_t* ring, uef_u8_t* destination,
                            uef_u32_t length) {
    if (ring == NULL || ring->buf == NULL || destination == NULL) {
        return 0u;
    }

    /* Advance tail only after copying out, so the producer can safely reuse those slots. */
    const uef_u32_t available = umid_ringbuf_available(ring);
    const uef_u32_t count = length < available ? length : available;
    for (uef_u32_t index = 0u; index < count; ++index) {
        destination[index] = ring->buf[(ring->tail + index) & (ring->size - 1u)];
    }
    atomic_thread_fence(memory_order_release);
    ring->tail += count;
    return count;
}

uef_u32_t umid_ringbuf_peek(const umid_ringbuf_t* ring, uef_u8_t* destination,
                            uef_u32_t length) {
    if (ring == NULL || ring->buf == NULL || destination == NULL) {
        return 0u;
    }

    const uef_u32_t available = umid_ringbuf_available(ring);
    const uef_u32_t count = length < available ? length : available;
    for (uef_u32_t index = 0u; index < count; ++index) {
        destination[index] = ring->buf[(ring->tail + index) & (ring->size - 1u)];
    }
    return count;
}

void umid_ringbuf_flush(umid_ringbuf_t* ring) {
    if (ring == NULL) {
        return;
    }
    /* Discard every currently published byte without clearing the backing storage. */
    ring->tail = ring->head;
    atomic_thread_fence(memory_order_release);
}
