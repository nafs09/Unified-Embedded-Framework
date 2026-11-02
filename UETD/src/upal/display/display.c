/// @file src/upal/display/display.c
/// @brief Validated dispatch and bounded pixel-loan ownership for display providers.
#include "upal/display/display.h"
#include <string.h>

static size_t pixel_size(upal_display_format_t format) {
    switch (format) {
    case UPAL_DISPLAY_RGB565_LE:
    case UPAL_DISPLAY_RGB565_BE: return 2u;
    case UPAL_DISPLAY_RGB888: return 3u;
    case UPAL_DISPLAY_BGRX8888: return 4u;
    default: return 0u;
    }
}

static bool valid_display(const upal_display_t* display) {
    return display != NULL && display->initialized;
}

uetd_status_t upal_display_init(upal_display_t* display, const upal_display_info_t* info,
                                const upal_display_ops_t* ops, void* context) {
    const uetd_u32_t formats = UPAL_DISPLAY_FORMAT_BIT(UPAL_DISPLAY_RGB565_LE) |
        UPAL_DISPLAY_FORMAT_BIT(UPAL_DISPLAY_RGB565_BE) | UPAL_DISPLAY_FORMAT_BIT(UPAL_DISPLAY_RGB888) |
        UPAL_DISPLAY_FORMAT_BIT(UPAL_DISPLAY_BGRX8888);
    const uetd_u32_t capabilities = UPAL_DISPLAY_CAP_PARTIAL_UPDATE | UPAL_DISPLAY_CAP_BRIGHTNESS |
        UPAL_DISPLAY_CAP_SLEEP | UPAL_DISPLAY_CAP_CANCEL;
    if (display == NULL || info == NULL || ops == NULL || info->width == 0u || info->height == 0u ||
        info->backend < UPAL_DISPLAY_COMMAND_PANEL || info->backend > UPAL_DISPLAY_COPROCESSOR ||
        info->formats == 0u || (info->formats & ~formats) != 0u || (info->capabilities & ~capabilities) != 0u ||
        info->buffer_alignment == 0u || (info->buffer_alignment & (info->buffer_alignment - 1u)) != 0u ||
        info->maximum_transfer_bytes == 0u || ops->start == NULL || ops->stop == NULL || ops->poll == NULL ||
        ops->submit == NULL || ops->try_complete == NULL ||
        ((info->capabilities & UPAL_DISPLAY_CAP_CANCEL) != 0u && ops->cancel == NULL) ||
        ((info->capabilities & UPAL_DISPLAY_CAP_BRIGHTNESS) != 0u && ops->set_brightness == NULL) ||
        ((info->capabilities & UPAL_DISPLAY_CAP_SLEEP) != 0u && ops->set_sleep == NULL)) return UETD_INVALID_ARG;
    if (display->initialized) return UETD_CONFLICT;
    display->info = *info;
    display->ops = ops;
    display->context = context;
    memset(&display->request, 0, sizeof(display->request));
    display->last_request_id = 0u;
    display->running = false;
    display->sleeping = false;
    display->pending = false;
    display->cancel_requested = false;
    display->initialized = true;
    return UETD_OK;
}

uetd_status_t upal_display_get_info(const upal_display_t* display, upal_display_info_t* info) {
    if (!valid_display(display) || info == NULL) return UETD_INVALID_ARG;
    *info = display->info;
    return UETD_OK;
}

uetd_status_t upal_display_start(upal_display_t* display) {
    uetd_status_t status;
    if (!valid_display(display)) return UETD_INVALID_ARG;
    if (display->running) return UETD_CONFLICT;
    status = display->ops->start(display->context);
    if (status == UETD_OK) { display->running = true; display->sleeping = false; }
    return status;
}

uetd_status_t upal_display_stop(upal_display_t* display) {
    uetd_status_t status;
    if (!valid_display(display)) return UETD_INVALID_ARG;
    if (!display->running) return UETD_NOT_READY;
    if (display->pending) return UETD_BUSY;
    status = display->ops->stop(display->context);
    if (status == UETD_OK) { display->running = false; display->sleeping = false; }
    return status;
}

uetd_status_t upal_display_poll(upal_display_t* display, uetd_u16_t work_budget) {
    if (!valid_display(display) || work_budget == 0u) return UETD_INVALID_ARG;
    if (!display->running) return UETD_NOT_READY;
    return display->ops->poll(display->context, work_budget);
}

uetd_status_t upal_display_submit(upal_display_t* display, const upal_display_region_t* region,
                                  uetd_u64_t* request_id) {
    size_t bpp, row_bytes, span_bytes, pixel_bytes;
    uetd_status_t status;
    if (!valid_display(display) || region == NULL || request_id == NULL || region->pixels == NULL ||
        region->width == 0u || region->height == 0u || region->timeout_ms == 0u) return UETD_INVALID_ARG;
    if (!display->running || display->sleeping) return UETD_NOT_READY;
    if (display->pending) return UETD_BUSY;
    bpp = pixel_size(region->format);
    if (bpp == 0u || region->x > display->info.width || region->width > display->info.width - region->x ||
        region->y > display->info.height || region->height > display->info.height - region->y) return UETD_INVALID_ARG;
    if ((display->info.formats & UPAL_DISPLAY_FORMAT_BIT(region->format)) == 0u) return UETD_NOT_SUPPORTED;
    if ((display->info.capabilities & UPAL_DISPLAY_CAP_PARTIAL_UPDATE) == 0u &&
        (region->x != 0u || region->y != 0u || region->width != display->info.width || region->height != display->info.height))
        return UETD_NOT_SUPPORTED;
    if ((size_t)region->width > SIZE_MAX / bpp) return UETD_OVERFLOW;
    row_bytes = (size_t)region->width * bpp;
    if (region->stride_bytes < row_bytes) return UETD_INVALID_ARG;
    if (((uintptr_t)region->pixels & (display->info.buffer_alignment - 1u)) != 0u ||
        (region->stride_bytes & (display->info.buffer_alignment - 1u)) != 0u) return UETD_CACHE_ERROR;
    if (region->height > 1u && region->stride_bytes > (SIZE_MAX - row_bytes) / ((size_t)region->height - 1u))
        return UETD_OVERFLOW;
    span_bytes = ((size_t)region->height - 1u) * region->stride_bytes + row_bytes;
    if (span_bytes > region->buffer_bytes || span_bytes > display->info.maximum_transfer_bytes ||
        span_bytes - 1u > UINTPTR_MAX - (uintptr_t)region->pixels ||
        (size_t)region->height > SIZE_MAX / row_bytes) return UETD_OVERFLOW;
    pixel_bytes = row_bytes * (size_t)region->height;
    if (display->last_request_id == UINT64_MAX) return UETD_OVERFLOW;
    display->request.id = display->last_request_id + 1u;
    display->request.region = *region;
    display->request.pixel_bytes = pixel_bytes;
    status = display->ops->submit(display->context, &display->request);
    if (status != UETD_OK) { memset(&display->request, 0, sizeof(display->request)); return status; }
    display->last_request_id = display->request.id;
    display->pending = true;
    display->cancel_requested = false;
    *request_id = display->request.id;
    return UETD_OK;
}

uetd_status_t upal_display_try_complete(upal_display_t* display, upal_display_completion_t* completion) {
    upal_display_completion_t candidate = {0};
    uetd_status_t status;
    if (!valid_display(display) || completion == NULL) return UETD_INVALID_ARG;
    if (!display->pending) return UETD_NOT_READY;
    status = display->ops->try_complete(display->context, &candidate);
    if (status != UETD_OK) return status;
    if (candidate.request_id != display->request.id || !candidate.pixels_released ||
        candidate.outcome < UPAL_DISPLAY_PRESENTED || candidate.outcome > UPAL_DISPLAY_FAILED ||
        candidate.status < UETD_OK || candidate.status > UETD_CONFLICT ||
        candidate.transferred_bytes > display->request.pixel_bytes ||
        (candidate.outcome == UPAL_DISPLAY_PRESENTED &&
         (candidate.status != UETD_OK || candidate.transferred_bytes != display->request.pixel_bytes)) ||
        (candidate.outcome == UPAL_DISPLAY_FAILED && candidate.status == UETD_OK)) return UETD_ERROR;
    *completion = candidate;
    display->pending = false;
    display->cancel_requested = false;
    memset(&display->request, 0, sizeof(display->request));
    return UETD_OK;
}

uetd_status_t upal_display_cancel(upal_display_t* display) {
    uetd_status_t status;
    if (!valid_display(display)) return UETD_INVALID_ARG;
    if (!display->pending) return UETD_NOT_READY;
    if ((display->info.capabilities & UPAL_DISPLAY_CAP_CANCEL) == 0u) return UETD_NOT_SUPPORTED;
    if (display->cancel_requested) return UETD_OK;
    status = display->ops->cancel(display->context, display->request.id);
    if (status == UETD_OK) display->cancel_requested = true;
    return status;
}

uetd_status_t upal_display_set_brightness(upal_display_t* display, uetd_u16_t permille) {
    if (!valid_display(display) || permille > 1000u) return UETD_INVALID_ARG;
    if (!display->running) return UETD_NOT_READY;
    if (display->pending) return UETD_BUSY;
    if ((display->info.capabilities & UPAL_DISPLAY_CAP_BRIGHTNESS) == 0u) return UETD_NOT_SUPPORTED;
    return display->ops->set_brightness(display->context, permille);
}

uetd_status_t upal_display_set_sleep(upal_display_t* display, bool sleeping) {
    uetd_status_t status;
    if (!valid_display(display)) return UETD_INVALID_ARG;
    if (!display->running) return UETD_NOT_READY;
    if (display->pending) return UETD_BUSY;
    if ((display->info.capabilities & UPAL_DISPLAY_CAP_SLEEP) == 0u) return UETD_NOT_SUPPORTED;
    status = display->ops->set_sleep(display->context, sleeping);
    if (status == UETD_OK) display->sleeping = sleeping;
    return status;
}
