/// @file include/umid/drivers/encoder.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UMID_ENCODER_H
#define UMID_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/timing/timer.h"

typedef enum {
    UMID_ENC_QUADRATURE, /* AB quadrature */
    UMID_ENC_HALL,       /* hall-effect 3-phase */
    UMID_ENC_SINGLEEDGE, /* pulse counting */
} umid_encoder_type_t;

typedef struct {
    upal_timer_t* timer; /* configured in encoder input mode */
    umid_encoder_type_t type;
    uetd_i32_t counts_per_rev;
    uetd_f32_t gear_ratio;
    /* Velocity estimation */
    uetd_u32_t vel_filter_taps;
} umid_encoder_t;

/// Initialize the encoder measurement adapter.
uetd_status_t umid_encoder_init(umid_encoder_t* enc);
/// Return the current encoder count.
uetd_i32_t umid_encoder_count(const umid_encoder_t* enc);
/// Return the encoder position in radians.
uetd_f32_t umid_encoder_position_rad(const umid_encoder_t* enc);
/// Return the encoder velocity in revolutions per second.
uetd_f32_t umid_encoder_velocity_rps(umid_encoder_t* enc);
/// Reset the encoder count and position reference.
void umid_encoder_reset(umid_encoder_t* enc);

#ifdef __cplusplus
}
#endif

#endif /* UMID_ENCODER_H */
