/// @file src/uhal/targets/nrf52840/profile.c
/// @brief Incomplete family metadata; no exact-part resources are asserted.
#include <uef/uhal/targets/nrf52840/profile.h>

const uef_target_profile_t UEF_TARGET_PROFILE_NRF52840_DATA = {
    .id = UEF_TARGET_PROFILE_NRF52840,
    .phase = 0U,
    .family_key = "nrf52840",
    .core = "Cortex-M4F",
    .integration_notes = "Resolve Nordic peripherals, radio coexistence, pins, clocks, memory, startup, and package.",
    .profile_complete = false
};
