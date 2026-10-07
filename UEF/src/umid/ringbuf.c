/// @file src/umid/ringbuf.c
/// @brief Bounded single-producer/single-consumer byte ring buffer.

#include "uef/umid/umid_ringbuf.h"
#include "uef/uhal/uhal_atomic.h"

#include <stddef.h>

static bool is_power_of_two(uef_u32_t value) {
    return value != 0u && (value & (value - 1u)) == 0u;
}

static bool ring_is_valid(const umid_ringbuf_t* ring) {
    return ring != NULL && ring->buf != NULL && ring->size != 0u &&
           is_power_of_two(ring->size);
}

/* A double read avoids treating an index pair observed across a simultaneous
 * wrap/refill as valid occupancy. A failed snapshot is handled as empty/full,
 * preserving memory safety if a caller violates the SPSC contract.
 */
static bool occupancy_snapshot(const umid_ringbuf_t* ring,
                               uef_u32_t* head_out,
                               uef_u32_t* tail_out,
                               uef_u32_t* used_out) {
    if (!ring_is_valid(ring) || head_out == NULL || tail_out == NULL ||
        used_out == NULL) {
        return false;
    }

    for (uef_u8_t attempt = 0u; attempt < 2u; ++attempt) {
        const uef_u32_t head = uhal_atomic_load_u32(&ring->head);
        const uef_u32_t tail = uhal_atomic_load_u32(&ring->tail);
        const uef_u32_t used = head - tail;
        if (used <= ring->size) {
            *head_out = head;
            *tail_out = tail;
            *used_out = used;
            return true;
        }
    }
    return false;
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
    uef_u32_t head;
    uef_u32_t tail;
    uef_u32_t used;
    if (!occupancy_snapshot(ring, &head, &tail, &used)) {
        return 0u;
    }
    (void)head;
    (void)tail;
    return used;
}

uef_u32_t umid_ringbuf_free(const umid_ringbuf_t* ring) {
    uef_u32_t head;
    uef_u32_t tail;
    uef_u32_t used;
    if (!occupancy_snapshot(ring, &head, &tail, &used)) {
        return 0u;
    }
    (void)head;
    (void)tail;
    return ring->size - used;
}

uef_u32_t umid_ringbuf_write(umid_ringbuf_t* ring, const uef_u8_t* source,
                             uef_u32_t length) {
    if (!ring_is_valid(ring) || source == NULL) {
        return 0u;
    }

    /* The producer owns head; acquiring tail prevents reuse before consumption. */
    const uef_u32_t head = uhal_atomic_load_u32(&ring->head);
    const uef_u32_t tail = uhal_atomic_load_u32(&ring->tail);
    const uef_u32_t used = head - tail;
    if (used > ring->size) {
        return 0u;
    }

    const uef_u32_t free_bytes = ring->size - used;
    const uef_u32_t count = length < free_bytes ? length : free_bytes;
    for (uef_u32_t index = 0u; index < count; ++index) {
        ring->buf[(head + index) & (ring->size - 1u)] = source[index];
    }

    /* Publish only after all payload bytes have been copied into free slots. */
    uhal_atomic_store_u32(&ring->head, head + count);
    return count;
}

uef_u32_t umid_ringbuf_read(umid_ringbuf_t* ring, uef_u8_t* destination,
                            uef_u32_t length) {
    if (!ring_is_valid(ring) || destination == NULL) {
        return 0u;
    }

    /* The consumer owns tail; acquire head before reading published payload. */
    const uef_u32_t tail = uhal_atomic_load_u32(&ring->tail);
    const uef_u32_t head = uhal_atomic_load_u32(&ring->head);
    const uef_u32_t available = head - tail;
    if (available > ring->size) {
        return 0u;
    }

    const uef_u32_t count = length < available ? length : available;
    for (uef_u32_t index = 0u; index < count; ++index) {
        destination[index] = ring->buf[(tail + index) & (ring->size - 1u)];
    }

    /* Release storage only after the consumer has copied the requested bytes. */
    uhal_atomic_store_u32(&ring->tail, tail + count);
    return count;
}

uef_u32_t umid_ringbuf_peek(const umid_ringbuf_t* ring, uef_u8_t* destination,
                            uef_u32_t length) {
    if (!ring_is_valid(ring) || destination == NULL) {
        return 0u;
    }

    const uef_u32_t tail = uhal_atomic_load_u32(&ring->tail);
    const uef_u32_t head = uhal_atomic_load_u32(&ring->head);
    const uef_u32_t available = head - tail;
    if (available > ring->size) {
        return 0u;
    }

    const uef_u32_t count = length < available ? length : available;
    for (uef_u32_t index = 0u; index < count; ++index) {
        destination[index] = ring->buf[(tail + index) & (ring->size - 1u)];
    }
    return count;
}

void umid_ringbuf_flush(umid_ringbuf_t* ring) {
    if (!ring_is_valid(ring)) {
        return;
    }

    /* Flush is a quiescent-state operation; callers stop both sides first. */
    uhal_atomic_store_u32(&ring->tail,
                          uhal_atomic_load_u32(&ring->head));
}
