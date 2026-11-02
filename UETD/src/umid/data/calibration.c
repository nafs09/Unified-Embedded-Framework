/// @file src/umid/data/calibration.c
/// @brief Fail-closed entry points for the first-pass UMID calibration persistence service.
#include "umid/data/calibration.h"

uetd_status_t umid_calibration_store_save(umid_store_t* store, const umid_calibration_record_t* record) {
    (void)store;
    (void)record;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_calibration_store_load(umid_store_t* store, const char* driver_id,
                                          const char* sensor_identity, uetd_u16_t expected_schema_version,
                                          uetd_u8_t* payload, uetd_u32_t capacity, uetd_u32_t* payload_length,
                                          umid_calibration_record_t* metadata) {
    (void)store;
    (void)driver_id;
    (void)sensor_identity;
    (void)expected_schema_version;
    (void)payload;
    (void)capacity;
    (void)payload_length;
    (void)metadata;
    return UETD_NOT_SUPPORTED;
}