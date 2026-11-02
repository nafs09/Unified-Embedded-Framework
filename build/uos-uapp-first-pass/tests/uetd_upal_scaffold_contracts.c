#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <upal/analog/adc.h>
#include <upal/analog/comp.h>
#include <upal/analog/dac.h>
#include <upal/analog/opamp.h>
#include <upal/can/can.h>
#include <upal/capability_contracts.h>
#include <upal/compute/cordic.h>
#include <upal/compute/fmac.h>
#include <upal/display/display_panel.h>
#include <upal/display/dsi_host.h>
#include <upal/display/touch.h>
#include <upal/dma.h>
#include <upal/power.h>
#include <upal/serial/i2c.h>
#include <upal/serial/spi.h>
#include <upal/serial/uart.h>
#include <upal/serial/usb.h>
#include <upal/storage/block_device.h>
#include <upal/storage/flash.h>
#include <upal/storage/qspi.h>
#include <upal/storage/sdmmc.h>
#include <upal/timing/hrtim.h>
#include <upal/timing/rtc.h>
#include <upal/timing/timer.h>
#include <upal/transaction.h>
#include <upal/watchdog/iwdg.h>
#include <upal/watchdog/wwdg.h>

static union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;
static unsigned char before[sizeof(workspace.bytes)];

int main(void) {
    size_t failures = 0u;
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_adc_init(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (bool){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_adc_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_adc_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_adc_calibrate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_adc_calibrate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_adc_calibrate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_adc_start(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_adc_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_adc_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_adc_stop(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_adc_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_adc_read_blocking(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_adc_read_blocking: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_adc_read_blocking: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_adc_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_adc_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_adc_dma_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_adc_dma_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_comp_init(((void*)workspace.bytes), ((void*)workspace.bytes), (upal_comp_inm_t){0}, (upal_comp_cb_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_comp_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_comp_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_comp_enable(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_comp_enable: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_comp_disable(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_comp_disable: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_comp_read(((void*)workspace.bytes)) != false) {
        printf("FAIL upal_comp_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_comp_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_comp_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_comp_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dac_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dac_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dac_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_dac_write(((void*)workspace.bytes), (uetd_u8_t){0}, (uetd_u16_t){0});
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dac_write: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dac_dma_start(((void*)workspace.bytes), (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (bool){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dac_dma_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dac_dma_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_dac_dma_stop(((void*)workspace.bytes), (uetd_u8_t){0});
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dac_dma_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_opamp_init(((void*)workspace.bytes), ((void*)workspace.bytes), (upal_opamp_mode_t){0}, (uetd_u8_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_opamp_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_opamp_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_opamp_enable(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_opamp_enable: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_opamp_disable(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_opamp_disable: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_opamp_calibrate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_opamp_calibrate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_opamp_calibrate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_can_init(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (upal_can_rx_cb_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_can_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_can_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_can_add_filter(((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}, (bool){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_can_add_filter: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_can_add_filter: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_can_transmit(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_can_transmit: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_can_transmit: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_can_transmit_async(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_can_transmit_async: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_can_transmit_async: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_can_rx_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_can_rx_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_can_tx_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_can_tx_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_can_err_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_can_err_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
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
    if (upal_cordic_init() != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_cordic_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_cordic_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_cordic_sincos_q31((uetd_q31_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_cordic_sincos_q31: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_cordic_atan2_q31((uetd_q31_t){0}, (uetd_q31_t){0}) != 0) {
        printf("FAIL upal_cordic_atan2_q31: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_cordic_atan2_q31: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_cordic_modulus_q31((uetd_q31_t){0}, (uetd_q31_t){0}) != 0) {
        printf("FAIL upal_cordic_modulus_q31: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_cordic_modulus_q31: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_cordic_sqrt_q31((uetd_q31_t){0}) != 0) {
        printf("FAIL upal_cordic_sqrt_q31: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_cordic_sqrt_q31: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_cordic_batch_sincos_q31(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_cordic_batch_sincos_q31: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_cordic_batch_sincos_q31: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_fmac_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_fmac_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_fmac_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_fmac_write(((void*)workspace.bytes), (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_fmac_write: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_fmac_write: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_fmac_read(((void*)workspace.bytes), (uetd_u32_t){0}) != 0) {
        printf("FAIL upal_fmac_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_fmac_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_fmac_irq_handler();
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_fmac_irq_handler: changed caller-owned storage\n");
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
    if (upal_dma_init(((void*)workspace.bytes), ((void*)workspace.bytes), (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dma_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dma_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dma_start(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dma_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dma_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dma_abort(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dma_abort: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dma_abort: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dma_wait(((void*)workspace.bytes), (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_dma_wait: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dma_wait: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_dma_remaining(((void*)workspace.bytes)) != 0) {
        printf("FAIL upal_dma_remaining: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dma_remaining: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_dma_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_dma_irq_handler: changed caller-owned storage\n");
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
    if (upal_i2c_init(((void*)workspace.bytes), ((void*)workspace.bytes), (upal_i2c_speed_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_i2c_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_i2c_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_i2c_write_dma(((void*)workspace.bytes), (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_i2c_write_dma: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_i2c_write_dma: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_i2c_read_dma(((void*)workspace.bytes), (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_i2c_read_dma: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_i2c_read_dma: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_i2c_write_read_dma(((void*)workspace.bytes), (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_i2c_write_read_dma: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_i2c_write_read_dma: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_i2c_write_blocking(((void*)workspace.bytes), (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_i2c_write_blocking: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_i2c_write_blocking: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_i2c_read_blocking(((void*)workspace.bytes), (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_i2c_read_blocking: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_i2c_read_blocking: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_i2c_ev_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_i2c_ev_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_i2c_er_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_i2c_er_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_spi_init(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_spi_mode_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_spi_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_spi_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_spi_transfer_dma(((void*)workspace.bytes), (uhal_gpio_pin_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_spi_transfer_dma: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_spi_transfer_dma: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_spi_transfer_blocking(((void*)workspace.bytes), (uhal_gpio_pin_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_spi_transfer_blocking: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_spi_transfer_blocking: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_spi_dma_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_spi_dma_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_uart_init(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_uart_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_uart_tx_dma(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_uart_tx_dma: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_tx_dma: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_uart_tx_blocking(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_uart_tx_blocking: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_tx_blocking: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_uart_rx_available(((void*)workspace.bytes)) != 0) {
        printf("FAIL upal_uart_rx_available: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_rx_available: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_uart_rx_read(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}) != 0) {
        printf("FAIL upal_uart_rx_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_rx_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_uart_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_uart_dma_rx_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_dma_rx_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_uart_dma_tx_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_uart_dma_tx_handler: changed caller-owned storage\n");
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
    if (upal_flash_unlock() != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_flash_unlock: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_flash_unlock: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_flash_lock();
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_flash_lock: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_flash_erase_page((uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_flash_erase_page: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_flash_erase_page: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_flash_write_dword((uetd_u32_t){0}, (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_flash_write_dword: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_flash_write_dword: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_flash_write((uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_flash_write: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_flash_write: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_flash_verify((uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}) != false) {
        printf("FAIL upal_flash_verify: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_flash_verify: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_qspi_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_qspi_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_qspi_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_qspi_enter_memory_mapped(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_qspi_enter_memory_mapped: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_qspi_enter_memory_mapped: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_qspi_exit_memory_mapped(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_qspi_exit_memory_mapped: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_qspi_exit_memory_mapped: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_qspi_read(((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_qspi_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_qspi_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_qspi_erase_sector(((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_qspi_erase_sector: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_qspi_erase_sector: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_qspi_write_page(((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_qspi_write_page: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_qspi_write_page: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_sdmmc_init(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_sdmmc_width_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_sdmmc_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_sdmmc_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_sdmmc_card_present(((void*)workspace.bytes)) != false) {
        printf("FAIL upal_sdmmc_card_present: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_sdmmc_card_present: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_sdmmc_read_blocks(((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_sdmmc_read_blocks: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_sdmmc_read_blocks: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_hrtim_calibrate(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_hrtim_calibrate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_hrtim_calibrate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_hrtim_pwm_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_hrtim_pwm_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_hrtim_pwm_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_hrtim_set_duty(((void*)workspace.bytes), (upal_hrtim_timer_t){0}, (uetd_u32_t){0});
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_hrtim_set_duty: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_hrtim_preload_enable(((void*)workspace.bytes), (upal_hrtim_timer_t){0});
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_hrtim_preload_enable: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_hrtim_fault_init(((void*)workspace.bytes), (uetd_u8_t){0}, (uhal_gpio_pin_t){0}, (uetd_u8_t){0}, (bool){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_hrtim_fault_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_hrtim_fault_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_hrtim_master_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_hrtim_master_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_hrtim_timer_irq_handler(((void*)workspace.bytes), (upal_hrtim_timer_t){0});
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_hrtim_timer_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_hrtim_fault_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_hrtim_fault_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_rtc_init() != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_rtc_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_rtc_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_rtc_set_time(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_rtc_set_time: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_rtc_set_date(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_rtc_set_date: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_rtc_get_time(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_rtc_get_time: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_rtc_get_date(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_rtc_get_date: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_rtc_get_unix() != 0) {
        printf("FAIL upal_rtc_get_unix: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_rtc_get_unix: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_rtc_set_unix((uetd_u32_t){0});
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_rtc_set_unix: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_timer_init_timebase(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_timer_init_timebase: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_init_timebase: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_timer_pwm_init(((void*)workspace.bytes), (uetd_u8_t){0}, (uhal_gpio_pin_t){0}, (uetd_u8_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_timer_pwm_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_pwm_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_timer_pwm_set_duty(((void*)workspace.bytes), (uetd_u8_t){0}, (uetd_u32_t){0});
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_pwm_set_duty: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_timer_capture_init(((void*)workspace.bytes), (uetd_u8_t){0}, (uhal_gpio_pin_t){0}, (uetd_u8_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_timer_capture_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_capture_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_timer_dma_output_start(((void*)workspace.bytes), (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, (upal_dma_callback_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_timer_dma_output_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_dma_output_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_timer_start(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_timer_stop(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_timer_up_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_up_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_timer_cc_irq_handler(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_timer_cc_irq_handler: changed caller-owned storage\n");
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
    if (upal_iwdg_init((uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_iwdg_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_iwdg_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_iwdg_refresh();
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_iwdg_refresh: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (upal_wwdg_init((uetd_u32_t){0}, (uetd_u32_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_wwdg_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_wwdg_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_wwdg_refresh();
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_wwdg_refresh: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_wwdg_irq_handler();
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_wwdg_irq_handler: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    upal_wwdg_early_warning_hook();
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL upal_wwdg_early_warning_hook: changed caller-owned storage\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu uetd_upal_scaffold_contracts check(s) failed across 149 operations\n", failures);
        return 1;
    }
    printf("Validated 149 uetd_upal_scaffold_contracts operations from 27 source files\n");
    return 0;
}
