/// @file include/upal/storage/block_device.h
/// @brief First-pass scaffold: Generation-tagged block storage and durability.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/PRODUCT_SERVICES.md#storage-partitions-and-filesystems.
#ifndef UETD_UPAL_BLOCK_DEVICE_H
#define UETD_UPAL_BLOCK_DEVICE_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/deadline.h"

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
typedef struct { uetd_u64_t block_count; uetd_u32_t block_bytes; uetd_u32_t erase_blocks; size_t alignment; uetd_u64_t media_generation; bool read_only; bool durable_flush; } upal_block_geometry_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } upal_block_device_t;
typedef struct { uetd_u64_t media_generation, first_block; uetd_u32_t block_count; void* bytes; size_t capacity; ucore_deadline_t deadline; } upal_block_transfer_t;

/// Resolve exact media/partition provider and ownership without treating absent media as initialized.
uetd_status_t upal_block_init(upal_block_device_t* device, void* provider_context);
/// Return checked geometry, alignment and current media generation with an honest flush capability.
uetd_status_t upal_block_geometry(upal_block_device_t* device, upal_block_geometry_t* geometry);
/// Validate LBA/count/capacity arithmetic, media generation and DMA/cache constraints; define partial-read failure.
uetd_status_t upal_block_read(upal_block_device_t* device, const upal_block_transfer_t* transfer);
/// Check writable geometry and source extent; distinguish accepted writes from durable storage.
uetd_status_t upal_block_write(upal_block_device_t* device, const upal_block_transfer_t* transfer);
/// Reach the declared durability boundary or report unsupported/timeout; reject replacement media.
uetd_status_t upal_block_flush(upal_block_device_t* device, uetd_u64_t media_generation, ucore_deadline_t deadline);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPAL_BLOCK_DEVICE_H */
