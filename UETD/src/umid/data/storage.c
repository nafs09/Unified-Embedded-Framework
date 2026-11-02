/// @file src/umid/data/storage.c
/// @brief Fail-closed entry points for the first-pass UMID transactional storage service.
#include "umid/data/storage.h"

uetd_status_t umid_store_init(umid_store_t* store, const umid_store_backend_t* backend) {
    (void)store;
    (void)backend;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_store_read(umid_store_t* store, const umid_store_key_t* key, uetd_u8_t* payload,
                              uetd_u32_t capacity, uetd_u32_t* payload_length, umid_store_record_t* metadata) {
    (void)store;
    (void)key;
    (void)payload;
    (void)capacity;
    (void)payload_length;
    (void)metadata;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_store_write_atomic(umid_store_t* store, const umid_store_key_t* key,
                                      const umid_store_record_t* record) {
    (void)store;
    (void)key;
    (void)record;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_store_erase(umid_store_t* store, const umid_store_key_t* key) {
    (void)store;
    (void)key;
    return UETD_NOT_SUPPORTED;
}