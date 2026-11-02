/// @file include/umid/messaging/buffer_pool.h
/// @brief First-pass scaffold: Fixed aligned blocks with generation-tagged ownership.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/PRODUCT_SERVICES.md#buffer-pools-events-and-dataflow.
#ifndef UETD_UMID_BUFFER_POOL_H
#define UETD_UMID_BUFFER_POOL_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/span.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Intended implementation: serialized task/main owner; no hidden heap.
 * Configurations, provider context and backing storage outlive the object.
 * Explicitly asynchronous operations retain buffers only after admission and
 * until terminal completion; synchronous spans are borrowed only for the call.
 * Schema IDs bind typed, versioned layouts/units; they do not permit arbitrary
 * native structs as a wire representation. Output/state commits and callback
 * context must be reviewed against the linked design before implementation.
 */
typedef struct { void* arena; size_t arena_bytes, block_bytes, alignment; uetd_u32_t block_count; void* bookkeeping; size_t bookkeeping_bytes; } umid_pool_config_t;
typedef struct { uetd_u32_t index; uetd_u64_t generation; } umid_pool_handle_t;
typedef struct { void* arena; void* bookkeeping; uetd_u64_t generation; bool initialized; } umid_pool_t;

/// Check arena size/alignment and bookkeeping capacity using overflow-safe arithmetic.
uetd_status_t umid_pool_init(umid_pool_t* pool, const umid_pool_config_t* config);
/// Reserve one block with a non-repeating generation or report exhaustion without allocating heap.
uetd_status_t umid_pool_acquire(umid_pool_t* pool, umid_pool_handle_t* handle);
/// Validate live handle ownership and expose only the selected block extent.
uetd_status_t umid_pool_resolve(umid_pool_t* pool, umid_pool_handle_t handle, ucore_write_span_t* block);
/// Detect stale/double release and require all borrowers to be quiescent before reuse.
uetd_status_t umid_pool_release(umid_pool_t* pool, umid_pool_handle_t handle);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMID_BUFFER_POOL_H */
