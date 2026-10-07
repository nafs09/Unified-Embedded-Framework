/// @file src/uhal/targets/stm32f7/profile.c
/// @brief Incomplete family metadata; no exact-part resources are asserted.
#include <uef/uhal/targets/stm32f7/profile.h>

const uef_target_profile_t UEF_TARGET_PROFILE_STM32F7_DATA = {
    .id = UEF_TARGET_PROFILE_STM32F7,
    .phase = 0U,
    .family_key = "stm32f7xx",
    .core = "Cortex-M7",
    .integration_notes = "Resolve per-part cache, USB HS routing, SDMMC, DMA, clocks, IRQs, and package pins.",
    .profile_complete = false
};
