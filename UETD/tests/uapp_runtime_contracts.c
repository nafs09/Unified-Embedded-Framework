#include "uos_uapp_test.h"
#include "uapp/uapp.h"
#include "uos/deferred.h"
#include "ucore/span.h"

typedef struct {
    uapp_statemachine_t sm;
    unsigned trace[32], count, faults, unhandled, fail_stage;
    bool post_exit, post_entry, guard_pass;
} fixture_t;
static void trace(fixture_t* f, unsigned id) { CHECK(f->count < 32); f->trace[f->count++] = id; }
static uetd_status_t entry(uapp_sm_state_t state, const uapp_sm_event_t* event, void* ctx) {
    fixture_t* f = ctx; trace(f, 10u + state);
    CHECK(!uapp_sm_is_active(&f->sm));
    CHECK(uapp_sm_reset(&f->sm) == UETD_BUSY);
    CHECK(uapp_sm_init(&f->sm, NULL) == UETD_BUSY);
    CHECK(uapp_sm_process_one(&f->sm) == UETD_BUSY);
    if (f->post_entry) { uapp_sm_event_t e = {9, 0, {0}}; CHECK(uapp_sm_post(&f->sm, &e) == UETD_OK); }
    (void)event;
    return f->fail_stage == 3 ? UETD_ERROR : UETD_OK;
}
static uetd_status_t leave(uapp_sm_state_t state, const uapp_sm_event_t* event, void* ctx) {
    fixture_t* f = ctx; trace(f, 20u + state);
    CHECK(!uapp_sm_is_active(&f->sm));
    if (f->post_exit) { uapp_sm_event_t e = {8, 0, {0}}; CHECK(uapp_sm_post(&f->sm, &e) == UETD_OK); }
    (void)event;
    return f->fail_stage == 1 ? UETD_TIMEOUT : UETD_OK;
}
static uetd_status_t action(const uapp_sm_event_t* event, void* ctx) {
    fixture_t* f = ctx; trace(f, 30);
    CHECK(event->id == 1 && !uapp_sm_is_active(&f->sm));
    return f->fail_stage == 2 ? UETD_BUSY : UETD_OK;
}
static bool guard(const uapp_sm_event_t* event, void* ctx) { (void)event; return ((fixture_t*)ctx)->guard_pass; }
static void fault(uapp_sm_state_t state, const uapp_sm_event_t* event, uetd_status_t status, void* ctx) {
    fixture_t* f = ctx; ++f->faults;
    CHECK(uapp_sm_is_faulted(&f->sm)); CHECK(!uapp_sm_is_active(&f->sm));
    CHECK(f->sm.queue_count == 0 && status != UETD_OK);
    uapp_sm_event_t e = {1, 0, {0}};
    CHECK(uapp_sm_post(&f->sm, &e) == UETD_NOT_READY);
    CHECK(uapp_sm_reset(&f->sm) == UETD_BUSY);
    (void)state; (void)event;
}
static void unhandled(uapp_sm_state_t state, const uapp_sm_event_t* event, void* ctx) {
    fixture_t* f = ctx; ++f->unhandled; CHECK(uapp_sm_is_active(&f->sm)); (void)state; (void)event;
}
static const uapp_sm_transition_t transitions[] = {{0, 1, 1, action, guard}, {0, 1, 0, action, NULL}, {1, 1, 0, action, NULL}};
static const uapp_sm_state_hooks_t hooks[] = {{entry, leave}, {entry, leave}};
static uapp_sm_config_t configuration(fixture_t* f) {
    const uapp_sm_config_t config = {transitions, 3, hooks, 2, 0, unhandled, fault, f}; return config;
}
static void state_machine_tests(void) {
    fixture_t f = {0}; uapp_sm_config_t config = configuration(&f);
    uapp_sm_state_t state = 77; uetd_status_t error = UETD_OK;
    CHECK(uapp_sm_get_state(&f.sm, &state) == UETD_NOT_READY && state == 77);
    config.state_count = 0; CHECK(uapp_sm_init(&f.sm, &config) == UETD_INVALID_ARG);
    config = configuration(&f);
    const uapp_sm_transition_t unreachable[] = {{0, 1, 1, NULL, NULL}, {0, 1, 0, NULL, guard}};
    config.transitions = unreachable; config.transition_count = 2;
    CHECK(uapp_sm_init(&f.sm, &config) == UETD_INVALID_ARG && !f.sm.initialized);
    config = configuration(&f); CHECK(uapp_sm_init(&f.sm, &config) == UETD_OK);
    CHECK(f.count == 1 && f.trace[0] == 10 && uapp_sm_is_active(&f.sm));
    uapp_sm_event_t e; unsigned char payload[] = {1, 2, 3};
    CHECK(uapp_sm_make_event(&e, 1, payload, 3) == UETD_OK);
    CHECK(uapp_sm_post(&f.sm, &e) == UETD_OK); memset(&e, 0xA5, sizeof(e));
    CHECK(f.sm.event_queue[0].payload[0] == 1 && f.sm.event_queue[0].payload[3] == 0);
    CHECK(uapp_sm_process_one(&f.sm) == UETD_OK);
    CHECK(f.trace[1] == 20 && f.trace[2] == 30 && f.trace[3] == 10);
    f.count = 0; f.guard_pass = true; CHECK(uapp_sm_make_event(&e, 1, NULL, 0) == UETD_OK);
    CHECK(uapp_sm_post(&f.sm, &e) == UETD_OK); CHECK(uapp_sm_process_one(&f.sm) == UETD_OK);
    CHECK(f.trace[0] == 20 && f.trace[1] == 30 && f.trace[2] == 11);
    CHECK(uapp_sm_get_state(&f.sm, &state) == UETD_OK && state == 1);
    e.id = 99;
    for (unsigned cycle = 0; cycle < 3; ++cycle) {
        for (unsigned i = 0; i < UAPP_SM_EVENT_QUEUE_CAPACITY; ++i) { CHECK(uapp_sm_post(&f.sm, &e) == UETD_OK); }
        CHECK(uapp_sm_post(&f.sm, &e) == UETD_OVERFLOW);
        for (unsigned i = 0; i < UAPP_SM_EVENT_QUEUE_CAPACITY; ++i) { CHECK(uapp_sm_process_one(&f.sm) == UETD_NOT_SUPPORTED); }
        CHECK(uapp_sm_process_one(&f.sm) == UETD_NOT_READY);
    }
    f.post_exit = true; f.post_entry = true;
    CHECK(uapp_sm_reset(&f.sm) == UETD_OK && f.sm.queue_count == 1);
    CHECK(f.sm.event_queue[f.sm.queue_tail].id == 9);
    for (unsigned stage = 1; stage <= 3; ++stage) {
        fixture_t g = {0}; config = configuration(&g); g.guard_pass = true;
        CHECK(uapp_sm_init(&g.sm, &config) == UETD_OK); g.fail_stage = stage; e.id = 1;
        CHECK(uapp_sm_post(&g.sm, &e) == UETD_OK); CHECK(uapp_sm_post(&g.sm, &e) == UETD_OK);
        CHECK(uapp_sm_process_one(&g.sm) != UETD_OK && g.faults == 1);
        CHECK(uapp_sm_get_fault_status(&g.sm, &error) == UETD_OK && error != UETD_OK);
        CHECK(g.sm.current == (stage == 3 ? 1 : 0));
        CHECK(uapp_sm_process_one(&g.sm) == UETD_NOT_READY && g.faults == 1);
        g.fail_stage = 0; g.count = 0;
        CHECK(uapp_sm_reset(&g.sm) == UETD_OK && g.count == 1 && g.trace[0] == 10);
    }
    fixture_t g = {0}; config = configuration(&g); g.fail_stage = 3;
    CHECK(uapp_sm_init(&g.sm, &config) == UETD_ERROR && g.sm.initialized && g.faults == 1);
    CHECK(uapp_sm_get_state(&g.sm, &state) == UETD_OK && state == 0);
}

static void state_machine_capacity_tests(void) {
    uapp_statemachine_t machine = UAPP_SM_INITIALIZER;
    uapp_sm_transition_t rows[UAPP_SM_MAX_TRANSITIONS];
    for (unsigned i = 0; i < UAPP_SM_MAX_TRANSITIONS; ++i) {
        rows[i] = (uapp_sm_transition_t){(uetd_u8_t)(i % UAPP_SM_MAX_STATES), (uetd_u8_t)i,
                                       UAPP_SM_MAX_STATES - 1u, NULL, NULL};
    }
    const unsigned last = UAPP_SM_MAX_TRANSITIONS - 1u;
    const uapp_sm_config_t config = {rows, UAPP_SM_MAX_TRANSITIONS, NULL, UAPP_SM_MAX_STATES,
                                     rows[last].from, NULL, NULL, NULL};
    CHECK(uapp_sm_init(&machine, &config) == UETD_OK);
    unsigned char bytes[UAPP_SM_EVENT_PAYLOAD_CAPACITY]; memset(bytes, 0xA5, sizeof(bytes));
    uapp_sm_event_t event;
    CHECK(uapp_sm_make_event(&event, (uetd_u8_t)last, bytes, sizeof(bytes)) == UETD_OK);
    CHECK(uapp_sm_post(&machine, &event) == UETD_OK);
    CHECK(machine.event_queue[0].size == sizeof(bytes));
    CHECK(memcmp(machine.event_queue[0].payload, bytes, sizeof(bytes)) == 0);
    CHECK(uapp_sm_process_one(&machine) == UETD_OK && machine.current == UAPP_SM_MAX_STATES - 1u);
}
typedef struct { uos_deferred_t queue; uos_deferred_token_t running, spawned; unsigned calls; } deferred_fixture_t;
static void deferred_callback(void* ctx) {
    deferred_fixture_t* f = ctx; ++f->calls;
    CHECK(uos_deferred_cancel(&f->queue, f->running) == UETD_BUSY);
    CHECK(uos_deferred_poll(&f->queue, f->queue.last_poll, 1) == UETD_BUSY);
    CHECK(uos_deferred_reset(&f->queue, f->queue.last_poll) == UETD_BUSY);
    if (f->calls == 1) {
        const uos_deferred_item_t item = {deferred_callback, f, {f->queue.last_poll, false}};
        CHECK(uos_deferred_submit(&f->queue, &item, &f->spawned) == UETD_OK);
    }
}
static void deferred_tests(void) {
    deferred_fixture_t f = {0}; uos_deferred_slot_t slots[2];
    const ucore_clock_stamp_t now = {10, 1, 2};
    const uos_deferred_config_t config = {slots, 2, now};
    CHECK(uos_deferred_init(&f.queue, &config) == UETD_OK);
    CHECK(uos_deferred_init(&f.queue, &config) == UETD_BUSY);
    uos_deferred_item_t item = {deferred_callback, &f, {{20, 1, 2}, false}};
    CHECK(uos_deferred_submit(&f.queue, &item, &f.running) == UETD_OK);
    CHECK(uos_deferred_poll(&f.queue, now, 10) == UETD_OK && f.calls == 0);
    CHECK(uos_deferred_poll(&f.queue, (ucore_clock_stamp_t){20, 1, 2}, 10) == UETD_OK && f.calls == 1);
    f.running = f.spawned;
    CHECK(uos_deferred_poll(&f.queue, (ucore_clock_stamp_t){20, 1, 2}, 1) == UETD_OK && f.calls == 2);
    CHECK(uos_deferred_cancel(&f.queue, f.running) == UETD_NOT_READY);
    CHECK(uos_deferred_poll(&f.queue, (ucore_clock_stamp_t){19, 1, 2}, 1) == UETD_CONFLICT);
    CHECK(uos_deferred_poll(&f.queue, (ucore_clock_stamp_t){20, 1, 3}, 1) == UETD_CONFLICT);
    CHECK(uos_deferred_submit(&f.queue, &item, &f.running) == UETD_OK);
    CHECK(uos_deferred_submit(&f.queue, &item, &f.spawned) == UETD_OK);
    uos_deferred_token_t unchanged = {123, 456};
    CHECK(uos_deferred_submit(&f.queue, &item, &unchanged) == UETD_OVERFLOW && unchanged.id == 123);
    CHECK(uos_deferred_reset(&f.queue, now) == UETD_OK);
    CHECK(uos_deferred_cancel(&f.queue, f.running) == UETD_CONFLICT);
}
typedef struct { uapp_component_t component; unsigned faults; } lifecycle_fixture_t;
static void lifecycle_fault(void* ctx, uetd_u32_t code) {
    lifecycle_fixture_t* f = ctx; ++f->faults; CHECK(code == 42); uapp_component_fault(&f->component, 43);
}
static uetd_status_t lifecycle_init(void* ctx) {
    lifecycle_fixture_t* f = ctx; uapp_component_fault(&f->component, 42); return UETD_OK;
}
int main(void) {
    state_machine_tests(); state_machine_capacity_tests(); deferred_tests();
    lifecycle_fixture_t f = {0}; f.component.init = lifecycle_init; f.component.on_fault = lifecycle_fault; f.component.ctx = &f;
    CHECK(uapp_component_init(&f.component) == UETD_ERROR && f.faults == 1 && f.component.fault_code == 42);
    ucore_deadline_t deadline = {{99, 8, 7}, false}, before = deadline;
    CHECK(ucore_deadline_after((ucore_clock_stamp_t){UINT64_MAX, 1, 1}, 1, &deadline) == UETD_OVERFLOW);
    CHECK(memcmp(&deadline, &before, sizeof(deadline)) == 0);
    CHECK(ucore_deadline_after((ucore_clock_stamp_t){10, 1, 1}, 5, &deadline) == UETD_OK);
    bool expired = false;
    CHECK(ucore_deadline_expired(deadline, (ucore_clock_stamp_t){15, 1, 1}, &expired) == UETD_OK && expired);
    CHECK(ucore_deadline_expired(deadline, (ucore_clock_stamp_t){15, 2, 1}, &expired) == UETD_CONFLICT);
    uetd_u64_t value = 77;
    CHECK(ucore_duration_from_ms(UINT64_MAX, &value) == UETD_OVERFLOW && value == 77);
    unsigned char bytes[] = {1, 2, 3, 4};
    CHECK(ucore_span_copy((ucore_write_span_t){bytes + 1, 3}, (ucore_read_span_t){bytes, 3}) == UETD_OK);
    CHECK(bytes[1] == 1 && bytes[2] == 2 && bytes[3] == 3);
    puts("UAPP state machine, lifecycle, deferred work and checked values passed"); return 0;
}
