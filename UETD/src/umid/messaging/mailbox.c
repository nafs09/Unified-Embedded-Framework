/// @file src/umid/messaging/mailbox.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "umid/messaging/mailbox.h"

uetd_status_t umid_mailbox_init(umid_mailbox_t* mailbox, const umid_mailbox_config_t* config) {
    /* TODO: Validate fixed-size storage and a real synchronization route for the declared producer/consumer topology. */
    (void)mailbox;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_mailbox_publish(umid_mailbox_t* mailbox, ucore_read_span_t value, const umid_mailbox_metadata_t* metadata) {
    /* TODO: Commit the entire value and metadata coherently; detect sequence exhaustion/reset. */
    (void)mailbox;
    (void)value;
    (void)metadata;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_mailbox_read(const umid_mailbox_t* mailbox, ucore_write_span_t value, umid_mailbox_metadata_t* metadata) {
    /* TODO: Read one coherent generation, preserving destination on no value or capacity failure. */
    (void)mailbox;
    (void)value;
    (void)metadata;
    return UETD_NOT_SUPPORTED;
}

