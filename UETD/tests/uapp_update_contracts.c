#include "uos_uapp_test.h"
#include "uapp/secure_update.h"

typedef struct {
    uapp_update_progress_t progress;
    bool have_progress, healthy;
    uapp_update_boot_state_t boot;
    uetd_u64_t now, index;
    unsigned begins, writes, verifies, selects, confirms, advances, rollbacks;
    unsigned fail_operation;
    bool fail_next_commit;
    unsigned char image[8];
} backend_t;
enum { FAIL_SIGNATURE = 1, FAIL_COMPATIBILITY, FAIL_IMAGE, FAIL_WRITE, FAIL_CONFIRM, FAIL_ADVANCE };
static uetd_status_t signature(void* ctx, const uapp_update_descriptor_t* d) {
    CHECK(d->signature_bytes == 1); return ((backend_t*)ctx)->fail_operation == FAIL_SIGNATURE ? UETD_ERROR : UETD_OK;
}
static uetd_status_t compatible(void* ctx, const uapp_update_descriptor_t* d) {
    CHECK(d->segment_count == 1); return ((backend_t*)ctx)->fail_operation == FAIL_COMPATIBILITY ? UETD_CONFLICT : UETD_OK;
}
static uetd_status_t begin(void* ctx, const uapp_update_descriptor_t* d) {
    backend_t* b = ctx; ++b->begins; CHECK(d->total_image_bytes == 8); b->boot = UAPP_BOOT_STAGED; memset(b->image, 0, 8); return UETD_OK;
}
static uetd_status_t write_image(void* ctx, uetd_u32_t offset, const uetd_u8_t* bytes, uetd_u32_t length) {
    backend_t* b = ctx; ++b->writes;
    if (b->fail_operation == FAIL_WRITE) { return UETD_ERROR; }
    CHECK(offset <= 8 && length <= 8 - offset); memcpy(b->image + offset, bytes, length); return UETD_OK;
}
static uetd_status_t verify(void* ctx, const uapp_update_descriptor_t* d) {
    backend_t* b = ctx; ++b->verifies; CHECK(d->total_image_bytes == 8);
    return b->fail_operation == FAIL_IMAGE ? UETD_CRC_ERROR : UETD_OK;
}
static uetd_status_t select_image(void* ctx) { backend_t* b = ctx; ++b->selects; b->boot = UAPP_BOOT_PENDING; return UETD_OK; }
static uetd_status_t confirm(void* ctx) {
    backend_t* b = ctx; ++b->confirms;
    if (b->fail_operation == FAIL_CONFIRM) { return UETD_ERROR; }
    CHECK(b->boot == UAPP_BOOT_TRIAL || b->boot == UAPP_BOOT_CONFIRMED); b->boot = UAPP_BOOT_CONFIRMED; return UETD_OK;
}
static uetd_status_t rollback(void* ctx) { backend_t* b = ctx; ++b->rollbacks; b->boot = UAPP_BOOT_ROLLED_BACK; return UETD_OK; }
static uetd_status_t read_index(void* ctx, uetd_u64_t* value) { *value = ((backend_t*)ctx)->index; return UETD_OK; }
static uetd_status_t advance(void* ctx, uetd_u64_t value) {
    backend_t* b = ctx; ++b->advances; CHECK(b->boot == UAPP_BOOT_CONFIRMED);
    if (b->fail_operation == FAIL_ADVANCE) { return UETD_ERROR; }
    CHECK(value >= b->index); b->index = value; return UETD_OK;
}
static uetd_status_t read_progress(void* ctx, uapp_update_progress_t* p) {
    backend_t* b = ctx; if (!b->have_progress) { return UETD_NOT_READY; } *p = b->progress; return UETD_OK;
}
static uetd_status_t commit(void* ctx, const uapp_update_progress_t* p, uetd_u32_t expected) {
    backend_t* b = ctx;
    if ((b->have_progress ? b->progress.sequence : 0u) != expected) { return UETD_CONFLICT; }
    if (b->fail_next_commit) { b->fail_next_commit = false; return UETD_ERROR; }
    CHECK(p->integrity_crc32 == uapp_update_progress_crc(p)); b->progress = *p; b->have_progress = true; return UETD_OK;
}
static uetd_status_t time_now(void* ctx, uetd_u64_t* value) { *value = ((backend_t*)ctx)->now; return UETD_OK; }
static uetd_status_t hash(void* ctx, const uapp_update_descriptor_t* d, uetd_u8_t result[32]) {
    (void)ctx; memset(result, d->canonical_manifest[0], 32); return UETD_OK;
}
static uetd_status_t resume(void* ctx, const uapp_update_descriptor_t* d, uetd_u32_t bytes) {
    (void)ctx; CHECK(bytes <= d->total_image_bytes); return UETD_OK;
}
static uetd_status_t boot_status(void* ctx, const uetd_u8_t digest[32], uapp_update_boot_state_t* state, bool* healthy) {
    backend_t* b = ctx; CHECK(digest[0] == 1); *state = b->boot; *healthy = b->healthy; return UETD_OK;
}
static uapp_update_backend_t make_backend(backend_t* b) {
    uapp_update_backend_t api = {0};
    api.verify_signature = signature; api.check_compatibility = compatible; api.begin_staging = begin;
    api.write_staging = write_image; api.verify_staged_image = verify; api.select_trial_image = select_image;
    api.confirm_trial_image = confirm; api.rollback_trial_image = rollback; api.read_security_index = read_index;
    api.advance_security_index = advance; api.read_progress = read_progress; api.commit_progress_atomic = commit;
    api.context = b; api.now_us = time_now; api.hash_manifest = hash; api.resume_staging = resume; api.boot_status = boot_status;
    api.capabilities.verifies_signatures = true; api.capabilities.protected_trust_anchor = true;
    api.capabilities.has_staging_region = true; api.capabilities.has_trial_boot = true; api.capabilities.has_rollback = true;
    api.capabilities.has_monotonic_security_index = true; api.capabilities.maximum_image_bytes = 1024;
    api.capabilities.maximum_transfer_chunk_bytes = 4; api.capabilities.maximum_session_time_ms = 100;
    return api;
}
static const uapp_update_segment_t segment = {UAPP_IMAGE_SEGMENT_EXECUTABLE, 0x1000, 8, {1}};
static const unsigned char manifest[] = {7}, signed_bytes[] = {9}, image[] = {1,2,3,4,5,6,7,8};
static uapp_update_descriptor_t descriptor(void) {
    uapp_update_descriptor_t d = {0};
    d.project_id = "test"; d.chip_id = "host"; d.board_id = "test";
    d.minimum_hardware_revision = "1"; d.maximum_hardware_revision = "1"; d.firmware_version = "1";
    d.manifest_schema_version = 1; d.trial_health_window_ms = 10; d.security_index = 2;
    d.total_image_bytes = 8; d.segment_count = 1; d.segments = &segment; d.artifact_sha256[0] = 1;
    d.canonicalization_id = "test"; d.canonical_manifest = manifest; d.manifest_bytes = 1;
    d.declared_policy = UAPP_UPDATE_REQUIRE_ANTI_ROLLBACK | UAPP_UPDATE_REQUIRE_RECOVERY;
    d.signature = signed_bytes; d.signature_bytes = 1; d.signature_algorithm = "test"; d.key_id = "key"; d.key_id_bytes = 3;
    return d;
}
static void receive_image(uapp_update_session_t* s) {
    CHECK(uapp_update_write(s, 0, image, 4) == UETD_OK);
    CHECK(uapp_update_write(s, 4, image + 4, 4) == UETD_OK);
}
int main(void) {
    backend_t b = {0}; uapp_update_backend_t api = make_backend(&b); uapp_update_descriptor_t d = descriptor();
    uapp_update_session_t s = {0};
    CHECK(uapp_update_begin(&s, &api, &d, UAPP_UPDATE_ALLOW_UNSIGNED_DEVELOPMENT) == UETD_INVALID_ARG && b.begins == 0);
    CHECK(uapp_update_begin(&s, &api, &d, 0) == UETD_OK && b.begins == 1);
    CHECK(uapp_update_begin(&s, &api, &d, 0) == UETD_BUSY);
    CHECK(uapp_update_finalize(&s) == UETD_NOT_READY && b.selects == 0);
    CHECK(uapp_update_write(&s, 1, image, 4) == UETD_INVALID_ARG && b.writes == 0);
    CHECK(uapp_update_write(&s, 0, image, 4) == UETD_OK);
    uapp_update_session_t resumed = {0};
    CHECK(uapp_update_resume(&resumed, &api, &d, 0) == UETD_OK && resumed.received_bytes == 4);
    CHECK(uapp_update_write(&resumed, 4, image + 4, 4) == UETD_OK);
    const unsigned writes_before_stale = b.writes;
    CHECK(uapp_update_write(&s, 4, image + 4, 4) == UETD_CONFLICT && b.writes == writes_before_stale);
    CHECK(uapp_update_finalize(&resumed) == UETD_OK && b.selects == 1 && b.index == 0);
    CHECK(uapp_update_confirm_trial(&resumed) == UETD_NOT_READY && b.confirms == 0);
    b.boot = UAPP_BOOT_TRIAL;
    CHECK(uapp_update_confirm_trial(&resumed) == UETD_NOT_READY && b.confirms == 0);
    b.healthy = true; b.fail_operation = FAIL_ADVANCE;
    CHECK(uapp_update_confirm_trial(&resumed) == UETD_ERROR && b.boot == UAPP_BOOT_CONFIRMED && b.index == 0);
    CHECK(uapp_update_rollback(&resumed) == UETD_CONFLICT && b.rollbacks == 0);
    uapp_update_session_t after_reset = {0}; b.fail_operation = 0;
    CHECK(uapp_update_resume(&after_reset, &api, &d, 0) == UETD_OK);
    CHECK(uapp_update_confirm_trial(&after_reset) == UETD_OK && b.index == 2 && after_reset.state == UAPP_UPDATE_CONFIRMED);
    CHECK(uapp_update_confirm_trial(&after_reset) == UETD_OK && b.confirms == 2);
    for (unsigned failure = FAIL_SIGNATURE; failure <= FAIL_WRITE; ++failure) {
        backend_t f = {0}; uapp_update_backend_t provider = make_backend(&f); uapp_update_session_t test = {0};
        f.fail_operation = failure;
        uetd_status_t status = uapp_update_begin(&test, &provider, &d, 0);
        if (failure <= FAIL_COMPATIBILITY) { CHECK(status != UETD_OK && f.begins == 0); }
        else {
            CHECK(status == UETD_OK);
            if (failure == FAIL_WRITE) { CHECK(uapp_update_write(&test, 0, image, 4) == UETD_ERROR && test.state == UAPP_UPDATE_FAILED); }
            else { receive_image(&test); CHECK(uapp_update_finalize(&test) == UETD_CRC_ERROR && f.selects == 0); }
        }
    }
    backend_t f = {0}; api = make_backend(&f); uapp_update_session_t first = {0};
    CHECK(uapp_update_begin(&first, &api, &d, 0) == UETD_OK);
    f.fail_next_commit = true;
    CHECK(uapp_update_write(&first, 0, image, 4) == UETD_ERROR && f.progress.received_bytes == 0);
    uapp_update_session_t retry = {0};
    CHECK(uapp_update_resume(&retry, &api, &d, 0) == UETD_OK && retry.received_bytes == 0);
    receive_image(&retry); CHECK(uapp_update_finalize(&retry) == UETD_OK);
    CHECK(uapp_update_rollback(&retry) == UETD_OK && f.rollbacks == 1);
    CHECK(uapp_update_rollback(&retry) == UETD_OK && f.rollbacks == 1);
    f.progress.integrity_crc32 ^= 1; uapp_update_session_t corrupt = {0};
    CHECK(uapp_update_resume(&corrupt, &api, &d, 0) == UETD_CRC_ERROR);
    backend_t t = {0}; api = make_backend(&t); uapp_update_session_t timed = {0};
    CHECK(uapp_update_begin(&timed, &api, &d, 0) == UETD_OK); t.now = 100000;
    CHECK(uapp_update_write(&timed, 0, image, 4) == UETD_TIMEOUT && t.writes == 0);
    puts("Update admission, recovery, integrity, boot evidence and anti-rollback contracts passed"); return 0;
}
