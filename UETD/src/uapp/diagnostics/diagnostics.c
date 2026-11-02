/// @file src/uapp/diagnostics/diagnostics.c
#include "uapp/diagnostics/diagnostics.h"
#include <string.h>
static uetd_status_t ready(uapp_diagnostics_t* d) {
    return d == NULL ? UETD_INVALID_ARG : !d->initialized ? UETD_NOT_READY : d->busy ? UETD_BUSY : UETD_OK;
}
static void increment(uetd_u32_t* n) { if (*n != UINT32_MAX) { ++*n; } }
static uetd_status_t fresh(uapp_diagnostics_t* d, ucore_deadline_t deadline) {
    ucore_clock_stamp_t now;
    uetd_status_t status = d->config.provider.now(d->config.provider_context, &now);
    if (status != UETD_OK) { return status; }
    bool expired;
    status = ucore_deadline_expired(deadline, now, &expired);
    return status != UETD_OK ? status : expired ? UETD_TIMEOUT : UETD_OK;
}
uetd_status_t uapp_diagnostics_init(uapp_diagnostics_t* d, const uapp_diagnostics_config_t* c) {
    if (d == NULL || c == NULL || c->slots == NULL || c->queue_depth == 0u ||
        c->queue_depth > SIZE_MAX / sizeof(*c->slots) ||
        c->maximum_record_bytes > UAPP_DIAGNOSTIC_PAYLOAD_CAPACITY ||
        c->maximum_command_bytes > UAPP_DIAGNOSTIC_PAYLOAD_CAPACITY ||
        c->provider.now == NULL || c->provider.validate_record == NULL || c->provider.authorize == NULL ||
        c->provider.write_record == NULL || c->provider.execute == NULL || c->provider.on_result == NULL) {
        return UETD_INVALID_ARG;
    }
    if (d->initialized) { return UETD_BUSY; }
    d->config = *c; d->generation = 1u; d->last_request_id = 0u;
    d->head = d->tail = d->count = 0u; d->dropped_records = d->failed_records = 0u;
    d->busy = false; d->initialized = true;
    return UETD_OK;
}
uetd_status_t uapp_diagnostics_emit(uapp_diagnostics_t* d, const uapp_diagnostic_record_t* r) {
    uetd_status_t status = ready(d);
    if (status != UETD_OK) { return status; }
    if (r == NULL || r->schema_id == 0u || r->fields.size > d->config.maximum_record_bytes ||
        (r->fields.size != 0u && r->fields.data == NULL)) { return UETD_INVALID_ARG; }
    if (d->count == d->config.queue_depth) { increment(&d->dropped_records); return UETD_OVERFLOW; }
    d->busy = true;
    status = d->config.provider.validate_record(d->config.provider_context, r);
    d->busy = false;
    if (status != UETD_OK) { return status; }
    uapp_diagnostics_slot_t* slot = &d->config.slots[d->head];
    slot->record = *r; slot->maintenance = false;
    if (r->fields.size != 0u) { memmove(slot->payload, r->fields.data, r->fields.size); }
    slot->record.fields.data = slot->payload;
    d->head = (uetd_u16_t)(((size_t)d->head + 1u) % d->config.queue_depth); ++d->count;
    return UETD_OK;
}
uetd_status_t uapp_diagnostics_request(uapp_diagnostics_t* d, const uapp_maintenance_request_t* r) {
    uetd_status_t status = ready(d);
    if (status != UETD_OK) { return status; }
    if (r == NULL || r->request_id == 0u || r->principal == 0u || r->deadline.infinite ||
        r->arguments.size > d->config.maximum_command_bytes ||
        (r->arguments.size != 0u && r->arguments.data == NULL)) { return UETD_INVALID_ARG; }
    if (r->request_id <= d->last_request_id) { return UETD_CONFLICT; }
    if (d->count == d->config.queue_depth) { return UETD_OVERFLOW; }
    d->busy = true;
    status = fresh(d, r->deadline);
    if (status == UETD_OK) { status = d->config.provider.authorize(d->config.provider_context, r); }
    d->busy = false;
    if (status != UETD_OK) { return status; }
    uapp_diagnostics_slot_t* slot = &d->config.slots[d->head];
    slot->request = *r; slot->maintenance = true;
    if (r->arguments.size != 0u) { memmove(slot->payload, r->arguments.data, r->arguments.size); }
    slot->request.arguments.data = slot->payload;
    d->head = (uetd_u16_t)(((size_t)d->head + 1u) % d->config.queue_depth); ++d->count;
    d->last_request_id = r->request_id;
    return UETD_OK;
}
uetd_status_t uapp_diagnostics_poll(uapp_diagnostics_t* d, uetd_u16_t budget) {
    uetd_status_t status = ready(d), first_error = UETD_OK;
    if (status != UETD_OK) { return status; }
    d->busy = true;
    for (uetd_u32_t i = 0; i < budget && d->count != 0u; ++i) {
        uapp_diagnostics_slot_t* slot = &d->config.slots[d->tail];
        if (slot->maintenance) {
            status = fresh(d, slot->request.deadline);
            if (status == UETD_OK) { status = d->config.provider.authorize(d->config.provider_context, &slot->request); }
            if (status == UETD_OK) { status = fresh(d, slot->request.deadline); }
            if (status == UETD_OK) { status = d->config.provider.execute(d->config.provider_context, &slot->request); }
            d->config.provider.on_result(d->config.provider_context, slot->request.request_id, status);
        } else {
            status = d->config.provider.validate_record(d->config.provider_context, &slot->record);
            if (status == UETD_OK) { status = d->config.provider.write_record(d->config.provider_context, &slot->record); }
            if (status != UETD_OK) { increment(&d->failed_records); }
        }
        if (first_error == UETD_OK && status != UETD_OK) { first_error = status; }
        d->tail = (uetd_u16_t)(((size_t)d->tail + 1u) % d->config.queue_depth); --d->count;
    }
    d->busy = false;
    return first_error;
}
