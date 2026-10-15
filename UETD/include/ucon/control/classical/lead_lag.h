/// @file include/ucon/control/classical/lead_lag.h
/// @brief First-order bilinear-transform lead-lag compensator.

#ifndef UETD_UCON_CONTROL_LEAD_LAG_H
#define UETD_UCON_CONTROL_LEAD_LAG_H

#include <ucon/status.h>
#include <umath/scalar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ucon_scalar_t gain;
    ucon_scalar_t lead_time;
    ucon_scalar_t lag_time;
    ucon_scalar_t sample_time;
} ucon_lead_lag_config_t;

typedef struct {
    ucon_scalar_t previous_input;
    ucon_scalar_t previous_output;
    bool initialized;
} ucon_lead_lag_state_t;

ucon_status_t ucon_lead_lag_init(ucon_lead_lag_state_t* state, const ucon_lead_lag_config_t* config);
ucon_status_t ucon_lead_lag_reset(ucon_lead_lag_state_t* state);
ucon_status_t ucon_lead_lag_step(ucon_lead_lag_state_t* state, const ucon_lead_lag_config_t* config, ucon_scalar_t input,
                                 ucon_scalar_t* output);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UCON_CONTROL_LEAD_LAG_H */
