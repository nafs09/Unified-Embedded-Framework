/// @file include/umid/data/calibration.h
/// @brief First-pass UMID calibration record contract.

#ifndef UMID_CALIBRATION_H
#define UMID_CALIBRATION_H

#include "umid/data/storage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMID_CAL_DRIVER_ID_CAPACITY 48u
#define UMID_CAL_SENSOR_ID_CAPACITY 64u
#define UMID_CAL_BOARD_ID_CAPACITY 48u
#define UMID_CAL_UNITS_ID_CAPACITY 24u
#define UMID_CAL_METHOD_ID_CAPACITY 32u

typedef struct {
    char driver_id[UMID_CAL_DRIVER_ID_CAPACITY];
    uetd_u16_t driver_id_length;
    char sensor_identity[UMID_CAL_SENSOR_ID_CAPACITY];
    uetd_u16_t sensor_identity_length;
    char board_id[UMID_CAL_BOARD_ID_CAPACITY];
    uetd_u16_t board_id_length;
    char units_id[UMID_CAL_UNITS_ID_CAPACITY];
    uetd_u16_t units_id_length;
    uetd_u16_t calibration_schema_version;
    char method_id[UMID_CAL_METHOD_ID_CAPACITY];
    uetd_u16_t method_id_length;
    uetd_u16_t method_revision;
    uetd_u32_t clock_id;
    uetd_u64_t calibrated_at_us;
    uetd_u32_t payload_length;
    const uetd_u8_t* opaque_driver_payload;
    uetd_u32_t payload_crc32;
    uetd_u8_t payload_sha256[32];
} umid_calibration_record_t;

/* save consumes the opaque payload synchronously. load returns the metadata
 * separately so the driver can verify board, units, method, and sensor identity
 * before applying its own calibration interpretation. */
/// Save a calibration record to the persistent store.
uetd_status_t umid_calibration_store_save(umid_store_t* store, const umid_calibration_record_t* record);
/// Load a matching calibration record from the persistent store.
uetd_status_t umid_calibration_store_load(umid_store_t* store, const char* driver_id, const char* sensor_identity,
                                         uetd_u16_t expected_schema_version, uetd_u8_t* payload, uetd_u32_t capacity,
                                         uetd_u32_t* payload_length, umid_calibration_record_t* metadata);

#ifdef __cplusplus
}
#endif

#endif /* UMID_CALIBRATION_H */