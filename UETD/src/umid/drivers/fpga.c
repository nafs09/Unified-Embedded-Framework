/// @file src/umid/drivers/fpga.c
/// @brief MCU-side endpoint qualification and receive-freshness bookkeeping.

#include "umid/drivers/fpga.h"

void umid_fpga_init(umid_fpga_t* endpoint) {
    if (endpoint == NULL) { return; }
    endpoint->link_qualified = false;
    endpoint->has_valid_data = false;
    endpoint->last_valid_message_at_us = 0U;
}

uetd_status_t umid_fpga_set_link_qualified(umid_fpga_t* endpoint, bool qualified) {
    if (endpoint == NULL) { return UETD_INVALID_ARG; }
    endpoint->link_qualified = qualified;
    if (!qualified) { umid_fpga_invalidate_data(endpoint); }
    return UETD_OK;
}

uetd_status_t umid_fpga_note_valid_message(umid_fpga_t* endpoint, uetd_time_us_t received_at_us) {
    if (endpoint == NULL) { return UETD_INVALID_ARG; }
    if (!endpoint->link_qualified) { return UETD_NOT_READY; }
    endpoint->last_valid_message_at_us = received_at_us;
    endpoint->has_valid_data = true;
    return UETD_OK;
}

void umid_fpga_invalidate_data(umid_fpga_t* endpoint) {
    if (endpoint == NULL) { return; }
    endpoint->has_valid_data = false;
    endpoint->last_valid_message_at_us = 0U;
}

bool umid_fpga_has_valid_data(const umid_fpga_t* endpoint) {
    return endpoint != NULL && endpoint->link_qualified && endpoint->has_valid_data;
}

bool umid_fpga_is_connected(const umid_fpga_t* endpoint) { return endpoint != NULL && endpoint->link_qualified; }

uetd_status_t umid_fpga_is_data_fresh(const umid_fpga_t* endpoint, uetd_time_us_t now_us, uetd_dur_us_t maximum_age_us,
                                     bool* fresh) {
    if (endpoint == NULL || fresh == NULL || maximum_age_us == 0U) { return UETD_INVALID_ARG; }
    *fresh = umid_fpga_has_valid_data(endpoint) && now_us >= endpoint->last_valid_message_at_us &&
             now_us - endpoint->last_valid_message_at_us <= maximum_age_us;
    return UETD_OK;
}

uetd_status_t umid_fpga_last_valid_message_time(const umid_fpga_t* endpoint, uetd_time_us_t* received_at_us) {
    if (endpoint == NULL || received_at_us == NULL) { return UETD_INVALID_ARG; }
    if (!umid_fpga_has_valid_data(endpoint)) { return UETD_NOT_READY; }
    *received_at_us = endpoint->last_valid_message_at_us;
    return UETD_OK;
}
