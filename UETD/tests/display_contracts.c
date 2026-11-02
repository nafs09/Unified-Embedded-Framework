#include <stdio.h>
#include <string.h>
#include <upal/display/display.h>

#define CHECK(condition) do { if (!(condition)) { \
    printf("FAIL line %d: %s\n", __LINE__, #condition); return 1; } } while (0)

typedef struct {
    uetd_status_t result;
    const upal_display_request_t* borrowed_request;
    upal_display_completion_t completion;
    unsigned submissions, cancellations, power_calls;
    bool complete;
} fake_display_t;

static uetd_status_t power(void* context) { return ((fake_display_t*)context)->result; }
static uetd_status_t poll(void* context, uetd_u16_t budget) { (void)budget; return ((fake_display_t*)context)->result; }
static uetd_status_t submit(void* context, const upal_display_request_t* request) {
    fake_display_t* fake = context;
    ++fake->submissions;
    if (fake->result == UETD_OK) fake->borrowed_request = request;
    return fake->result;
}
static uetd_status_t complete(void* context, upal_display_completion_t* completion) {
    fake_display_t* fake = context;
    if (!fake->complete) return UETD_NOT_READY;
    *completion = fake->completion;
    fake->complete = false;
    return UETD_OK;
}
static uetd_status_t cancel(void* context, uetd_u64_t id) {
    fake_display_t* fake = context;
    if (id != fake->borrowed_request->id) return UETD_INVALID_ARG;
    ++fake->cancellations;
    return fake->result;
}
static uetd_status_t brightness(void* context, uetd_u16_t value) {
    fake_display_t* fake = context;
    (void)value; ++fake->power_calls; return fake->result;
}
static uetd_status_t sleep_display(void* context, bool value) {
    fake_display_t* fake = context;
    (void)value; ++fake->power_calls; return fake->result;
}

int main(void) {
    const upal_display_ops_t ops = {power, power, poll, submit, complete, cancel, brightness, sleep_display};
    upal_display_info_t info = {UPAL_DISPLAY_SCANOUT, 4, 4,
        UPAL_DISPLAY_FORMAT_BIT(UPAL_DISPLAY_RGB565_LE),
        UPAL_DISPLAY_CAP_PARTIAL_UPDATE | UPAL_DISPLAY_CAP_CANCEL | UPAL_DISPLAY_CAP_BRIGHTNESS | UPAL_DISPLAY_CAP_SLEEP,
        8, SIZE_MAX};
    fake_display_t fake = {0};
    upal_display_t display = UPAL_DISPLAY_INITIALIZER;
    UETD_ALIGNAS(8) uetd_u8_t pixels[64] = {0};
    upal_display_region_t region = {0, 0, 2, 2, UPAL_DISPLAY_RGB565_LE, pixels, sizeof(pixels), 8, 100};
    upal_display_completion_t result, before;
    uetd_u64_t id = 77;

    info.buffer_alignment = 3;
    CHECK(upal_display_init(&display, &info, &ops, &fake) == UETD_INVALID_ARG && !display.initialized);
    info.buffer_alignment = 8;
    CHECK(upal_display_init(&display, &info, &ops, &fake) == UETD_OK);
    CHECK(upal_display_init(&display, &info, &ops, &fake) == UETD_CONFLICT);
    CHECK(upal_display_submit(&display, &region, &id) == UETD_NOT_READY && id == 77);
    fake.result = UETD_ERROR;
    CHECK(upal_display_start(&display) == UETD_ERROR && !display.running);
    fake.result = UETD_OK;
    CHECK(upal_display_start(&display) == UETD_OK);
    CHECK(upal_display_poll(&display, 0) == UETD_INVALID_ARG);
    CHECK(upal_display_poll(&display, 1) == UETD_OK);

    region.x = UINT32_MAX;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_INVALID_ARG);
    region.x = 0;
    region.format = (upal_display_format_t)99;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_INVALID_ARG);
    region.format = UPAL_DISPLAY_RGB888;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_NOT_SUPPORTED);
    region.format = UPAL_DISPLAY_RGB565_LE;
    region.pixels = pixels + 1;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_CACHE_ERROR);
    region.pixels = pixels;
    region.stride_bytes = 3;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_INVALID_ARG);
    region.stride_bytes = SIZE_MAX & ~(size_t)7;
    region.height = 3;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_OVERFLOW);
    region.stride_bytes = 8; region.height = 2; region.buffer_bytes = 11;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_OVERFLOW);
    region.buffer_bytes = sizeof(pixels);
    region.pixels = (const void*)(uintptr_t)(UINTPTR_MAX & ~(uintptr_t)7);
    CHECK(upal_display_submit(&display, &region, &id) == UETD_OVERFLOW);
    region.pixels = pixels;
    CHECK(fake.submissions == 0 && id == 77);

    fake.result = UETD_BUSY;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_BUSY && !display.pending && id == 77);
    fake.result = UETD_OK;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_OK && id == 1);
    CHECK(fake.borrowed_request == &display.request && fake.borrowed_request->pixel_bytes == 8);
    region.x = 1;
    CHECK(fake.borrowed_request->region.x == 0); /* descriptor copied, pixels borrowed */
    CHECK(upal_display_submit(&display, &region, &id) == UETD_BUSY);
    CHECK(upal_display_stop(&display) == UETD_BUSY);
    CHECK(upal_display_set_sleep(&display, true) == UETD_BUSY);
    CHECK(upal_display_set_brightness(&display, 500) == UETD_BUSY && fake.power_calls == 0);
    memset(&result, 0x5A, sizeof(result)); before = result;
    CHECK(upal_display_try_complete(&display, &result) == UETD_NOT_READY);
    CHECK(memcmp(&result, &before, sizeof(result)) == 0);
    fake.completion = (upal_display_completion_t){2, UPAL_DISPLAY_PRESENTED, UETD_OK, 8, true};
    fake.complete = true;
    CHECK(upal_display_try_complete(&display, &result) == UETD_ERROR && display.pending);
    CHECK(memcmp(&result, &before, sizeof(result)) == 0);
    fake.completion.request_id = 1; fake.completion.pixels_released = false; fake.complete = true;
    CHECK(upal_display_try_complete(&display, &result) == UETD_ERROR && display.pending);
    CHECK(upal_display_cancel(&display) == UETD_OK && display.pending);
    CHECK(upal_display_cancel(&display) == UETD_OK && fake.cancellations == 1);
    fake.completion = (upal_display_completion_t){1, UPAL_DISPLAY_CANCELLED, UETD_OK, 0, true}; fake.complete = true;
    CHECK(upal_display_try_complete(&display, &result) == UETD_OK && !display.pending);
    CHECK(result.outcome == UPAL_DISPLAY_CANCELLED);

    CHECK(upal_display_submit(&display, &region, &id) == UETD_OK && id == 2);
    fake.completion = (upal_display_completion_t){2, UPAL_DISPLAY_PRESENTED, UETD_OK, 7, true}; fake.complete = true;
    CHECK(upal_display_try_complete(&display, &result) == UETD_ERROR && display.pending);
    fake.completion.transferred_bytes = 8; fake.complete = true;
    CHECK(upal_display_try_complete(&display, &result) == UETD_OK && !display.pending);
    CHECK(upal_display_submit(&display, &region, &id) == UETD_OK && id == 3);
    fake.completion = (upal_display_completion_t){3, UPAL_DISPLAY_FAILED, UETD_TIMEOUT, 0, true}; fake.complete = true;
    CHECK(upal_display_try_complete(&display, &result) == UETD_OK && !display.pending && result.status == UETD_TIMEOUT);
    CHECK(upal_display_set_brightness(&display, 1001) == UETD_INVALID_ARG);
    CHECK(upal_display_set_brightness(&display, 500) == UETD_OK);
    fake.result = UETD_ERROR;
    CHECK(upal_display_set_sleep(&display, true) == UETD_ERROR && !display.sleeping);
    fake.result = UETD_OK;
    CHECK(upal_display_set_sleep(&display, true) == UETD_OK && display.sleeping);
    CHECK(upal_display_submit(&display, &region, &id) == UETD_NOT_READY);
    CHECK(upal_display_set_sleep(&display, false) == UETD_OK);
    display.last_request_id = UINT64_MAX;
    CHECK(upal_display_submit(&display, &region, &id) == UETD_OVERFLOW);
    fake.result = UETD_ERROR;
    CHECK(upal_display_stop(&display) == UETD_ERROR && display.running);
    fake.result = UETD_OK;
    CHECK(upal_display_stop(&display) == UETD_OK && !display.running);

    /* A provider without optional capabilities accepts only full-frame writes. */
    {
        upal_display_t basic = UPAL_DISPLAY_INITIALIZER;
        upal_display_ops_t basic_ops = ops;
        basic_ops.cancel = NULL; basic_ops.set_sleep = NULL; basic_ops.set_brightness = NULL;
        info.capabilities = 0;
        CHECK(upal_display_init(&basic, &info, &basic_ops, &fake) == UETD_OK);
        CHECK(upal_display_start(&basic) == UETD_OK);
        CHECK(upal_display_set_brightness(&basic, 1) == UETD_NOT_SUPPORTED);
        CHECK(upal_display_set_sleep(&basic, true) == UETD_NOT_SUPPORTED);
        region.x = 0;
        CHECK(upal_display_submit(&basic, &region, &id) == UETD_NOT_SUPPORTED);
        region.width = 4; region.height = 4;
        CHECK(upal_display_submit(&basic, &region, &id) == UETD_OK);
        CHECK(upal_display_cancel(&basic) == UETD_NOT_SUPPORTED);
    }
    puts("Display provider contracts passed");
    return 0;
}
