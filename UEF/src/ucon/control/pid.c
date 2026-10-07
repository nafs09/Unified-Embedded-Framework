/// @file src/ucon/control/pid.c
/// @brief Tustin 2-DOF PID with filtered derivative and explicit anti-windup.

#include <string.h>
#include <uef/ucon/control/pid.h>

static bool pid_config_valid(const ucon_pid_config_t *config)
{
    if (config == NULL ||
        !umath_scalar_is_finite(config->kp) ||
        !umath_scalar_is_finite(config->ki) ||
        !umath_scalar_is_finite(config->kd) ||
        !umath_scalar_is_finite(config->derivative_filter_n) ||
        !umath_scalar_is_finite(config->proportional_weight_b) ||
        !umath_scalar_is_finite(config->derivative_weight_c) ||
        !umath_scalar_is_finite(config->sample_time) ||
        !umath_scalar_is_finite(config->output_min) ||
        !umath_scalar_is_finite(config->output_max) ||
        !umath_scalar_is_finite(config->integral_min) ||
        !umath_scalar_is_finite(config->integral_max) ||
        !umath_scalar_is_finite(config->tracking_time) ||
        config->sample_time <= (ucon_scalar_t)0 ||
        config->integral_min > config->integral_max ||
        config->anti_windup < UCON_ANTI_WINDUP_NONE ||
        config->anti_windup > UCON_ANTI_WINDUP_CONDITIONAL) {
        return false;
    }
    if (config->saturation_enabled &&
        config->output_min > config->output_max) {
        return false;
    }
    if (config->kd != (ucon_scalar_t)0 &&
        config->derivative_filter_n <= (ucon_scalar_t)0) {
        return false;
    }
    if (config->anti_windup == UCON_ANTI_WINDUP_BACK_CALCULATION &&
        config->ki != (ucon_scalar_t)0 &&
        config->tracking_time <= (ucon_scalar_t)0) {
        return false;
    }
    return true;
}

static ucon_scalar_t clamp_integral(const ucon_pid_config_t *config,
                                    ucon_scalar_t value)
{
    return value < config->integral_min ? config->integral_min :
           (value > config->integral_max ? config->integral_max : value);
}

ucon_status_t ucon_pid_init(ucon_pid_state_t *state,
                            const ucon_pid_config_t *config)
{
    if (state == NULL || !pid_config_valid(config)) {
        return UCON_INVALID_ARGUMENT;
    }
    memset(state, 0, sizeof(*state));
    state->initialized = true;
    return UCON_OK;
}

ucon_status_t ucon_pid_reset(ucon_pid_state_t *state)
{
    if (state == NULL) {
        return UCON_INVALID_ARGUMENT;
    }
    memset(state, 0, sizeof(*state));
    state->initialized = true;
    return UCON_OK;
}

ucon_status_t ucon_pid_step(ucon_pid_state_t *state,
                            const ucon_pid_config_t *config,
                            ucon_scalar_t reference,
                            ucon_scalar_t measurement,
                            ucon_scalar_t feedforward,
                            const ucon_scalar_t *tracking_output,
                            ucon_scalar_t *output)
{
    ucon_pid_state_t next;
    ucon_scalar_t proportional_error, integral_error, derivative_error;
    ucon_scalar_t proposed_integral, next_derivative;
    ucon_scalar_t raw_output, saturated_output, tracking;
    ucon_scalar_t derivative_a0, derivative_a1;

    if (state == NULL || output == NULL || !state->initialized ||
        !pid_config_valid(config) ||
        !umath_scalar_is_finite(reference) ||
        !umath_scalar_is_finite(measurement) ||
        !umath_scalar_is_finite(feedforward) ||
        (config->anti_windup == UCON_ANTI_WINDUP_BACK_CALCULATION &&
         tracking_output != NULL && !umath_scalar_is_finite(*tracking_output))) {
        return UCON_INVALID_ARGUMENT;
    }

    proportional_error =
        config->proportional_weight_b * reference - measurement;
    integral_error = reference - measurement;
    derivative_error =
        config->derivative_weight_c * reference - measurement;
    if (!umath_scalar_is_finite(proportional_error) ||
        !umath_scalar_is_finite(integral_error) ||
        !umath_scalar_is_finite(derivative_error)) {
        return UCON_NUMERIC_FAILURE;
    }

    next = *state;
    proposed_integral = next.integral_state +
        (config->sample_time * (integral_error +
                                next.previous_integral_error) /
         (ucon_scalar_t)2);
    if (!umath_scalar_is_finite(proposed_integral)) {
        return UCON_NUMERIC_FAILURE;
    }
    proposed_integral = clamp_integral(config, proposed_integral);

    if (config->kd == (ucon_scalar_t)0) {
        next_derivative = (ucon_scalar_t)0;
    } else {
        const ucon_scalar_t denominator =
            (ucon_scalar_t)2 +
            config->derivative_filter_n * config->sample_time;
        if (!(denominator > (ucon_scalar_t)0)) {
            return UCON_NUMERIC_FAILURE;
        }
        derivative_a0 =
            (ucon_scalar_t)2 * config->kd * config->derivative_filter_n /
            denominator;
        derivative_a1 =
            ((ucon_scalar_t)2 -
             config->derivative_filter_n * config->sample_time) /
            denominator;
        next_derivative = derivative_a1 * next.derivative_state +
            derivative_a0 *
            (derivative_error - next.previous_derivative_error);
        if (!umath_scalar_is_finite(next_derivative)) {
            return UCON_NUMERIC_FAILURE;
        }
    }

    raw_output = config->kp * proportional_error +
                 config->ki * proposed_integral +
                 next_derivative + feedforward;
    if (!umath_scalar_is_finite(raw_output)) {
        return UCON_NUMERIC_FAILURE;
    }

    /* Conditional integration rejects only increments that deepen saturation. */
    if (config->saturation_enabled &&
        config->anti_windup == UCON_ANTI_WINDUP_CONDITIONAL) {
        const ucon_scalar_t integral_output_delta =
            config->ki * (proposed_integral - next.integral_state);
        if (!umath_scalar_is_finite(integral_output_delta)) {
            return UCON_NUMERIC_FAILURE;
        }
        const bool drives_high =
            raw_output > config->output_max &&
            integral_output_delta > (ucon_scalar_t)0;
        const bool drives_low =
            raw_output < config->output_min &&
            integral_output_delta < (ucon_scalar_t)0;
        if (drives_high || drives_low) {
            proposed_integral = next.integral_state;
            raw_output = config->kp * proportional_error +
                         config->ki * proposed_integral +
                         next_derivative + feedforward;
            if (!umath_scalar_is_finite(raw_output)) {
                return UCON_NUMERIC_FAILURE;
            }
        }
    }

    saturated_output = raw_output;
    if (config->saturation_enabled) {
        saturated_output =
            raw_output < config->output_min ? config->output_min :
            (raw_output > config->output_max ? config->output_max : raw_output);
    }

    if (config->anti_windup == UCON_ANTI_WINDUP_BACK_CALCULATION &&
        config->ki != (ucon_scalar_t)0) {
        tracking = tracking_output != NULL ? *tracking_output : saturated_output;
        proposed_integral +=
            config->sample_time / (config->tracking_time * config->ki) *
            (tracking - raw_output);
        if (!umath_scalar_is_finite(proposed_integral)) {
            return UCON_NUMERIC_FAILURE;
        }
        proposed_integral = clamp_integral(config, proposed_integral);
    }

    next.integral_state = proposed_integral;
    next.derivative_state = next_derivative;
    next.previous_integral_error = integral_error;
    next.previous_derivative_error = derivative_error;
    if (!umath_scalar_is_finite(saturated_output)) {
        return UCON_NUMERIC_FAILURE;
    }

    *state = next;
    *output = saturated_output;
    return UCON_OK;
}
