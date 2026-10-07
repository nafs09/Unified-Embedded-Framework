/// @file include/uef/uproto/uproto_ppm.h
/// @brief Generic pulse-width decoder for PPM receiver input.

#ifndef UPROTO_PPM_H
#define UPROTO_PPM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_status.h"
#include "uef/ucore/uef_types.h"

#define UPROTO_PPM_MAX_CHANNELS 16U

/// Timing and frame-size policy supplied for the connected receiver.
typedef struct {
    uef_u16_t min_pulse_us;
    uef_u16_t max_pulse_us;
    uef_u16_t sync_gap_us;
    uef_u8_t min_channels;
    uef_u8_t max_channels;
} uproto_ppm_config_t;

/// Last complete decoded pulse frame.
typedef struct {
    uef_u16_t channels_us[UPROTO_PPM_MAX_CHANNELS];
    uef_u8_t channel_count;
    uef_u32_t sequence;
    uef_u64_t timestamp_us;
} uproto_ppm_frame_t;

/// Fixed-memory decoder state; no allocation or target timer dependency.
typedef struct {
    uproto_ppm_config_t config;
    uef_u16_t working_channels_us[UPROTO_PPM_MAX_CHANNELS];
    uef_u8_t working_count;
    bool synchronized;
    bool frame_available;
    uproto_ppm_frame_t latest_frame;
} uproto_ppm_t;

/// Initialize one decoder using the receiver's documented pulse and sync ranges.
uef_status_t uproto_ppm_init(uproto_ppm_t* decoder,
                             const uproto_ppm_config_t* config);

/// Feed the elapsed microseconds between successive edges of the same polarity.
///
/// Call from one serialized context. If the capture interrupt and application
/// task are separate, defer this call to a task or protect all decoder access
/// with the same target critical section/queue policy.
uef_status_t uproto_ppm_capture_interval(uproto_ppm_t* decoder,
                                         uef_u32_t interval_us,
                                         uef_u64_t timestamp_us);

bool uproto_ppm_available(const uproto_ppm_t* decoder);
uef_status_t uproto_ppm_get_frame(uproto_ppm_t* decoder,
                                  uproto_ppm_frame_t* frame);
void uproto_ppm_reset(uproto_ppm_t* decoder);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_PPM_H */
