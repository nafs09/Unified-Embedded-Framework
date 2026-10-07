/// @file src/ucon/control/lead_lag.c
/// @brief Discrete lead-lag recurrence using the bilinear coefficients in Part XVI.

#include <string.h>
#include <uef/ucon/control/lead_lag.h>

static bool lead_lag_config_valid(const ucon_lead_lag_config_t *config)
{
    return config != NULL &&
        umath_scalar_is_finite(config->gain) &&
        umath_scalar_is_finite(config->lead_time) &&
        umath_scalar_is_finite(config->lag_time) &&
        umath_scalar_is_finite(config->sample_time) &&
        config->lead_time > (ucon_scalar_t)0 &&
        config->lag_time > (ucon_scalar_t)0 &&
        config->sample_time > (ucon_scalar_t)0;
}

ucon_status_t ucon_lead_lag_init(ucon_lead_lag_state_t *state,
                                 const ucon_lead_lag_config_t *config)
{
    if (state == NULL || !lead_lag_config_valid(config)) {
        return UCON_INVALID_ARGUMENT;
    }
    memset(state, 0, sizeof(*state));
    state->initialized = true;
    return UCON_OK;
}

ucon_status_t ucon_lead_lag_reset(ucon_lead_lag_state_t *state)
{
    if (state == NULL) {
        return UCON_INVALID_ARGUMENT;
    }
    memset(state, 0, sizeof(*state));
    state->initialized = true;
    return UCON_OK;
}

ucon_status_t ucon_lead_lag_step(ucon_lead_lag_state_t *state,
                                 const ucon_lead_lag_config_t *config,
                                 ucon_scalar_t input,
                                 ucon_scalar_t *output)
{
    ucon_scalar_t lead_denominator, lag_denominator;
    ucon_scalar_t a, b, bilinear_gain, candidate;

    if (state == NULL || output == NULL || !state->initialized ||
        !lead_lag_config_valid(config) || !umath_scalar_is_finite(input)) {
        return UCON_INVALID_ARGUMENT;
    }

    lead_denominator = config->sample_time +
                       (ucon_scalar_t)2 * config->lead_time;
    lag_denominator = config->sample_time +
                      (ucon_scalar_t)2 * config->lag_time;
    if (!umath_scalar_is_finite(lead_denominator) ||
        !umath_scalar_is_finite(lag_denominator) ||
        lead_denominator <= (ucon_scalar_t)0 ||
        lag_denominator <= (ucon_scalar_t)0) {
        return UCON_NUMERIC_FAILURE;
    }

    a = (config->sample_time -
         (ucon_scalar_t)2 * config->lead_time) / lead_denominator;
    b = (config->sample_time -
         (ucon_scalar_t)2 * config->lag_time) / lag_denominator;
    /* Bilinear substitution changes the static gain as well as the poles/zeros. */
    bilinear_gain = config->gain * lead_denominator / lag_denominator;
    if (!umath_scalar_is_finite(a) || !umath_scalar_is_finite(b) ||
        !umath_scalar_is_finite(bilinear_gain)) {
        return UCON_NUMERIC_FAILURE;
    }

    /* H(z)=Kd(z+a)/(z+b), represented as a recurrence in z^-1. */
    candidate = -b * state->previous_output +
                bilinear_gain * (input + a * state->previous_input);
    if (!umath_scalar_is_finite(candidate)) {
        return UCON_NUMERIC_FAILURE;
    }

    state->previous_input = input;
    state->previous_output = candidate;
    *output = candidate;
    return UCON_OK;
}
