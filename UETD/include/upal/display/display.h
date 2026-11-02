/// @file include/upal/display/display.h
/// @brief Bounded, panel-independent display-provider interface.
///
/// Supports command panels, RGB/MIPI-DSI scanout and display coprocessors.
/// Panel commands, clocks, touch, rendering and radio stacks are provider or
/// higher-layer concerns. No controller or MCU is selected by this interface.
#ifndef UETD_UPAL_DISPLAY_H
#define UETD_UPAL_DISPLAY_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UPAL_DISPLAY_RGB565_LE = 0, /* little-endian 16-bit R5:G6:B5 */
    UPAL_DISPLAY_RGB565_BE,     /* big-endian 16-bit R5:G6:B5 */
    UPAL_DISPLAY_RGB888,        /* bytes R,G,B */
    UPAL_DISPLAY_BGRX8888,      /* bytes B,G,R,X; X is ignored, pixels are precomposed */
} upal_display_format_t;

#define UPAL_DISPLAY_FORMAT_BIT(format) (1u << (format))
#define UPAL_DISPLAY_INITIALIZER {0}

typedef enum {
    UPAL_DISPLAY_COMMAND_PANEL = 0,
    UPAL_DISPLAY_SCANOUT,
    UPAL_DISPLAY_COPROCESSOR,
} upal_display_backend_t;

enum {
    UPAL_DISPLAY_CAP_PARTIAL_UPDATE = 1u << 0,
    UPAL_DISPLAY_CAP_BRIGHTNESS = 1u << 1,
    UPAL_DISPLAY_CAP_SLEEP = 1u << 2,
    UPAL_DISPLAY_CAP_CANCEL = 1u << 3,
};

typedef struct {
    upal_display_backend_t backend;
    uetd_u32_t width;
    uetd_u32_t height;
    uetd_u32_t formats;       /* UPAL_DISPLAY_FORMAT_BIT for each supported format */
    uetd_u32_t capabilities;
    size_t buffer_alignment; /* nonzero power of two; also applies to row strides */
    size_t maximum_transfer_bytes; /* maximum address span of one region */
} upal_display_info_t;

typedef struct {
    uetd_u32_t x, y, width, height; /* logical, already-rotated display coordinates */
    upal_display_format_t format;
    const void* pixels;
    size_t buffer_bytes;
    size_t stride_bytes;
    uetd_u32_t timeout_ms; /* finite and positive; measured from accepted submission */
} upal_display_region_t;

typedef struct {
    uetd_u64_t id;
    upal_display_region_t region;
    size_t pixel_bytes; /* useful pixels, excluding row padding */
} upal_display_request_t;

typedef enum {
    UPAL_DISPLAY_PRESENTED = 0,
    UPAL_DISPLAY_CANCELLED,
    UPAL_DISPLAY_FAILED,
} upal_display_outcome_t;

typedef struct {
    uetd_u64_t request_id;
    upal_display_outcome_t outcome;
    uetd_status_t status;
    size_t transferred_bytes; /* useful pixel bytes, excluding padding */
    bool pixels_released;
} upal_display_completion_t;

/* All methods are serialized in one task/main context, never called reentrantly
 * or from an ISR. Inputs/outputs and display storage must not overlap. poll
 * does at most work_budget provider-defined work units.
 * submit is a bounded, nonblocking admission operation. On failure it retains
 * no request/pixel reference. On success the request descriptor remains valid
 * until one terminal completion, including timeout and cancellation.
 *
 * A terminal completion MUST mean no hardware/provider will access the caller's
 * pixels again. A scanout provider therefore copies into provider-owned frame
 * storage; it cannot report completion while scanning the caller's buffer.
 * Visibility/tearing and the chosen panel synchronization are provider facts.
 * cancel only requests cancellation; it never releases the pixel loan.
 */
typedef struct {
    uetd_status_t (*start)(void* context);
    uetd_status_t (*stop)(void* context);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
    uetd_status_t (*submit)(void* context, const upal_display_request_t* request);
    uetd_status_t (*try_complete)(void* context, upal_display_completion_t* completion);
    uetd_status_t (*cancel)(void* context, uetd_u64_t request_id);
    uetd_status_t (*set_brightness)(void* context, uetd_u16_t permille);
    uetd_status_t (*set_sleep)(void* context, bool sleeping);
} upal_display_ops_t;

typedef struct {
    upal_display_info_t info;
    const upal_display_ops_t* ops;
    void* context;
    upal_display_request_t request;
    uetd_u64_t last_request_id;
    bool initialized;
    bool running;
    bool sleeping;
    bool pending;
    bool cancel_requested;
} upal_display_t;

/// Initialize with UPAL_DISPLAY_INITIALIZER before first use. The display object
/// is not movable/copyable/rebindable after initialization; ops/context outlive it.
uetd_status_t upal_display_init(upal_display_t* display, const upal_display_info_t* info,
                                const upal_display_ops_t* ops, void* context);
uetd_status_t upal_display_get_info(const upal_display_t* display, upal_display_info_t* info);
uetd_status_t upal_display_start(upal_display_t* display);
/// Stop/sleep/brightness changes reject pending transfers with UETD_BUSY.
uetd_status_t upal_display_stop(upal_display_t* display);
uetd_status_t upal_display_poll(upal_display_t* display, uetd_u16_t work_budget);
/// One request may be in flight. region is copied; pixels remain borrowed until
/// try_complete succeeds. request_id is written only on accepted submission.
uetd_status_t upal_display_submit(upal_display_t* display, const upal_display_region_t* region,
                                  uetd_u64_t* request_id);
/// UETD_NOT_READY means no completion. Output and ownership are unchanged on
/// failure, including malformed/stale provider completions.
uetd_status_t upal_display_try_complete(upal_display_t* display, upal_display_completion_t* completion);
uetd_status_t upal_display_cancel(upal_display_t* display);
uetd_status_t upal_display_set_brightness(upal_display_t* display, uetd_u16_t permille);
uetd_status_t upal_display_set_sleep(upal_display_t* display, bool sleeping);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPAL_DISPLAY_H */
