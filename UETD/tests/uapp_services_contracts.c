#include "uos_uapp_test.h"
#include "uapp/product_session.h"
#include "uapp/diagnostics/diagnostics.h"
#include "uapp/diagnostics/fault_history.h"
#include "uapp/diagnostics/fault.h"

typedef struct {
    ucore_clock_stamp_t clock;
    bool authorized, bad_evidence;
    unsigned sends, records, commands, results;
    uetd_status_t result;
    bool expire_in_authorize;
} provider_t;
static uetd_status_t now(void* ctx, ucore_clock_stamp_t* stamp) { *stamp = ((provider_t*)ctx)->clock; return UETD_OK; }
static uetd_status_t authorize_command(void* ctx, const uapp_product_identity_t* peer, const uapp_product_command_t* cmd) {
    CHECK(peer->identity_schema == 1 && cmd->schema_id == 1);
    if (((provider_t*)ctx)->expire_in_authorize) { ((provider_t*)ctx)->clock = cmd->expires.expires; }
    return ((provider_t*)ctx)->authorized ? UETD_OK : UETD_CONFLICT;
}
static uetd_status_t send_command(void* ctx, const uapp_product_identity_t* peer, const uapp_product_command_t* cmd) {
    ++((provider_t*)ctx)->sends; CHECK(peer->identity_schema == 1 && cmd->payload.data[0] == 7); return UETD_OK;
}
static uetd_status_t observe(void* ctx, const uapp_product_identity_t* peer, uapp_product_outcome_t* out) {
    (void)peer; if (((provider_t*)ctx)->bad_evidence) { ++out->session_generation; }
    out->state = UAPP_COMMAND_COMPLETED; out->result = UETD_OK; return UETD_OK;
}
static void product_tests(void) {
    provider_t p = {{10, 1, 1}, true, false, 0, 0, 0, 0, UETD_OK};
    uapp_product_session_t s = {0}; uapp_product_slot_t slots[2];
    uapp_product_session_config_t cfg = {{{0}, 1}, 2, &p, slots, {now, authorize_command, send_command, observe}};
    CHECK(uapp_product_session_init(&s, &cfg) == UETD_OK);
    unsigned char bytes[] = {7};
    uapp_product_command_t command = {1, 1, 1, {{20, 1, 1}, false}, 1, {bytes, 1}};
    CHECK(uapp_product_session_submit(&s, &command) == UETD_OK); bytes[0] = 9;
    CHECK(uapp_product_session_submit(&s, &command) == UETD_CONFLICT);
    CHECK(uapp_product_session_poll(&s, 0) == UETD_OK && p.sends == 0);
    CHECK(uapp_product_session_poll(&s, 1) == UETD_OK && p.sends == 1);
    uapp_product_outcome_t out = {99, 99, UAPP_COMMAND_UNKNOWN, UETD_ERROR};
    CHECK(uapp_product_session_next_outcome(&s, &out) == UETD_NOT_READY && out.command_id == 99);
    CHECK(uapp_product_session_poll(&s, 1) == UETD_OK && p.sends == 1);
    CHECK(uapp_product_session_next_outcome(&s, &out) == UETD_OK && out.state == UAPP_COMMAND_COMPLETED);
    bytes[0] = 7; command.id = 2;
    CHECK(uapp_product_session_submit(&s, &command) == UETD_OK);
    p.authorized = false;
    CHECK(uapp_product_session_poll(&s, 1) == UETD_OK && p.sends == 1);
    CHECK(uapp_product_session_next_outcome(&s, &out) == UETD_OK && out.state == UAPP_COMMAND_REJECTED);
    p.authorized = true; command.id = 3;
    CHECK(uapp_product_session_submit(&s, &command) == UETD_OK);
    CHECK(uapp_product_session_poll(&s, 1) == UETD_OK && p.sends == 2);
    p.clock.ticks_us = 20;
    CHECK(uapp_product_session_poll(&s, 1) == UETD_OK);
    CHECK(uapp_product_session_next_outcome(&s, &out) == UETD_OK && out.state == UAPP_COMMAND_UNKNOWN);
    command.id = 4;
    CHECK(uapp_product_session_submit(&s, &command) == UETD_TIMEOUT);
    command.expires.expires.ticks_us = 30;
    CHECK(uapp_product_session_submit(&s, &command) == UETD_OK);
    CHECK(uapp_product_session_invalidate(&s) == UETD_OK && s.generation == 2);
    CHECK(uapp_product_session_next_outcome(&s, &out) == UETD_OK && out.session_generation == 1 && out.state == UAPP_COMMAND_UNKNOWN);
    command.id = 5; command.session_generation = 2; p.bad_evidence = true;
    CHECK(uapp_product_session_submit(&s, &command) == UETD_OK);
    CHECK(uapp_product_session_poll(&s, 1) == UETD_OK);
    CHECK(uapp_product_session_poll(&s, 1) == UETD_OK);
    CHECK(uapp_product_session_next_outcome(&s, &out) == UETD_OK && out.state == UAPP_COMMAND_UNKNOWN && out.result == UETD_CONFLICT);
    command.id = 6; p.bad_evidence = false;
    CHECK(uapp_product_session_submit(&s, &command) == UETD_OK);
    const unsigned sent = p.sends; p.expire_in_authorize = true;
    CHECK(uapp_product_session_poll(&s, 1) == UETD_OK && p.sends == sent);
    CHECK(uapp_product_session_next_outcome(&s, &out) == UETD_OK && out.state == UAPP_COMMAND_EXPIRED);
}
static uetd_status_t validate_record(void* ctx, const uapp_diagnostic_record_t* r) { (void)ctx; return r->schema_id == 1 ? UETD_OK : UETD_NOT_SUPPORTED; }
static uetd_status_t authorize_request(void* ctx, const uapp_maintenance_request_t* r) {
    if (((provider_t*)ctx)->expire_in_authorize) { ((provider_t*)ctx)->clock = r->deadline.expires; }
    CHECK(r->principal == 1); return ((provider_t*)ctx)->authorized ? UETD_OK : UETD_CONFLICT;
}
static uetd_status_t write_record(void* ctx, const uapp_diagnostic_record_t* r) {
    CHECK(r->fields.data[0] == 7); ++((provider_t*)ctx)->records; return UETD_OK;
}
static uetd_status_t execute(void* ctx, const uapp_maintenance_request_t* r) { (void)r; ++((provider_t*)ctx)->commands; return UETD_OK; }
static void result(void* ctx, uetd_u64_t id, uetd_status_t status) {
    provider_t* p = ctx; CHECK(id > 0); ++p->results; p->result = status;
}
static void diagnostics_tests(void) {
    provider_t p = {{10, 1, 1}, true, false, 0, 0, 0, 0, UETD_OK};
    uapp_diagnostics_t d = {0}; uapp_diagnostics_slot_t slots[2];
    const uapp_diagnostics_config_t cfg = {&p, 16, 16, 2, slots, {now, validate_record, authorize_request, write_record, execute, result}};
    CHECK(uapp_diagnostics_init(&d, &cfg) == UETD_OK);
    unsigned char bytes[] = {7};
    const uapp_diagnostic_record_t record = {1, 1, 0, {10, 1, 1}, {bytes, 1}};
    uapp_maintenance_request_t request = {1, 1, 1, 0, {{20, 1, 1}, false}, {bytes, 1}};
    CHECK(uapp_diagnostics_emit(&d, &record) == UETD_OK);
    CHECK(uapp_diagnostics_request(&d, &request) == UETD_OK);
    CHECK(uapp_diagnostics_emit(&d, &record) == UETD_OVERFLOW && d.dropped_records == 1);
    bytes[0] = 9; p.authorized = false;
    CHECK(uapp_diagnostics_poll(&d, 1) == UETD_OK && p.records == 1 && p.results == 0);
    CHECK(uapp_diagnostics_poll(&d, 1) == UETD_CONFLICT && p.results == 1 && p.commands == 0);
    CHECK(uapp_diagnostics_request(&d, &request) == UETD_CONFLICT);
    p.authorized = true; request.request_id = 2;
    CHECK(uapp_diagnostics_request(&d, &request) == UETD_OK);
    p.clock.ticks_us = 20;
    CHECK(uapp_diagnostics_poll(&d, 1) == UETD_TIMEOUT && p.commands == 0 && p.result == UETD_TIMEOUT);
    request.request_id = 3; request.deadline.expires.ticks_us = 30;
    CHECK(uapp_diagnostics_request(&d, &request) == UETD_OK);
    CHECK(uapp_diagnostics_poll(&d, 2) == UETD_OK && p.commands == 1 && p.results == 3);
    CHECK(uapp_diagnostics_poll(&d, 2) == UETD_OK && p.commands == 1);
    request.request_id = 4;
    CHECK(uapp_diagnostics_request(&d, &request) == UETD_OK);
    p.expire_in_authorize = true;
    CHECK(uapp_diagnostics_poll(&d, 1) == UETD_TIMEOUT && p.commands == 1 && p.results == 4);
}
static unsigned char retained[1024]; static size_t retained_size; static uetd_u32_t retained_sequence; static bool fail_commit;
static uetd_u32_t retained_high_water;
static uetd_status_t read_crash(void* ctx, uetd_u8_t* out, size_t capacity, size_t* length) {
    (void)ctx; if (!retained_size) { return UETD_NOT_READY; }
    CHECK(capacity >= retained_size); memcpy(out, retained, retained_size); *length = retained_size; return UETD_OK;
}
static uetd_status_t commit_crash(void* ctx, const uetd_u8_t* data, size_t length, uetd_u32_t expected) {
    (void)ctx; if (expected != retained_sequence) { return UETD_CONFLICT; }
    if (fail_commit) { return UETD_ERROR; }
    const uetd_u32_t sequence = (uetd_u32_t)data[8] | ((uetd_u32_t)data[9] << 8) | ((uetd_u32_t)data[10] << 16) | ((uetd_u32_t)data[11] << 24);
    if (sequence != retained_sequence && sequence <= retained_high_water) { return UETD_CONFLICT; }
    CHECK(length <= sizeof(retained)); memcpy(retained, data, length); retained_size = length;
    retained_sequence = (uetd_u32_t)data[8] | ((uetd_u32_t)data[9] << 8) | ((uetd_u32_t)data[10] << 16) | ((uetd_u32_t)data[11] << 24);
    retained_high_water = retained_sequence;
    return UETD_OK;
}
static uetd_status_t clear_crash(void* ctx, uetd_u32_t expected) {
    (void)ctx; if (expected != retained_sequence) { return UETD_CONFLICT; }
    retained_size = 0; retained_sequence = 0; return UETD_OK;
}
static void crash_tests(void) {
    uapp_crash_record_t record = {0}, out = {0}; uetd_u16_t length = 99;
    const uapp_crash_backend_t backend = {read_crash, commit_crash, clear_crash, NULL};
    CHECK(uapp_crash_record_read(&out, NULL, 0, &length) == UETD_NOT_READY && length == 99);
    CHECK(uapp_crash_history_bind(&backend) == UETD_OK);
    record.sequence = 1; record.fault_code = 42; record.detail_length = 3; memcpy(record.detail, "bad", 3);
    record.architecture_context_length = 3; record.architecture_context_schema = 1;
    record.capture_valid_mask = UAPP_CRASH_CAPTURE_ARCH_CONTEXT_VALID;
    unsigned char context[] = {1, 2, 3}, copy[3] = {9, 9, 9};
    CHECK(uapp_crash_record_commit(&record, context, 3) == UETD_OK);
    CHECK(uapp_crash_record_read(&out, copy, 2, &length) == UETD_OVERFLOW && length == 99 && copy[0] == 9);
    CHECK(uapp_crash_record_read(&out, copy, 3, &length) == UETD_OK && length == 3 && out.fault_code == 42);
    CHECK(memcmp(copy, context, 3) == 0 && out.magic == UAPP_CRASH_MAGIC && out.format_version == 1);
    CHECK(uapp_crash_record_clear(1) == UETD_NOT_READY);
    CHECK(uapp_crash_record_acknowledge(1) == UETD_NOT_READY);
    CHECK(uapp_crash_record_mark_exported(2) == UETD_CONFLICT);
    fail_commit = true; record.sequence = 2;
    CHECK(uapp_crash_record_commit(&record, context, 3) == UETD_ERROR);
    CHECK(uapp_crash_record_read(&out, copy, 3, &length) == UETD_OK && out.sequence == 1);
    fail_commit = false; retained[20] ^= 1;
    CHECK(uapp_crash_record_read(&out, copy, 3, &length) == UETD_CRC_ERROR && out.sequence == 1);
    CHECK(uapp_crash_record_commit(&record, context, 3) == UETD_CRC_ERROR);
    retained[20] ^= 1;
    CHECK(uapp_crash_record_mark_exported(1) == UETD_OK);
    CHECK(uapp_crash_record_acknowledge(1) == UETD_OK);
    CHECK(uapp_crash_record_mark_exported(1) == UETD_OK);
    CHECK(uapp_crash_record_clear(1) == UETD_OK);
    CHECK(uapp_crash_record_read(&out, copy, 3, &length) == UETD_NOT_READY);
    record.sequence = 1; CHECK(uapp_crash_record_commit(&record, context, 3) == UETD_CONFLICT);
    record.sequence = 2; CHECK(uapp_crash_record_commit(&record, context, 3) == UETD_OK);
    CHECK(uapp_crash_record_acknowledge(1) == UETD_CONFLICT);
}
static unsigned recovery_count;
static void recover_fault(const uapp_fault_record_t* record, void* context) {
    CHECK(context == &recovery_count && record->fault_code == 42);
    ++recovery_count;
    CHECK(uapp_fault_dispatch(1) == 0);
    uapp_fault_report(42, "next dispatch");
}
static void fault_tests(void) {
    uapp_fault_clear_recoverable();
    uapp_fault_set_recovery_handler(NULL, NULL);
    char detail[] = "copied"; uapp_fault_report(42, detail); detail[0] = 'X';
    CHECK(uapp_fault_dispatch(1) == 0);
    uapp_fault_record_t record;
    CHECK(uapp_fault_try_pop(&record) && strcmp(record.detail, "copied") == 0);
    for (unsigned i = 0; i < UAPP_FAULT_QUEUE_CAPACITY + 1; ++i) { uapp_fault_report(42, "bounded"); }
    CHECK(uapp_fault_dropped_count() == 1);
    uapp_fault_set_recovery_handler(recover_fault, &recovery_count);
    CHECK(uapp_fault_dispatch(UINT32_MAX) == UAPP_FAULT_QUEUE_CAPACITY);
    CHECK(recovery_count == UAPP_FAULT_QUEUE_CAPACITY);
    uapp_fault_set_recovery_handler(NULL, NULL);
    CHECK(uapp_fault_dispatch(1) == 0 && uapp_fault_try_pop(&record));
    uapp_fault_clear_recoverable(); CHECK(!uapp_fault_try_pop(&record));
}
int main(void) { product_tests(); diagnostics_tests(); crash_tests(); fault_tests(); puts("UAPP service contracts passed"); return 0; }
