/// @file src/uos/deferred.c
#include "uos/deferred.h"
#include <string.h>

uetd_status_t uos_deferred_init(uos_deferred_t* d, const uos_deferred_config_t* config) {
    if (d == NULL || config == NULL || config->slots == NULL || config->capacity == 0u ||
        config->capacity > SIZE_MAX / sizeof(*config->slots)) { return UETD_INVALID_ARG; }
    if (d->initialized) { return UETD_BUSY; }
    d->config = *config;
    memset(config->slots, 0, config->capacity * sizeof(*config->slots));
    d->generation = 1u; d->next_id = 1u; d->running_id = 0u;
    d->last_poll = config->clock; d->polling = false; d->initialized = true;
    return UETD_OK;
}

uetd_status_t uos_deferred_submit(uos_deferred_t* d, const uos_deferred_item_t* item, uos_deferred_token_t* token) {
    if (d == NULL || item == NULL || token == NULL || item->callback == NULL || item->release.infinite) {
        return UETD_INVALID_ARG;
    }
    if (!d->initialized) { return UETD_NOT_READY; }
    bool ignored;
    const uetd_status_t status = ucore_deadline_expired(item->release, d->last_poll, &ignored);
    if (status != UETD_OK) { return status; }
    if (d->next_id == UINT64_MAX) { return UETD_OVERFLOW; }
    for (size_t i = 0; i < d->config.capacity; ++i) {
        uos_deferred_slot_t* slot = &d->config.slots[i];
        if (!slot->occupied) {
            slot->item = *item; slot->token.id = d->next_id++; slot->token.generation = d->generation;
            slot->occupied = true; *token = slot->token;
            return UETD_OK;
        }
    }
    return UETD_OVERFLOW;
}

uetd_status_t uos_deferred_cancel(uos_deferred_t* d, uos_deferred_token_t token) {
    if (d == NULL || token.id == 0u) { return UETD_INVALID_ARG; }
    if (!d->initialized) { return UETD_NOT_READY; }
    if (token.generation != d->generation) { return UETD_CONFLICT; }
    if (d->running_id == token.id) { return UETD_BUSY; }
    for (size_t i = 0; i < d->config.capacity; ++i) {
        uos_deferred_slot_t* slot = &d->config.slots[i];
        if (slot->occupied && slot->token.id == token.id) { slot->occupied = false; return UETD_OK; }
    }
    return UETD_NOT_READY;
}

uetd_status_t uos_deferred_poll(uos_deferred_t* d, ucore_clock_stamp_t now, uetd_u16_t budget) {
    if (d == NULL) { return UETD_INVALID_ARG; }
    if (!d->initialized) { return UETD_NOT_READY; }
    if (d->polling) { return UETD_BUSY; }
    if (now.domain != d->last_poll.domain || now.generation != d->last_poll.generation ||
        now.ticks_us < d->last_poll.ticks_us) { return UETD_CONFLICT; }
    const uetd_u64_t cutoff = d->next_id;
    d->last_poll = now; d->polling = true;
    for (uetd_u32_t n = 0; n < budget; ++n) {
        uos_deferred_slot_t* selected = NULL;
        for (size_t i = 0; i < d->config.capacity; ++i) {
            uos_deferred_slot_t* slot = &d->config.slots[i];
            if (slot->occupied && slot->token.id < cutoff && slot->item.release.expires.ticks_us <= now.ticks_us &&
                (selected == NULL || slot->token.id < selected->token.id)) { selected = slot; }
        }
        if (selected == NULL) { break; }
        const uos_deferred_item_t item = selected->item;
        d->running_id = selected->token.id; selected->occupied = false;
        item.callback(item.context);
        d->running_id = 0u;
    }
    d->polling = false;
    return UETD_OK;
}

uetd_status_t uos_deferred_reset(uos_deferred_t* d, ucore_clock_stamp_t clock) {
    if (d == NULL) { return UETD_INVALID_ARG; }
    if (!d->initialized) { return UETD_NOT_READY; }
    if (d->polling) { return UETD_BUSY; }
    if (d->generation == UINT64_MAX) { return UETD_OVERFLOW; }
    memset(d->config.slots, 0, d->config.capacity * sizeof(*d->config.slots));
    ++d->generation; d->next_id = 1u; d->last_poll = clock; d->config.clock = clock;
    return UETD_OK;
}
