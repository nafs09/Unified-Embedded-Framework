/// @file include/upal/future/capability_contracts.h
/// @brief Type-level design outline for deferred UPAL peripheral extensions.
///
/// This header is intentionally not included by upal.h. No function declared
/// here has a supported implementation until its future catalogue item is
/// promoted and a target backend is qualified.

#ifndef UPAL_FUTURE_CAPABILITY_CONTRACTS_H
#define UPAL_FUTURE_CAPABILITY_CONTRACTS_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/gpio.h"
#include "upal/adc.h"
#include "upal/hrtim.h"
#include "upal/timer.h"
#include "upal/uart.h"

typedef struct upal_usb upal_usb_t;
typedef struct upal_ethernet upal_ethernet_t;

typedef enum {
    UPAL_UART_PARITY_NONE = 0,
    UPAL_UART_PARITY_EVEN,
    UPAL_UART_PARITY_ODD,
} upal_uart_parity_t;

typedef enum {
    UPAL_UART_STOP_1 = 1,
    UPAL_UART_STOP_2 = 2,
} upal_uart_stop_bits_t;

typedef enum {
    UPAL_UART_DUPLEX_FULL = 0,
    UPAL_UART_DUPLEX_HALF,
} upal_uart_duplex_t;

typedef enum {
    UPAL_UART_FLOW_NONE = 0,
    UPAL_UART_FLOW_RTS_CTS,
} upal_uart_flow_control_t;

typedef enum {
    UPAL_UART_DE_RELEASE_AT_DMA_COMPLETE = 0,
    UPAL_UART_DE_RELEASE_AT_UART_COMPLETE,
} upal_uart_de_release_t;

typedef enum {
    UPAL_UART_ECHO_IGNORE = 0,
    UPAL_UART_ECHO_EXPECT,
    UPAL_UART_ECHO_DISCARD,
} upal_uart_echo_policy_t;

typedef struct {
    uetd_u8_t data_bits; /* 7, 8, or 9, if supported by the selected instance */
    upal_uart_parity_t parity;
    upal_uart_stop_bits_t stop_bits;
    upal_uart_flow_control_t flow_control;
    upal_uart_duplex_t duplex;
    bool invert_rx;
    bool invert_tx;
    uetd_u8_t oversampling;
    uetd_u32_t maximum_baud_error_ppm;
    bool rs485_de_enabled;
    uhal_gpio_pin_t de_pin;
    uetd_u8_t de_af;
    bool de_active_high;
    uetd_u32_t de_assert_delay_us;
    uetd_u32_t de_release_delay_us;
    upal_uart_de_release_t de_release_boundary;
    upal_uart_echo_policy_t echo_policy;
    uetd_u32_t turnaround_timeout_us;
} upal_uart_advanced_config_t;

typedef struct {
    uetd_status_t status;
    uetd_u32_t bytes_transferred;
    uetd_u32_t terminal_flags;
    uetd_u64_t completed_at_us;
} upal_uart_transfer_result_t;

typedef void (*upal_uart_transfer_callback_t)(const upal_uart_transfer_result_t* result, void* context);

uetd_status_t upal_uart_configure_advanced(upal_uart_t* uart, const upal_uart_advanced_config_t* config);
uetd_status_t upal_uart_transmit_advanced(upal_uart_t* uart, const uetd_u8_t* data, uetd_u32_t length,
                                         upal_uart_transfer_callback_t callback, void* context);

typedef enum {
    UPAL_CAPTURE_EDGE_RISING = 1u,
    UPAL_CAPTURE_EDGE_FALLING = 2u,
    UPAL_CAPTURE_EDGE_BOTH = 3u,
} upal_capture_edge_t;

typedef struct {
    uetd_u8_t channel;
    uetd_u8_t edge;
    uetd_u16_t flags;
    uetd_u32_t raw_counter;
    uetd_u64_t extended_ticks;
    uetd_u64_t captured_at_us;
} upal_timer_capture_event_t;

typedef enum {
    UPAL_CAPTURE_DELIVERY_QUEUE = 0,
    UPAL_CAPTURE_DELIVERY_DEFERRED_CALLBACK,
    UPAL_CAPTURE_DELIVERY_DMA_BLOCK,
} upal_capture_delivery_t;

typedef enum {
    UPAL_CAPTURE_OVERFLOW_DROP_NEWEST = 0,
    UPAL_CAPTURE_OVERFLOW_DROP_OLDEST,
    UPAL_CAPTURE_OVERFLOW_STOP,
} upal_capture_overflow_t;

typedef void (*upal_timer_capture_callback_t)(const upal_timer_capture_event_t* event, void* context);

enum {
    UPAL_CAPTURE_FLAG_OVERCAPTURE = 1u << 0,
    UPAL_CAPTURE_FLAG_WRAP_AMBIGUOUS = 1u << 1,
    UPAL_CAPTURE_FLAG_TIMESTAMP_VALID = 1u << 2,
};

typedef struct {
    uetd_u8_t channel;
    upal_capture_edge_t edge;
    uetd_u8_t digital_filter;
    uetd_u64_t timer_tick_hz;
    uetd_u64_t maximum_unambiguous_interval_ticks;
    upal_capture_delivery_t delivery;
    upal_capture_overflow_t overflow;
    upal_timer_capture_event_t* event_storage;
    uetd_u16_t event_capacity;
    upal_timer_capture_callback_t callback;
    void* callback_context;
    uetd_u32_t* dma_tick_storage;
    uetd_u32_t dma_tick_capacity;
} upal_timer_capture_config_t;

uetd_status_t upal_timer_capture_configure(upal_timer_t* timer, const upal_timer_capture_config_t* config);
uetd_status_t upal_timer_capture_read(upal_timer_t* timer, upal_timer_capture_event_t* event);

typedef enum {
    UPAL_SYNC_MEMBER_TIMER = 0,
    UPAL_SYNC_MEMBER_ADC,
    UPAL_SYNC_MEMBER_HRTIM,
} upal_sync_member_kind_t;

typedef struct {
    upal_sync_member_kind_t kind;
    union {
        upal_timer_t* timer;
        upal_adc_t* adc;
        upal_hrtim_t* hrtim;
    } instance;
    uetd_u32_t trigger_route_id; /* resolved from the selected target record */
    uetd_s32_t phase_ticks;
    uetd_u8_t cohort_id;
    uetd_u16_t dma_frame_samples;
    void* caller_owned_dma_frame;
} upal_sync_member_t;

typedef enum {
    UPAL_SYNC_EVENT_UPDATE = 0,
    UPAL_SYNC_EVENT_COMPARE,
    UPAL_SYNC_EVENT_EXTERNAL_TRIGGER,
} upal_sync_event_t;

typedef struct {
    upal_sync_member_t* members;
    uetd_u8_t member_count;
    uetd_u8_t master_index;
    uetd_u32_t timer_clock_hz;
    upal_sync_event_t master_event;
    uetd_u32_t maximum_start_skew_ns;
    uetd_u32_t maximum_commit_skew_ns;
    bool rollback_on_partial_start;
    bool commit_preloaded_outputs_at_group_event;
} upal_sync_group_t;

uetd_status_t upal_sync_group_validate(const upal_sync_group_t* group);
uetd_status_t upal_sync_group_start(upal_sync_group_t* group);
uetd_status_t upal_sync_group_stop(upal_sync_group_t* group);

typedef enum {
    UPAL_USB_PROFILE_CDC_ACM = 0,
    UPAL_USB_PROFILE_HID,
} upal_usb_profile_kind_t;

typedef struct {
    upal_usb_profile_kind_t kind;
    uetd_u16_t vendor_id;
    uetd_u16_t product_id;
    uetd_u16_t max_packet_bytes;
    uetd_u16_t poll_interval_us;
    const char* manufacturer;
    const char* product;
    const char* serial_number;
    bool identity_is_project_owned;
    uetd_u8_t endpoint_count;
    uetd_u16_t endpoint_budget_bytes;
    uetd_u16_t report_bytes;
} upal_usb_profile_config_t;

uetd_status_t upal_usb_start_profile(upal_usb_t* usb, const upal_usb_profile_config_t* config);
uetd_status_t upal_usb_send_report(upal_usb_t* usb, const uetd_u8_t* report, uetd_u16_t report_bytes);
uetd_status_t upal_usb_stop_profile(upal_usb_t* usb);

typedef struct {
    uetd_u16_t maximum_frame_bytes;
    uetd_u16_t rx_descriptor_count;
    uetd_u16_t tx_descriptor_count;
    uetd_u16_t cache_line_bytes;
    uetd_u8_t* rx_descriptor_storage;
    uetd_u8_t* tx_descriptor_storage;
    uetd_u16_t rx_buffer_bytes;
    uetd_u16_t tx_buffer_bytes;
    bool request_hardware_timestamps;
} upal_ethernet_config_t;

uetd_status_t upal_ethernet_init(upal_ethernet_t* ethernet, const upal_ethernet_config_t* config);
uetd_status_t upal_ethernet_link_is_up(upal_ethernet_t* ethernet, bool* is_up);
uetd_status_t upal_ethernet_transmit(upal_ethernet_t* ethernet, const uetd_u8_t* frame, uetd_u16_t frame_bytes);
uetd_status_t upal_ethernet_receive(upal_ethernet_t* ethernet, uetd_u8_t* frame, uetd_u16_t capacity, uetd_u16_t* received_bytes);
uetd_status_t upal_ethernet_deinit(upal_ethernet_t* ethernet);

#endif /* UPAL_FUTURE_CAPABILITY_CONTRACTS_H */
