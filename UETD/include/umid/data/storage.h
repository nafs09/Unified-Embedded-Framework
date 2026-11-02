/// @file include/umid/data/storage.h
/// @brief First-pass UMID transactional storage contract.

#ifndef UMID_STORAGE_H
#define UMID_STORAGE_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UMID_STORE_CONFIGURATION = 0,
    UMID_STORE_CALIBRATION,
    UMID_STORE_FAULT_SUMMARY,
} umid_store_record_kind_t;

/* Every text field uses a fixed byte capacity and explicit byte length. The
 * store copies metadata; it never retains caller pointers after a call. */
#define UMID_STORE_KEY_CAPACITY 64u
#define UMID_STORE_SCHEMA_ID_CAPACITY 48u
#define UMID_STORE_OWNER_ID_CAPACITY 48u
#define UMID_STORE_TARGET_ID_CAPACITY 48u
#define UMID_STORE_DEVICE_ID_CAPACITY 64u
#define UMID_STORE_ENCODING_ID_CAPACITY 24u

typedef struct {
    char bytes[UMID_STORE_KEY_CAPACITY];
    uetd_u16_t length;
} umid_store_key_t;

typedef struct {
    char schema_id[UMID_STORE_SCHEMA_ID_CAPACITY];
    uetd_u16_t schema_id_length;
    uetd_u16_t schema_version;
    umid_store_record_kind_t kind;
    char owner_id[UMID_STORE_OWNER_ID_CAPACITY];
    uetd_u16_t owner_id_length;
    char target_id[UMID_STORE_TARGET_ID_CAPACITY];
    uetd_u16_t target_id_length;
    char device_identity[UMID_STORE_DEVICE_ID_CAPACITY];
    uetd_u16_t device_identity_length;
    char encoding_id[UMID_STORE_ENCODING_ID_CAPACITY];
    uetd_u16_t encoding_id_length;
    uetd_u32_t sequence;
    uetd_u32_t payload_length;
    uetd_u8_t payload_sha256[32];
    uetd_u32_t payload_crc32;
    const uetd_u8_t* payload;
} umid_store_record_t;

typedef struct {
    /* begin reserves an inactive generation for payload bytes only. */
    uetd_status_t (*begin_generation)(void* context, const umid_store_key_t* key, uetd_u32_t generation, uetd_u32_t total_length);
    /* Reads from the exact committed generation returned by read_metadata. */
    uetd_status_t (*read)(void* context, const umid_store_key_t* key, uetd_u32_t generation, uetd_u32_t offset,
                          uetd_u8_t* data, uetd_u32_t length);
    /* Returns committed metadata only; record->sequence identifies its generation. */
    uetd_status_t (*read_metadata)(void* context, const umid_store_key_t* key, umid_store_record_t* record);
    uetd_status_t (*write)(void* context, const umid_store_key_t* key, uetd_u32_t generation, uetd_u32_t offset,
                          const uetd_u8_t* data, uetd_u32_t length);
    uetd_status_t (*commit_generation)(void* context, const umid_store_key_t* key, uetd_u32_t generation, uetd_u32_t total_length,
                                      const umid_store_record_t* record);
    uetd_status_t (*abort_generation)(void* context, const umid_store_key_t* key, uetd_u32_t generation);
    uetd_status_t (*erase)(void* context, const umid_store_key_t* key);
    void* context;
    uetd_u32_t maximum_record_bytes;
    uetd_u32_t required_alignment;
    uetd_u32_t atomic_commit_bytes;
    uetd_u32_t maximum_write_stall_us;
} umid_store_backend_t;

typedef struct {
    /* The callback table and context are borrowed for the store lifetime. */
    umid_store_backend_t backend;
    bool initialized;
    bool transaction_active;
    umid_store_key_t active_key; /* single serialized transaction per store */
    uetd_u32_t active_generation;
} umid_store_t;

/* On read, metadata->payload aliases the caller's payload buffer and remains
 * valid only for that caller-owned buffer's lifetime. Store backends expose
 * metadata and payload from one committed generation. */
/// Initialize the storage adapter with the selected backend.
uetd_status_t umid_store_init(umid_store_t* store, const umid_store_backend_t* backend);
/// Read a stored record into the caller-provided payload buffer.
uetd_status_t umid_store_read(umid_store_t* store, const umid_store_key_t* key, uetd_u8_t* payload, uetd_u32_t capacity,
                             uetd_u32_t* payload_length, umid_store_record_t* metadata);
/// Atomically write a record to the store, replacing the previous value.
uetd_status_t umid_store_write_atomic(umid_store_t* store, const umid_store_key_t* key, const umid_store_record_t* record);
/// Erase the record identified by the supplied key.
uetd_status_t umid_store_erase(umid_store_t* store, const umid_store_key_t* key);

#ifdef __cplusplus
}
#endif

#endif /* UMID_STORAGE_H */