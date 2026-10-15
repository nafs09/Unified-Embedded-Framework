/// @file src/uhal/targets/stm32g4/profile.c
/// @brief Incomplete family metadata; no exact-part resources are asserted.
#include <uhal/targets/stm32g4/profile.h>

const uetd_target_profile_t UETD_TARGET_PROFILE_STM32G4_DATA = {
    .id = UETD_TARGET_PROFILE_STM32G4,
    .phase = 0U,
    .family_key = "stm32g4xx",
    .core = "Cortex-M4F",
    .integration_notes = "DMAMUX, HRTIM, FDCAN, CORDIC, FMAC, OPAMP, clocks, and package pins vary by exact part.",
    .profile_complete = false};
