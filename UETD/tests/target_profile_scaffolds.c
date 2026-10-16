#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <uhal/target_profiles.h>

typedef struct {
    const char* key;
    uetd_target_profile_id_t id;
    uetd_u8_t phase;
} expected_profile_t;

static const expected_profile_t expected[] = {
    {"stm32g4xx", UETD_TARGET_PROFILE_STM32G4, 0u},
    {"stm32f4xx", UETD_TARGET_PROFILE_STM32F4, 0u},
    {"stm32h7xx-single-core", UETD_TARGET_PROFILE_STM32H7, 0u},
    {"stm32f7xx", UETD_TARGET_PROFILE_STM32F7, 0u},
    {"stm32g0xx", UETD_TARGET_PROFILE_STM32G0, 0u},
    {"nrf52840", UETD_TARGET_PROFILE_NRF52840, 0u},
    {"stm32l4plus", UETD_TARGET_PROFILE_STM32L4PLUS, 1u},
};

static bool profile_is_listed(const uetd_target_profile_t* const* profiles, size_t count,
                              const uetd_target_profile_t* profile) {
    size_t index;
    if (profiles == NULL || profile == NULL) { return false; }
    for (index = 0u; index < count; ++index) {
        if (profiles[index] == profile) { return true; }
    }
    return false;
}

int main(void) {
    size_t failures = 0u;
    size_t all_count = 0u;
    size_t phase0_count = 0u;
    size_t phase1_count = 0u;
    const uetd_target_profile_t* const* all = uetd_target_profiles(&all_count);
    const uetd_target_profile_t* const* phase0 = uetd_target_phase0_profiles(&phase0_count);
    const uetd_target_profile_t* const* phase1 = uetd_target_phase1_profiles(&phase1_count);
    size_t index;

    if (all == NULL || all_count != sizeof(expected) / sizeof(expected[0])) {
        printf("FAIL target profile registry must expose all %zu expected profiles\n",
               sizeof(expected) / sizeof(expected[0]));
        ++failures;
    }
    if (phase0 == NULL || phase0_count != 6u || phase1 == NULL || phase1_count != 1u) {
        printf("FAIL target profile phase partitions must contain six Phase 0 and one Phase 1 record\n");
        ++failures;
    }
    for (index = 0u; index < sizeof(expected) / sizeof(expected[0]); ++index) {
        const uetd_target_profile_t* profile = uetd_target_profile_find(expected[index].key);
        if (profile == NULL || profile->id != expected[index].id || profile->phase != expected[index].phase ||
            profile->family_key == NULL || strcmp(profile->family_key, expected[index].key) != 0 ||
            profile->core == NULL || profile->core[0] == '\0' || profile->integration_notes == NULL ||
            profile->integration_notes[0] == '\0' || profile->profile_complete) {
            printf("FAIL target profile %s is missing, inconsistent, or marked complete\n", expected[index].key);
            ++failures;
        }
        if (expected[index].phase == 0u) {
            if (!profile_is_listed(phase0, phase0_count, profile)) {
                printf("FAIL target profile %s is missing from Phase 0\n", expected[index].key);
                ++failures;
            }
        } else if (!profile_is_listed(phase1, phase1_count, profile)) {
            printf("FAIL target profile %s is missing from Phase 1\n", expected[index].key);
            ++failures;
        }
    }
    if (uetd_target_profile_find(NULL) != NULL || uetd_target_profile_find("unknown-family") != NULL) {
        printf("FAIL target profile lookup must reject null and unknown family keys\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu target profile scaffold check(s) failed\n", failures);
        return 1;
    }
    printf("Validated %zu incomplete target-family profiles and their phase partitions\n", all_count);
    return 0;
}
