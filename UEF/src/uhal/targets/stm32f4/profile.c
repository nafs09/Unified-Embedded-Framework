/// @file src/uhal/targets/stm32f4/profile.c
/// @brief Incomplete family metadata; no exact-part resources are asserted.
#include <uef/uhal/targets/stm32f4/profile.h>

const uef_target_profile_t UEF_TARGET_PROFILE_STM32F4_DATA = {
    .id = UEF_TARGET_PROFILE_STM32F4,
    .phase = 0U,
    .family_key = "stm32f4xx",
    .core = "Cortex-M4F",
    .integration_notes = "DMA routing, bxCAN/FDCAN availability, clocks, memory, and package pins vary by exact part.",
    .profile_complete = false
};
