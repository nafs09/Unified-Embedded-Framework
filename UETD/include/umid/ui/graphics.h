/// @file include/umid/ui/graphics.h
/// @brief First-pass scaffold: GUI integration, assets and display flush ownership.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/MYOVENA_HANDHELD.md#rendering-flush-and-touch.
#ifndef UETD_UMID_GRAPHICS_H
#define UETD_UMID_GRAPHICS_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/display/display.h"

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
typedef struct { upal_display_t* display; void* gui_context; void* arena; size_t arena_bytes; size_t maximum_dirty_regions; uetd_u32_t maximum_decode_pixels; } umid_graphics_config_t;
typedef struct { void* gui_context; uetd_u64_t generation; bool initialized; } umid_graphics_t;
typedef struct { uetd_u32_t asset_id, encoding_id, width, height; const uetd_u8_t* bytes; size_t length; uetd_u8_t sha256[32]; } umid_graphics_asset_t;

/// Bind a pinned GUI adapter, fixed arena and compatible display; declare all GUI/decoder memory and owner context.
uetd_status_t umid_graphics_init(umid_graphics_t* graphics, const umid_graphics_config_t* config);
/// Submit a bounded region and retain the GUI buffer until genuine display terminal completion.
uetd_status_t umid_graphics_flush(umid_graphics_t* graphics, const upal_display_region_t* region, uetd_u64_t* request_id);
/// Bound render/event/completion work; signal GUI flush-ready once after pixel ownership returns.
uetd_status_t umid_graphics_poll(umid_graphics_t* graphics, uetd_u16_t work_budget);
/// Validate immutable asset identity/dimensions/encoding and bounded decoder requirements before registration.
uetd_status_t umid_graphics_register_asset(umid_graphics_t* graphics, const umid_graphics_asset_t* asset);
/// Reject pending flushes or use explicit cancellation; release resources only after all GUI and display loans end.
uetd_status_t umid_graphics_stop(umid_graphics_t* graphics);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMID_GRAPHICS_H */
