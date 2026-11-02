/// @file src/uapp/product_session.c
#include "uapp/product_session.h"
#include <string.h>

static bool terminal(uapp_command_state_t state) { return state >= UAPP_COMMAND_COMPLETED && state <= UAPP_COMMAND_UNKNOWN; }
static uetd_status_t ready(uapp_product_session_t* s) {
    return s == NULL ? UETD_INVALID_ARG : !s->initialized ? UETD_NOT_READY : s->busy ? UETD_BUSY : UETD_OK;
}
static uetd_status_t fresh(uapp_product_session_t* s, const uapp_product_command_t* command) {
    ucore_clock_stamp_t now;
    uetd_status_t status = s->config.provider.now(s->config.provider_context, &now);
    if (status != UETD_OK) { return status; }
    bool expired;
    status = ucore_deadline_expired(command->expires, now, &expired);
    return status != UETD_OK ? status : expired ? UETD_TIMEOUT : UETD_OK;
}
uetd_status_t uapp_product_session_init(uapp_product_session_t* s, const uapp_product_session_config_t* c) {
    if (s == NULL || c == NULL || c->slots == NULL || c->maximum_pending_commands == 0u ||
        c->maximum_pending_commands > SIZE_MAX / sizeof(*c->slots) || c->peer.identity_schema == 0u ||
        c->provider.now == NULL || c->provider.authorize == NULL || c->provider.send == NULL ||
        c->provider.observe == NULL) { return UETD_INVALID_ARG; }
    if (s->initialized) { return UETD_BUSY; }
    s->config = *c;
    memset(c->slots, 0, c->maximum_pending_commands * sizeof(*c->slots));
    s->generation = 1u; s->last_command_id = 0u; s->cursor = 0u; s->busy = false; s->initialized = true;
    return UETD_OK;
}
uetd_status_t uapp_product_session_submit(uapp_product_session_t* s, const uapp_product_command_t* c) {
    uetd_status_t status = ready(s);
    if (status != UETD_OK) { return status; }
    if (c == NULL || c->id == 0u || c->schema_id == 0u || c->authority_generation == 0u ||
        c->expires.infinite || c->payload.size > UAPP_PRODUCT_PAYLOAD_CAPACITY ||
        (c->payload.size != 0u && c->payload.data == NULL)) { return UETD_INVALID_ARG; }
    if (c->session_generation != s->generation || c->id <= s->last_command_id) { return UETD_CONFLICT; }
    uapp_product_slot_t* slot = NULL;
    for (size_t i = 0; i < s->config.maximum_pending_commands; ++i) {
        if (!s->config.slots[i].occupied) { slot = &s->config.slots[i]; break; }
    }
    if (slot == NULL) { return UETD_OVERFLOW; }
    s->busy = true;
    status = fresh(s, c);
    if (status == UETD_OK) { status = s->config.provider.authorize(s->config.provider_context, &s->config.peer, c); }
    s->busy = false;
    if (status != UETD_OK) { return status; }
    slot->command = *c;
    if (c->payload.size != 0u) { memmove(slot->payload, c->payload.data, c->payload.size); }
    slot->command.payload.data = slot->payload;
    slot->outcome = (uapp_product_outcome_t){c->id, s->generation, UAPP_COMMAND_ADMITTED, UETD_OK};
    slot->occupied = true; s->last_command_id = c->id;
    return UETD_OK;
}
uetd_status_t uapp_product_session_poll(uapp_product_session_t* s, uetd_u16_t budget) {
    uetd_status_t status = ready(s);
    if (status != UETD_OK) { return status; }
    s->busy = true;
    size_t visited = 0u;
    unsigned worked = 0u;
    while (visited++ < s->config.maximum_pending_commands && worked < budget) {
        uapp_product_slot_t* slot = &s->config.slots[s->cursor];
        s->cursor = (s->cursor + 1u) % s->config.maximum_pending_commands;
        if (!slot->occupied || terminal(slot->outcome.state)) { continue; }
        ++worked;
        status = fresh(s, &slot->command);
        if (status != UETD_OK) {
            slot->outcome.state = slot->outcome.state == UAPP_COMMAND_ADMITTED && status == UETD_TIMEOUT ?
                                  UAPP_COMMAND_EXPIRED : UAPP_COMMAND_UNKNOWN;
            slot->outcome.result = status;
            continue;
        }
        if (slot->outcome.state == UAPP_COMMAND_ADMITTED) {
            status = s->config.provider.authorize(s->config.provider_context, &s->config.peer, &slot->command);
            if (status != UETD_OK) { slot->outcome.state = UAPP_COMMAND_REJECTED; slot->outcome.result = status; continue; }
            status = fresh(s, &slot->command);
            if (status != UETD_OK) {
                slot->outcome.state = status == UETD_TIMEOUT ? UAPP_COMMAND_EXPIRED : UAPP_COMMAND_UNKNOWN;
                slot->outcome.result = status;
                continue;
            }
            status = s->config.provider.send(s->config.provider_context, &s->config.peer, &slot->command);
            if (status == UETD_BUSY) { continue; }
            slot->outcome.state = status == UETD_OK ? UAPP_COMMAND_SENT : UAPP_COMMAND_UNKNOWN;
            slot->outcome.result = status;
        } else {
            uapp_product_outcome_t evidence = slot->outcome;
            status = s->config.provider.observe(s->config.provider_context, &s->config.peer, &evidence);
            if (status == UETD_NOT_READY || status == UETD_BUSY) { continue; }
            if (status != UETD_OK || evidence.command_id != slot->command.id ||
                evidence.session_generation != slot->command.session_generation ||
                evidence.state < slot->outcome.state || evidence.state > UAPP_COMMAND_UNKNOWN ||
                evidence.state == UAPP_COMMAND_EXPIRED ||
                (evidence.state <= UAPP_COMMAND_COMPLETED && evidence.result != UETD_OK)) {
                slot->outcome.state = UAPP_COMMAND_UNKNOWN;
                slot->outcome.result = status == UETD_OK ? UETD_CONFLICT : status;
            } else { slot->outcome = evidence; }
        }
    }
    s->busy = false;
    return UETD_OK;
}
uetd_status_t uapp_product_session_next_outcome(uapp_product_session_t* s, uapp_product_outcome_t* output) {
    uetd_status_t status = ready(s);
    if (status != UETD_OK) { return status; }
    if (output == NULL) { return UETD_INVALID_ARG; }
    uapp_product_slot_t* chosen = NULL;
    for (size_t i = 0; i < s->config.maximum_pending_commands; ++i) {
        uapp_product_slot_t* slot = &s->config.slots[i];
        if (slot->occupied && terminal(slot->outcome.state) &&
            (chosen == NULL || slot->command.id < chosen->command.id)) { chosen = slot; }
    }
    if (chosen == NULL) { return UETD_NOT_READY; }
    *output = chosen->outcome; chosen->occupied = false;
    return UETD_OK;
}
uetd_status_t uapp_product_session_invalidate(uapp_product_session_t* s) {
    uetd_status_t status = ready(s);
    if (status != UETD_OK) { return status; }
    if (s->generation == UINT64_MAX) { return UETD_OVERFLOW; }
    ++s->generation;
    for (size_t i = 0; i < s->config.maximum_pending_commands; ++i) {
        uapp_product_slot_t* slot = &s->config.slots[i];
        if (slot->occupied && !terminal(slot->outcome.state)) {
            slot->outcome.state = UAPP_COMMAND_UNKNOWN; slot->outcome.result = UETD_CONFLICT;
        }
    }
    return UETD_OK;
}
