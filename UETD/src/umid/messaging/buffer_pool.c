/// @file src/umid/messaging/buffer_pool.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "umid/messaging/buffer_pool.h"

uetd_status_t umid_pool_init(umid_pool_t* pool, const umid_pool_config_t* config) {
    /* TODO: Check arena size/alignment and bookkeeping capacity using overflow-safe arithmetic. */
    (void)pool;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_pool_acquire(umid_pool_t* pool, umid_pool_handle_t* handle) {
    /* TODO: Reserve one block with a non-repeating generation or report exhaustion without allocating heap. */
    (void)pool;
    (void)handle;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_pool_resolve(umid_pool_t* pool, umid_pool_handle_t handle, ucore_write_span_t* block) {
    /* TODO: Validate live handle ownership and expose only the selected block extent. */
    (void)pool;
    (void)handle;
    (void)block;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_pool_release(umid_pool_t* pool, umid_pool_handle_t handle) {
    /* TODO: Detect stale/double release and require all borrowers to be quiescent before reuse. */
    (void)pool;
    (void)handle;
    return UETD_NOT_SUPPORTED;
}

