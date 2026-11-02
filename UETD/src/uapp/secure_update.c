/// @file src/uapp/secure_update.c
#include "uapp/secure_update.h"
#include "ucore/checksum.h"
#include <string.h>
#define POLICY_MASK 31u
#define AUTH_REQUIREMENTS (UAPP_UPDATE_REQUIRE_AUTHENTICATION | UAPP_UPDATE_REQUIRE_RECOVERY | UAPP_UPDATE_REQUIRE_ANTI_ROLLBACK | UAPP_UPDATE_REQUIRE_ENCRYPTION)
static void put(uetd_u8_t** p, uetd_u64_t value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) { *(*p)++ = (uetd_u8_t)value; value >>= 8; }
}
uetd_u32_t uapp_update_progress_crc(const uapp_update_progress_t* r) {
    if (r == NULL) { return 0u; }
    uetd_u8_t bytes[UAPP_UPDATE_PROGRESS_BYTES - 4u], *p = bytes;
    put(&p, r->schema_version, 2); put(&p, r->record_bytes, 2); put(&p, r->sequence, 4);
    put(&p, r->state, 4); put(&p, r->policy, 4);
    memcpy(p, r->artifact_sha256, 32); p += 32; memcpy(p, r->manifest_sha256, 32); p += 32;
    put(&p, r->received_bytes, 4); put(&p, r->verified_segment_count, 2); put(&p, r->boot_attempt_count, 2);
    memcpy(p, r->staging_slot_id, 24); p += 24; memcpy(p, r->confirmed_slot_id, 24); p += 24;
    put(&p, r->last_error, 4);
    return uetd_crc32(bytes, sizeof(bytes), UINT32_MAX) ^ UINT32_MAX;
}
static bool text_present(const char* text) {
    if (text == NULL || text[0] == 0) { return false; }
    /* Provider additionally validates vocabulary/encoding. All strings are
     * borrowed C strings with a documented maximum length of 255 bytes. */
    for (size_t i = 1u; i < 256u; ++i) { if (text[i] == 0) { return true; } }
    return false;
}
static uetd_status_t validate(const uapp_update_backend_t* b, const uapp_update_descriptor_t* d,
                              uapp_update_policy_t requested, uapp_update_policy_t* policy) {
    if (b == NULL || d == NULL || (requested & ~POLICY_MASK) != 0u || (d->declared_policy & ~POLICY_MASK) != 0u ||
        !text_present(d->project_id) || !text_present(d->chip_id) || !text_present(d->board_id) ||
        !text_present(d->firmware_version) || !text_present(d->minimum_hardware_revision) ||
        !text_present(d->maximum_hardware_revision) || !text_present(d->canonicalization_id) ||
        d->manifest_schema_version != 1u || d->canonical_manifest == NULL || d->manifest_bytes == 0u ||
        d->manifest_bytes > UAPP_UPDATE_MAX_MANIFEST_BYTES || d->segments == NULL ||
        d->segment_count == 0u || d->segment_count > UAPP_UPDATE_MAX_SEGMENTS || d->total_image_bytes == 0u ||
        d->trial_health_window_ms == 0u) { return UETD_INVALID_ARG; }
    uapp_update_policy_t p = requested | d->declared_policy;
    if ((p & AUTH_REQUIREMENTS) != 0u) { p |= UAPP_UPDATE_REQUIRE_AUTHENTICATION; }
    if (((p & UAPP_UPDATE_REQUIRE_AUTHENTICATION) != 0u) == ((p & UAPP_UPDATE_ALLOW_UNSIGNED_DEVELOPMENT) != 0u)) {
        return UETD_INVALID_ARG;
    }
    const uapp_update_capabilities_t* c = &b->capabilities;
    if (!c->has_staging_region || !c->has_trial_boot || !c->has_rollback || c->maximum_image_bytes == 0u ||
        c->maximum_transfer_chunk_bytes == 0u || c->maximum_session_time_ms == 0u ||
        b->check_compatibility == NULL || b->begin_staging == NULL || b->write_staging == NULL ||
        b->verify_staged_image == NULL || b->select_trial_image == NULL || b->confirm_trial_image == NULL ||
        b->rollback_trial_image == NULL || b->read_progress == NULL || b->commit_progress_atomic == NULL ||
        b->now_us == NULL || b->hash_manifest == NULL || b->resume_staging == NULL || b->boot_status == NULL) {
        return UETD_NOT_SUPPORTED;
    }
    if (d->total_image_bytes > c->maximum_image_bytes) { return UETD_OVERFLOW; }
    if ((p & UAPP_UPDATE_REQUIRE_AUTHENTICATION) != 0u &&
        (!c->verifies_signatures || !c->protected_trust_anchor || b->verify_signature == NULL)) { return UETD_NOT_SUPPORTED; }
    if ((p & UAPP_UPDATE_REQUIRE_AUTHENTICATION) != 0u &&
        (d->signature == NULL || d->signature_bytes == 0u || !text_present(d->signature_algorithm) ||
         !text_present(d->key_id) || d->key_id_bytes == 0u || d->key_id_bytes > 255u ||
         strlen(d->key_id) != d->key_id_bytes)) { return UETD_INVALID_ARG; }
    if ((p & UAPP_UPDATE_REQUIRE_ANTI_ROLLBACK) != 0u &&
        (!c->has_monotonic_security_index || b->read_security_index == NULL || b->advance_security_index == NULL)) {
        return UETD_NOT_SUPPORTED;
    }
    if ((p & UAPP_UPDATE_REQUIRE_ENCRYPTION) != 0u && !c->supports_artifact_decryption) { return UETD_NOT_SUPPORTED; }
    uetd_u64_t total = 0u, previous_end = 0u;
    for (uetd_u16_t i = 0; i < d->segment_count; ++i) {
        const uapp_update_segment_t* segment = &d->segments[i];
        uetd_u64_t end = (uetd_u64_t)segment->load_address + segment->byte_length;
        if (segment->role < UAPP_IMAGE_SEGMENT_EXECUTABLE || segment->role > UAPP_IMAGE_SEGMENT_RESOURCE ||
            segment->byte_length == 0u || end > UINT64_C(0x100000000) ||
            (i != 0u && segment->load_address < previous_end)) { return UETD_INVALID_ARG; }
        previous_end = end; total += segment->byte_length;
    }
    if (total != d->total_image_bytes) { return UETD_INVALID_ARG; }
    *policy = p; return UETD_OK;
}
static uetd_status_t authenticate(uapp_update_session_t* s) {
    uetd_status_t status = UETD_OK;
    if ((s->policy & UAPP_UPDATE_REQUIRE_AUTHENTICATION) != 0u) {
        status = s->backend.verify_signature(s->backend.context, s->descriptor);
    }
    if (status == UETD_OK) { status = s->backend.check_compatibility(s->backend.context, s->descriptor); }
    if (status == UETD_OK && (s->policy & UAPP_UPDATE_REQUIRE_ANTI_ROLLBACK) != 0u) {
        uetd_u64_t floor;
        status = s->backend.read_security_index(s->backend.context, &floor);
        if (status == UETD_OK && s->descriptor->security_index < floor) { status = UETD_CONFLICT; }
    }
    return status;
}
static uetd_status_t fail(uapp_update_session_t* s, uetd_status_t error) {
    s->state = UAPP_UPDATE_FAILED; s->last_error = error; s->busy = false;
    /* Preserve the last durable progress: it is the only restart checkpoint. */
    return error;
}
static uetd_status_t save(uapp_update_session_t* s, uapp_update_state_t state) {
    if (s->progress.sequence == UINT32_MAX) { return UETD_OVERFLOW; }
    uapp_update_progress_t p = s->progress;
    ++p.sequence; p.schema_version = 1u; p.record_bytes = UAPP_UPDATE_PROGRESS_BYTES;
    p.state = state; p.policy = s->policy; p.received_bytes = s->received_bytes;
    if (state >= UAPP_UPDATE_HASH_VERIFIED && state <= UAPP_UPDATE_CONFIRMED) {
        p.verified_segment_count = s->descriptor->segment_count;
    }
    p.last_error = UETD_OK; p.integrity_crc32 = uapp_update_progress_crc(&p);
    uetd_status_t status = s->backend.commit_progress_atomic(s->backend.context, &p, s->progress.sequence);
    if (status == UETD_OK) { s->progress = p; s->state = state; }
    return status;
}
static uetd_status_t enter(uapp_update_session_t* s) {
    if (s == NULL) { return UETD_INVALID_ARG; }
    if (!s->initialized || s->state == UAPP_UPDATE_FAILED) { return UETD_NOT_READY; }
    if (s->busy) { return UETD_BUSY; }
    s->busy = true; return UETD_OK;
}
static uetd_status_t check_time(uapp_update_session_t* s) {
    uetd_u64_t now;
    uetd_status_t status = s->backend.now_us(s->backend.context, &now);
    if (status != UETD_OK) { return status; }
    if (now < s->last_progress_us) { return UETD_CONFLICT; }
    if (now - s->started_us >= (uetd_u64_t)s->backend.capabilities.maximum_session_time_ms * 1000u) { return UETD_TIMEOUT; }
    s->last_progress_us = now;
    return UETD_OK;
}
static bool valid_progress(const uapp_update_progress_t* p) {
    return p->schema_version == 1u && p->record_bytes == UAPP_UPDATE_PROGRESS_BYTES && p->sequence != 0u &&
        p->state >= UAPP_UPDATE_RECEIVING && p->state <= UAPP_UPDATE_FAILED && (p->policy & ~POLICY_MASK) == 0u &&
        memchr(p->staging_slot_id, 0, sizeof(p->staging_slot_id)) != NULL &&
        memchr(p->confirmed_slot_id, 0, sizeof(p->confirmed_slot_id)) != NULL &&
        p->integrity_crc32 == uapp_update_progress_crc(p);
}
static uetd_status_t checkpoint_current(uapp_update_session_t* s) {
    uapp_update_progress_t current;
    uetd_status_t status = s->backend.read_progress(s->backend.context, &current);
    if (status != UETD_OK) { return status; }
    if (!valid_progress(&current)) { return UETD_CRC_ERROR; }
    if (current.sequence != s->progress.sequence || current.integrity_crc32 != s->progress.integrity_crc32) {
        return UETD_CONFLICT;
    }
    return UETD_OK;
}
static uetd_status_t prepare(uapp_update_session_t* s, const uapp_update_backend_t* b,
                            const uapp_update_descriptor_t* d, uapp_update_policy_t requested) {
    if (s == NULL) { return UETD_INVALID_ARG; }
    if (s->busy || s->initialized) { return UETD_BUSY; }
    uapp_update_policy_t policy;
    uetd_status_t status = validate(b, d, requested, &policy);
    if (status != UETD_OK) { return status; }
    s->backend = *b; s->descriptor = d; s->policy = policy; s->busy = true; s->initialized = true;
    s->state = UAPP_UPDATE_IDLE; s->received_bytes = s->next_expected_offset = 0u; s->last_error = UETD_OK;
    memset(&s->progress, 0, sizeof(s->progress));
    status = b->now_us(b->context, &s->started_us);
    if (status != UETD_OK) { return fail(s, status); }
    s->last_progress_us = s->started_us;
    status = authenticate(s);
    if (status == UETD_OK) { status = b->hash_manifest(b->context, d, s->progress.manifest_sha256); }
    if (status != UETD_OK) { return fail(s, status); }
    memcpy(s->progress.artifact_sha256, d->artifact_sha256, 32);
    return UETD_OK;
}
uetd_status_t uapp_update_begin(uapp_update_session_t* s, const uapp_update_backend_t* b,
                               const uapp_update_descriptor_t* d, uapp_update_policy_t requested) {
    uetd_status_t status = prepare(s, b, d, requested);
    if (status != UETD_OK) { return status; }
    uapp_update_progress_t previous;
    status = b->read_progress(b->context, &previous);
    if (status == UETD_OK) {
        if (!valid_progress(&previous)) { return fail(s, UETD_CRC_ERROR); }
        if (previous.state != UAPP_UPDATE_CONFIRMED && previous.state != UAPP_UPDATE_ROLLBACK) { return fail(s, UETD_BUSY); }
        s->progress.sequence = previous.sequence;
    } else if (status != UETD_NOT_READY) { return fail(s, status); }
    if (s->progress.sequence == UINT32_MAX) { return fail(s, UETD_OVERFLOW); }
    status = check_time(s);
    if (status == UETD_OK) { status = b->begin_staging(b->context, d); }
    if (status == UETD_OK) { status = save(s, UAPP_UPDATE_RECEIVING); }
    if (status != UETD_OK) { return fail(s, status); }
    s->busy = false; return UETD_OK;
}
uetd_status_t uapp_update_resume(uapp_update_session_t* s, const uapp_update_backend_t* b,
                                const uapp_update_descriptor_t* d, uapp_update_policy_t requested) {
    uetd_status_t status = prepare(s, b, d, requested);
    if (status != UETD_OK) { return status; }
    uapp_update_progress_t p;
    status = b->read_progress(b->context, &p);
    if (status != UETD_OK) { return fail(s, status); }
    if (!valid_progress(&p)) { return fail(s, UETD_CRC_ERROR); }
    if (p.policy != s->policy || memcmp(p.manifest_sha256, s->progress.manifest_sha256, 32) != 0 ||
        memcmp(p.artifact_sha256, d->artifact_sha256, 32) != 0 || p.received_bytes > d->total_image_bytes ||
        p.verified_segment_count > d->segment_count ||
        (p.state != UAPP_UPDATE_RECEIVING && p.state != UAPP_UPDATE_ROLLBACK && p.received_bytes != d->total_image_bytes)) {
        return fail(s, UETD_CONFLICT);
    }
    s->progress = p; s->state = p.state; s->received_bytes = s->next_expected_offset = p.received_bytes;
    if (p.state == UAPP_UPDATE_ROLLBACK) { s->busy = false; return UETD_OK; }
    if (p.state == UAPP_UPDATE_FAILED) { return fail(s, UETD_NOT_READY); }
    status = b->resume_staging(b->context, d, p.received_bytes);
    if (status != UETD_OK) { return fail(s, status); }
    if (p.received_bytes == d->total_image_bytes) {
        status = b->verify_staged_image(b->context, d);
        if (status != UETD_OK) { return fail(s, status); }
    }
    if (p.state >= UAPP_UPDATE_PENDING_TRIAL) {
        uapp_update_boot_state_t boot = UAPP_BOOT_STAGED; bool healthy = false;
        status = b->boot_status(b->context, d->artifact_sha256, &boot, &healthy);
        if (status != UETD_OK) { return fail(s, status); }
        if (boot == UAPP_BOOT_ROLLED_BACK) { status = save(s, UAPP_UPDATE_ROLLBACK); }
        else if (boot == UAPP_BOOT_TRIAL || boot == UAPP_BOOT_CONFIRMED) {
            if (p.state == UAPP_UPDATE_CONFIRMED && boot != UAPP_BOOT_CONFIRMED) { return fail(s, UETD_CONFLICT); }
            s->state = p.state == UAPP_UPDATE_CONFIRMED ? UAPP_UPDATE_CONFIRMED : UAPP_UPDATE_TRIAL_BOOT;
        } else if (boot != UAPP_BOOT_PENDING || p.state == UAPP_UPDATE_CONFIRMED) { return fail(s, UETD_CONFLICT); }
        if (status != UETD_OK) { return fail(s, status); }
    }
    s->busy = false; return UETD_OK;
}
uetd_status_t uapp_update_write(uapp_update_session_t* s, uetd_u32_t offset, const uetd_u8_t* bytes, uetd_u32_t length) {
    uetd_status_t status = enter(s);
    if (status != UETD_OK) { return status; }
    if (s->state != UAPP_UPDATE_RECEIVING) { s->busy = false; return UETD_NOT_READY; }
    if (bytes == NULL || length == 0u || length > s->backend.capabilities.maximum_transfer_chunk_bytes ||
        offset != s->next_expected_offset || length > s->descriptor->total_image_bytes - s->received_bytes) {
        s->busy = false; return UETD_INVALID_ARG;
    }
    if (s->progress.sequence == UINT32_MAX) { return fail(s, UETD_OVERFLOW); }
    status = checkpoint_current(s);
    if (status == UETD_OK) { status = check_time(s); }
    if (status == UETD_OK) { status = s->backend.write_staging(s->backend.context, offset, bytes, length); }
    if (status != UETD_OK) { return fail(s, status); }
    s->received_bytes += length; s->next_expected_offset += length;
    status = save(s, UAPP_UPDATE_RECEIVING);
    if (status != UETD_OK) { return fail(s, status); }
    s->busy = false; return UETD_OK;
}
uetd_status_t uapp_update_finalize(uapp_update_session_t* s) {
    uetd_status_t status = enter(s);
    if (status != UETD_OK) { return status; }
    if (s->state == UAPP_UPDATE_PENDING_TRIAL) { s->busy = false; return UETD_OK; }
    if (s->state < UAPP_UPDATE_RECEIVING || s->state > UAPP_UPDATE_COMPATIBLE ||
        s->received_bytes != s->descriptor->total_image_bytes) { s->busy = false; return UETD_NOT_READY; }
    if (s->progress.sequence > UINT32_MAX - 5u) { return fail(s, UETD_OVERFLOW); }
    status = checkpoint_current(s);
    if (status == UETD_OK) { status = check_time(s); }
    if (status == UETD_OK) { status = save(s, UAPP_UPDATE_STAGED); }
    if (status == UETD_OK) { status = s->backend.verify_staged_image(s->backend.context, s->descriptor); }
    if (status == UETD_OK) { status = save(s, UAPP_UPDATE_HASH_VERIFIED); }
    if (status == UETD_OK) { status = authenticate(s); }
    if (status == UETD_OK && (s->policy & UAPP_UPDATE_REQUIRE_AUTHENTICATION) != 0u) {
        status = save(s, UAPP_UPDATE_SIGNATURE_VERIFIED);
    }
    if (status == UETD_OK) { status = save(s, UAPP_UPDATE_COMPATIBLE); }
    if (status == UETD_OK) { status = check_time(s); }
    if (status == UETD_OK) { status = s->backend.select_trial_image(s->backend.context); }
    if (status == UETD_OK) { status = save(s, UAPP_UPDATE_PENDING_TRIAL); }
    if (status != UETD_OK) { return fail(s, status); }
    s->busy = false; return UETD_OK;
}
uetd_status_t uapp_update_confirm_trial(uapp_update_session_t* s) {
    uetd_status_t status = enter(s);
    if (status != UETD_OK) { return status; }
    if (s->state == UAPP_UPDATE_CONFIRMED) { s->busy = false; return UETD_OK; }
    if (s->state != UAPP_UPDATE_PENDING_TRIAL && s->state != UAPP_UPDATE_TRIAL_BOOT) { s->busy = false; return UETD_NOT_READY; }
    if (s->progress.sequence > UINT32_MAX - 2u) { return fail(s, UETD_OVERFLOW); }
    uapp_update_boot_state_t boot = UAPP_BOOT_STAGED; bool healthy = false;
    status = checkpoint_current(s);
    if (status == UETD_OK) { status = s->backend.boot_status(s->backend.context, s->descriptor->artifact_sha256, &boot, &healthy); }
    if (status != UETD_OK) { return fail(s, status); }
    if ((boot != UAPP_BOOT_TRIAL && boot != UAPP_BOOT_CONFIRMED) || !healthy) { s->busy = false; return UETD_NOT_READY; }
    status = authenticate(s);
    if (status == UETD_OK) { status = s->backend.verify_staged_image(s->backend.context, s->descriptor); }
    if (status == UETD_OK) { status = save(s, UAPP_UPDATE_TRIAL_BOOT); }
    if (status == UETD_OK) { status = s->backend.confirm_trial_image(s->backend.context); }
    if (status == UETD_OK && (s->policy & UAPP_UPDATE_REQUIRE_ANTI_ROLLBACK) != 0u) {
        status = s->backend.advance_security_index(s->backend.context, s->descriptor->security_index);
    }
    if (status == UETD_OK) { status = save(s, UAPP_UPDATE_CONFIRMED); }
    if (status != UETD_OK) { return fail(s, status); }
    s->busy = false; return UETD_OK;
}
uetd_status_t uapp_update_rollback(uapp_update_session_t* s) {
    if (s == NULL) { return UETD_INVALID_ARG; }
    if (!s->initialized) { return UETD_NOT_READY; }
    if (s->busy) { return UETD_BUSY; }
    if (s->progress.sequence == 0u || s->progress.schema_version != 1u) { return UETD_NOT_READY; }
    if (s->state == UAPP_UPDATE_CONFIRMED) { return UETD_CONFLICT; }
    if (s->state == UAPP_UPDATE_ROLLBACK) { return UETD_OK; }
    s->busy = true;
    uapp_update_boot_state_t boot = UAPP_BOOT_STAGED; bool healthy = false;
    uetd_status_t status = checkpoint_current(s);
    if (status == UETD_OK) { status = s->backend.boot_status(s->backend.context, s->descriptor->artifact_sha256, &boot, &healthy); }
    if (status != UETD_OK) { return fail(s, status); }
    if (boot == UAPP_BOOT_CONFIRMED) { s->busy = false; return UETD_CONFLICT; }
    if (s->progress.sequence == UINT32_MAX) { return fail(s, UETD_OVERFLOW); }
    status = s->backend.rollback_trial_image(s->backend.context);
    if (status == UETD_OK) { status = save(s, UAPP_UPDATE_ROLLBACK); }
    if (status != UETD_OK) { return fail(s, status); }
    s->last_error = UETD_OK; s->busy = false; return UETD_OK;
}
