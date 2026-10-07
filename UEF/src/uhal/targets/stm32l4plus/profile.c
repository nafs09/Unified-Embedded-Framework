/// @file src/uhal/targets/stm32l4plus/profile.c
/// @brief Incomplete family metadata; no exact-part resources are asserted.
#include <uef/uhal/targets/stm32l4plus/profile.h>

const uef_target_profile_t UEF_TARGET_PROFILE_STM32L4PLUS_DATA = {
    .id = UEF_TARGET_PROFILE_STM32L4PLUS,
    .phase = 1U,
    .family_key = "stm32l4plus",
    .core = "Cortex-M4F",
    .integration_notes = "Phase 1 SDMMC scaffold. Resolve controller, bus width, DMA, clocks, pins, memory, startup, and package per exact part.",
    .profile_complete = false
};
