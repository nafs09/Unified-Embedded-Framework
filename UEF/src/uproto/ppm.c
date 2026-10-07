/// @file src/uproto/ppm.c
/// @brief Bounded, fixed-memory PPM interval decoder independent of timer hardware.

#include "uef/uproto/uproto_ppm.h"

#include <string.h>

uef_status_t uproto_ppm_init(uproto_ppm_t* decoder,
                             const uproto_ppm_config_t* config)
{
    if (decoder == NULL || config == NULL ||
        config->min_pulse_us == 0U ||
        config->min_pulse_us > config->max_pulse_us ||
        config->max_pulse_us >= config->sync_gap_us ||
        config->min_channels == 0U ||
        config->min_channels > config->max_channels ||
        config->max_channels > UPROTO_PPM_MAX_CHANNELS) {
        return UEF_INVALID_ARG;
    }

    memset(decoder, 0, sizeof(*decoder));
    decoder->config = *config;
    return UEF_OK;
}

uef_status_t uproto_ppm_capture_interval(uproto_ppm_t* decoder,
                                         uef_u32_t interval_us,
                                         uef_u64_t timestamp_us)
{
    if (decoder == NULL || decoder->config.min_channels == 0U) {
        return UEF_INVALID_ARG;
    }

    if (interval_us >= decoder->config.sync_gap_us) {
        const bool complete = decoder->working_count >= decoder->config.min_channels &&
                              decoder->working_count <= decoder->config.max_channels;
        if (complete) {
            const uef_u32_t next_sequence = decoder->latest_frame.sequence + 1U;
            memset(&decoder->latest_frame, 0, sizeof(decoder->latest_frame));
            memcpy(decoder->latest_frame.channels_us,
                   decoder->working_channels_us,
                   (size_t)decoder->working_count * sizeof(decoder->working_channels_us[0]));
            decoder->latest_frame.channel_count = decoder->working_count;
            decoder->latest_frame.sequence = next_sequence;
            decoder->latest_frame.timestamp_us = timestamp_us;
            decoder->frame_available = true;
        }
        decoder->working_count = 0U;
        decoder->synchronized = true;
        return complete ? UEF_OK : UEF_NOT_READY;
    }

    if (interval_us < decoder->config.min_pulse_us ||
        interval_us > decoder->config.max_pulse_us) {
        decoder->working_count = 0U;
        decoder->synchronized = false;
        return UEF_INVALID_ARG;
    }
    if (!decoder->synchronized) {
        return UEF_NOT_READY;
    }
    if (decoder->working_count >= decoder->config.max_channels) {
        decoder->working_count = 0U;
        decoder->synchronized = false;
        return UEF_OVERFLOW;
    }

    decoder->working_channels_us[decoder->working_count++] = (uef_u16_t)interval_us;
    return UEF_OK;
}

bool uproto_ppm_available(const uproto_ppm_t* decoder)
{
    return decoder != NULL && decoder->frame_available;
}

uef_status_t uproto_ppm_get_frame(uproto_ppm_t* decoder,
                                  uproto_ppm_frame_t* frame)
{
    if (decoder == NULL || frame == NULL) {
        return UEF_INVALID_ARG;
    }
    if (!decoder->frame_available) {
        return UEF_NOT_READY;
    }

    *frame = decoder->latest_frame;
    decoder->frame_available = false;
    return UEF_OK;
}

void uproto_ppm_reset(uproto_ppm_t* decoder)
{
    if (decoder == NULL) {
        return;
    }
    decoder->working_count = 0U;
    decoder->synchronized = false;
    decoder->frame_available = false;
    memset(decoder->working_channels_us, 0, sizeof(decoder->working_channels_us));
    memset(&decoder->latest_frame, 0, sizeof(decoder->latest_frame));
}
