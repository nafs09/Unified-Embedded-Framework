/// @file include/uef/ucon/control/pid.h
/// @brief Generic allocation-free 2-DOF PID/PI/PD controller.

#ifndef UEF_UCON_CONTROL_PID_H
#define UEF_UCON_CONTROL_PID_H

#include <uef/ucon/ucon_status.h>
#include <uef/umath/scalar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UCON_ANTI_WINDUP_NONE = 0,
    UCON_ANTI_WINDUP_CLAMP,
    UCON_ANTI_WINDUP_BACK_CALCULATION,
    UCON_ANTI_WINDUP_CONDITIONAL
} ucon_anti_windup_t;

/// Runtime tuning and fixed sample period for one PID instance.
typedef struct {
    ucon_scalar_t kp;
    ucon_scalar_t ki;
    ucon_scalar_t kd;
    ucon_scalar_t derivative_filter_n;
    ucon_scalar_t proportional_weight_b;
    ucon_scalar_t derivative_weight_c;
    ucon_scalar_t sample_time;
    ucon_scalar_t output_min;
    ucon_scalar_t output_max;
    ucon_scalar_t integral_min;
    ucon_scalar_t integral_max;
    ucon_scalar_t tracking_time;
    bool saturation_enabled;
    ucon_anti_windup_t anti_windup;
} ucon_pid_config_t;

/// Persistent state. Initialize only through ucon_pid_init().
typedef struct {
    ucon_scalar_t previous_integral_error;
    ucon_scalar_t previous_derivative_error;
    ucon_scalar_t integral_state;
    ucon_scalar_t derivative_state;
    bool initialized;
} ucon_pid_state_t;

/// Validate configuration and clear dynamic state.
ucon_status_t ucon_pid_init(ucon_pid_state_t *state,
                            const ucon_pid_config_t *config);

/// Clear dynamic state while preserving the caller-owned configuration.
ucon_status_t ucon_pid_reset(ucon_pid_state_t *state);

/// Run one discrete controller sample. tracking_output may be NULL, in which
/// case back-calculation tracks the controller's own saturated output. State,
/// configuration, tracking input, and output storage must not overlap.
ucon_status_t ucon_pid_step(ucon_pid_state_t *state,
                            const ucon_pid_config_t *config,
                            ucon_scalar_t reference,
                            ucon_scalar_t measurement,
                            ucon_scalar_t feedforward,
                            const ucon_scalar_t *tracking_output,
                            ucon_scalar_t *output);

#ifdef __cplusplus
}
#endif

#endif /* UEF_UCON_CONTROL_PID_H */
