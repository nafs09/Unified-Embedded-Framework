/// @file src/uhal/targets/stm32h7/profile.c
/// @brief Incomplete family metadata; no exact-part resources are asserted.
#include <uef/uhal/targets/stm32h7/profile.h>

const uef_target_profile_t UEF_TARGET_PROFILE_STM32H7_DATA = {
    .id = UEF_TARGET_PROFILE_STM32H7,
    .phase = 0U,
    .family_key = "stm32h7xx-single-core",
    .core = "Cortex-M7 single-core",
    .integration_notes = "This record covers only single-core M7 parts; dual-core M7+M4 parts need a separate profile. Resolve BDMA, AXI SRAM MPU, D-cache, and flash.",
    .profile_complete = false
};
