/// @file src/uhal/target_profiles.c
/// @brief Central registration and lookup for Phase 0/1 target-family scaffolds.
#include <string.h>
#include <uhal/target_profiles.h>
#include <uhal/targets/nrf52840/profile.h>
#include <uhal/targets/stm32f4/profile.h>
#include <uhal/targets/stm32f7/profile.h>
#include <uhal/targets/stm32g0/profile.h>
#include <uhal/targets/stm32g4/profile.h>
#include <uhal/targets/stm32h7/profile.h>
#include <uhal/targets/stm32l4plus/profile.h>

static const uetd_target_profile_t* const phase0_profiles[] = {
    &UETD_TARGET_PROFILE_STM32G4_DATA, &UETD_TARGET_PROFILE_STM32F4_DATA, &UETD_TARGET_PROFILE_STM32H7_DATA,
    &UETD_TARGET_PROFILE_STM32F7_DATA, &UETD_TARGET_PROFILE_STM32G0_DATA, &UETD_TARGET_PROFILE_NRF52840_DATA};

static const uetd_target_profile_t* const phase1_profiles[] = {&UETD_TARGET_PROFILE_STM32L4PLUS_DATA};

static const uetd_target_profile_t* const all_profiles[] = {&UETD_TARGET_PROFILE_STM32G4_DATA,    &UETD_TARGET_PROFILE_STM32F4_DATA,
                                                           &UETD_TARGET_PROFILE_STM32H7_DATA,    &UETD_TARGET_PROFILE_STM32F7_DATA,
                                                           &UETD_TARGET_PROFILE_STM32G0_DATA,    &UETD_TARGET_PROFILE_NRF52840_DATA,
                                                           &UETD_TARGET_PROFILE_STM32L4PLUS_DATA};

#define UETD_ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

_Static_assert(UETD_ARRAY_COUNT(all_profiles) == UETD_TARGET_PROFILE_COUNT, "Each target profile ID must have one registry entry");

const uetd_target_profile_t* const* uetd_target_phase0_profiles(size_t* count) {
    if (count != NULL) { *count = UETD_ARRAY_COUNT(phase0_profiles); }
    return phase0_profiles;
}

const uetd_target_profile_t* const* uetd_target_phase1_profiles(size_t* count) {
    if (count != NULL) { *count = UETD_ARRAY_COUNT(phase1_profiles); }
    return phase1_profiles;
}

const uetd_target_profile_t* const* uetd_target_profiles(size_t* count) {
    if (count != NULL) { *count = UETD_ARRAY_COUNT(all_profiles); }
    return all_profiles;
}

const uetd_target_profile_t* uetd_target_profile_find(const char* family_key) {
    size_t index;
    if (family_key == NULL) { return NULL; }
    for (index = 0U; index < UETD_ARRAY_COUNT(all_profiles); ++index) {
        if (strcmp(family_key, all_profiles[index]->family_key) == 0) { return all_profiles[index]; }
    }
    return NULL;
}
