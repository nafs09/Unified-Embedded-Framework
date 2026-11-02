/// @file src/upal/storage/block_device.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "upal/storage/block_device.h"

uetd_status_t upal_block_init(upal_block_device_t* device, void* provider_context) {
    /* TODO: Resolve exact media/partition provider and ownership without treating absent media as initialized. */
    (void)device;
    (void)provider_context;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_block_geometry(upal_block_device_t* device, upal_block_geometry_t* geometry) {
    /* TODO: Return checked geometry, alignment and current media generation with an honest flush capability. */
    (void)device;
    (void)geometry;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_block_read(upal_block_device_t* device, const upal_block_transfer_t* transfer) {
    /* TODO: Validate LBA/count/capacity arithmetic, media generation and DMA/cache constraints; define partial-read failure. */
    (void)device;
    (void)transfer;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_block_write(upal_block_device_t* device, const upal_block_transfer_t* transfer) {
    /* TODO: Check writable geometry and source extent; distinguish accepted writes from durable storage. */
    (void)device;
    (void)transfer;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_block_flush(upal_block_device_t* device, uetd_u64_t media_generation, ucore_deadline_t deadline) {
    /* TODO: Reach the declared durability boundary or report unsupported/timeout; reject replacement media. */
    (void)device;
    (void)media_generation;
    (void)deadline;
    return UETD_NOT_SUPPORTED;
}

