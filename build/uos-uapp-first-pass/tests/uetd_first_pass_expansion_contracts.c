#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <upal/capability_contracts.h>
#include <ucore/codec.h>
#include <upal/transaction.h>
#include <upal/display/dsi_host.h>
#include <upal/display/display_panel.h>
#include <upal/display/touch.h>
#include <upal/power.h>
#include <upal/storage/block_device.h>
#include <umid/messaging/buffer_pool.h>
#include <umid/messaging/mailbox.h>
#include <umid/messaging/event_bus.h>
#include <umid/sensor.h>
#include <umid/data/settings.h>
#include <umid/ui/input.h>
#include <umid/ui/graphics.h>
#include <uactuator/actuator.h>
#include <usec/provider.h>
#include <uproto/network/connection_manager.h>
#include <uproto/usb_device.h>
#include <uproto/network/time_sync.h>
#include <uproto/framing.h>

static union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;
static unsigned char before[sizeof(workspace.bytes)];

int main(void) {
    size_t failures = 0u;
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_uart_configure_advanced(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_uart_configure_advanced: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_configure_advanced: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_uart_transmit_advanced(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_uart_transfer_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_uart_transmit_advanced: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_transmit_advanced: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_timer_capture_configure(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_timer_capture_configure: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_capture_configure: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_timer_capture_read(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_timer_capture_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_capture_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_sync_group_validate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_sync_group_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_sync_group_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_sync_group_start(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_sync_group_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_sync_group_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_sync_group_stop(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_sync_group_stop: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_sync_group_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_usb_start_profile(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_usb_start_profile: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_usb_start_profile: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_usb_send_report(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_usb_send_report: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_usb_send_report: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_usb_stop_profile(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_usb_stop_profile: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_usb_stop_profile: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (ucore_codec_read_u32(((void*)workspace.bytes), (ucore_codec_order_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL ucore_codec_read_u32: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL ucore_codec_read_u32: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (ucore_codec_write_u32(((void*)workspace.bytes), (ucore_codec_order_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL ucore_codec_write_u32: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL ucore_codec_write_u32: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (ucore_codec_read_bytes(((void*)workspace.bytes), (ucore_write_span_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL ucore_codec_read_bytes: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL ucore_codec_read_bytes: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (ucore_codec_write_bytes(((void*)workspace.bytes), (ucore_read_span_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL ucore_codec_write_bytes: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL ucore_codec_write_bytes: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_transaction_begin(((void*)workspace.bytes), (upal_transaction_token_t){0}, (ucore_deadline_t){0}, (size_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_transaction_begin: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_transaction_begin: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_transaction_request_cancel(((void*)workspace.bytes), (upal_transaction_token_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_transaction_request_cancel: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_transaction_request_cancel: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_transaction_complete(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_transaction_complete: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_transaction_complete: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_transaction_query(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_transaction_query: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_transaction_query: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dsi_validate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dsi_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dsi_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dsi_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dsi_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dsi_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dsi_submit(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dsi_submit: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dsi_submit: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dsi_poll(((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dsi_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dsi_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dsi_try_complete(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dsi_try_complete: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dsi_try_complete: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dsi_cancel(((void*)workspace.bytes), (upal_transaction_token_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dsi_cancel: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dsi_cancel: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dsi_stop(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dsi_stop: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dsi_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_panel_validate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_panel_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_panel_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_panel_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_panel_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_panel_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_panel_start(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_panel_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_panel_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_panel_poll(((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_panel_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_panel_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_panel_get_display(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_panel_get_display: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_panel_get_display: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_panel_stop(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_panel_stop: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_panel_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_touch_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_touch_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_touch_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_touch_poll(((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_touch_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_touch_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_touch_read(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_touch_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_touch_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_touch_set_sleep(((void*)workspace.bytes), (bool){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_touch_set_sleep: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_touch_set_sleep: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_power_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_power_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_power_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_power_acquire(((void*)workspace.bytes), (upal_power_mode_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_power_acquire: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_power_acquire: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_power_release(((void*)workspace.bytes), (upal_power_lease_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_power_release: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_power_release: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_power_enter(((void*)workspace.bytes), (upal_power_mode_t){0}, (ucore_deadline_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_power_enter: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_power_enter: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_power_get_status(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_power_get_status: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_power_get_status: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_block_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_block_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_block_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_block_geometry(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_block_geometry: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_block_geometry: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_block_read(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_block_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_block_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_block_write(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_block_write: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_block_write: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_block_flush(((void*)workspace.bytes), (uetd_u64_t){0}, (ucore_deadline_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_block_flush: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_block_flush: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_pool_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_pool_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_pool_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_pool_acquire(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_pool_acquire: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_pool_acquire: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_pool_resolve(((void*)workspace.bytes), (umid_pool_handle_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_pool_resolve: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_pool_resolve: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_pool_release(((void*)workspace.bytes), (umid_pool_handle_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_pool_release: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_pool_release: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_mailbox_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_mailbox_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_mailbox_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_mailbox_publish(((void*)workspace.bytes), (ucore_read_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_mailbox_publish: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_mailbox_publish: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_mailbox_read(((void*)workspace.bytes), (ucore_write_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_mailbox_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_mailbox_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_event_bus_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_event_bus_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_event_bus_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_event_publish(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_event_publish: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_event_publish: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_event_receive(((void*)workspace.bytes), (uetd_u32_t){0}, (ucore_write_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_event_receive: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_event_receive: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_event_bus_reset(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_event_bus_reset: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_event_bus_reset: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_sensor_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_sensor_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_sensor_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_sensor_poll(((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_sensor_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_sensor_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_sensor_read(((void*)workspace.bytes), (ucore_write_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_sensor_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_sensor_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_sensor_activate_calibration(((void*)workspace.bytes), (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_sensor_activate_calibration: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_sensor_activate_calibration: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_settings_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_settings_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_settings_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_settings_stage(((void*)workspace.bytes), (uetd_u64_t){0}, (ucore_read_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_settings_stage: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_settings_stage: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_settings_validate(((void*)workspace.bytes), (umid_settings_transaction_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_settings_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_settings_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_settings_commit(((void*)workspace.bytes), (umid_settings_transaction_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_settings_commit: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_settings_commit: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_settings_abort(((void*)workspace.bytes), (umid_settings_transaction_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_settings_abort: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_settings_abort: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_input_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_input_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_input_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_input_submit_touch(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_input_submit_touch: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_input_submit_touch: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_input_submit_event(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_input_submit_event: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_input_submit_event: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_input_next(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_input_next: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_input_next: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_input_cancel_source(((void*)workspace.bytes), (uetd_u16_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_input_cancel_source: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_input_cancel_source: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_graphics_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_graphics_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_graphics_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_graphics_flush(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_graphics_flush: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_graphics_flush: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_graphics_poll(((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_graphics_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_graphics_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_graphics_register_asset(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_graphics_register_asset: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_graphics_register_asset: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_graphics_stop(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_graphics_stop: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_graphics_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uactuator_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uactuator_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uactuator_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uactuator_arm(((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uactuator_arm: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uactuator_arm: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uactuator_submit(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uactuator_submit: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uactuator_submit: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uactuator_inhibit(((void*)workspace.bytes), (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uactuator_inhibit: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uactuator_inhibit: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uactuator_feedback(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uactuator_feedback: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uactuator_feedback: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (usec_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL usec_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL usec_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (usec_random(((void*)workspace.bytes), (ucore_write_span_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL usec_random: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL usec_random: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (usec_execute(((void*)workspace.bytes), ((void*)workspace.bytes), (ucore_write_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL usec_execute: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL usec_execute: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (usec_authorize(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL usec_authorize: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL usec_authorize: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (usec_key_destroy(((void*)workspace.bytes), (usec_key_handle_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL usec_key_destroy: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL usec_key_destroy: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_connection_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_connection_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_connection_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_connection_poll(((void*)workspace.bytes), (ucore_clock_stamp_t){0}, (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_connection_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_connection_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_connection_get_status(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_connection_get_status: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_connection_get_status: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_connection_stop(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_connection_stop: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_connection_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_usb_device_validate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_usb_device_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_usb_device_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_usb_device_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_usb_device_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_usb_device_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_usb_device_poll(((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_usb_device_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_usb_device_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_usb_device_write(((void*)workspace.bytes), (ucore_read_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_usb_device_write: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_usb_device_write: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_usb_device_read(((void*)workspace.bytes), (ucore_write_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_usb_device_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_usb_device_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_usb_device_stop(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_usb_device_stop: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_usb_device_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_time_sync_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_time_sync_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_time_sync_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_time_sync_poll(((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_time_sync_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_time_sync_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_time_sync_mapping(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_time_sync_mapping: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_time_sync_mapping: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_framing_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_framing_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_framing_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_framing_encode(((void*)workspace.bytes), (ucore_read_span_t){0}, (ucore_write_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_framing_encode: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_framing_encode: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_framing_feed(((void*)workspace.bytes), (ucore_read_span_t){0}, (ucore_clock_stamp_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_framing_feed: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_framing_feed: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_framing_next(((void*)workspace.bytes), (ucore_write_span_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_framing_next: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_framing_next: changed caller-owned storage\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu uetd_first_pass_expansion_contracts check(s) failed across 102 operations\n", failures);
        return 1;
    }
    printf("Validated 102 uetd_first_pass_expansion_contracts operations from 21 source files\n");
    return 0;
}
