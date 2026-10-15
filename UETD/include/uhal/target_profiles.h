/// @file include/uhal/target_profiles.h
/// @brief Queryable metadata for incomplete UETD target-family profile scaffolds.
#ifndef UETD_UHAL_TARGET_PROFILES_H
#define UETD_UHAL_TARGET_PROFILES_H

#include <ucore/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Identifiers for target-family profiles registered in the portable UHAL layer.
typedef enum {
    UETD_TARGET_PROFILE_STM32G4 = 0,
    UETD_TARGET_PROFILE_STM32F4,
    UETD_TARGET_PROFILE_STM32H7,
    UETD_TARGET_PROFILE_STM32F7,
    UETD_TARGET_PROFILE_STM32G0,
    UETD_TARGET_PROFILE_NRF52840,
    UETD_TARGET_PROFILE_STM32L4PLUS,
    UETD_TARGET_PROFILE_COUNT
} uetd_target_profile_id_t;

/// Descriptive family metadata; this is not a verified exact-part board profile.
typedef struct {
    uetd_target_profile_id_t id;
    uetd_u8_t phase;
    const char* family_key;
    const char* core;
    const char* integration_notes;
    bool profile_complete;
} uetd_target_profile_t;

/// Return Phase 0 profiles and write their count when non-null.
const uetd_target_profile_t* const* uetd_target_phase0_profiles(size_t* count);

/// Return Phase 1 profiles and write their count when non-null.
const uetd_target_profile_t* const* uetd_target_phase1_profiles(size_t* count);

/// Return every profile compiled into UHAL and write its count when non-null.
const uetd_target_profile_t* const* uetd_target_profiles(size_t* count);

/// Find a registered Phase 0/1 family profile by its canonical key.
const uetd_target_profile_t* uetd_target_profile_find(const char* family_key);

#ifdef __cplusplus
}
#endif

#endif
