/// @file src/uhal/targets/stm32g0/profile.c
/// @brief Incomplete family metadata; no exact-part resources are asserted.
#include <uhal/targets/stm32g0/profile.h>

const uetd_target_profile_t UETD_TARGET_PROFILE_STM32G0_DATA = {
    .id = UETD_TARGET_PROFILE_STM32G0,
    .phase = 0U,
    .family_key = "stm32g0xx",
    .core = "Cortex-M0+",
    .integration_notes = "Resolve DMAMUX, limited RAM, no-FPU arithmetic policy, peripherals, clocks, and package pins.",
    .profile_complete = false};
