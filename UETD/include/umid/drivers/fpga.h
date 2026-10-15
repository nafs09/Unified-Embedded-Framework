/// @file include/umid/drivers/fpga.h
/// @brief MCU-side endpoint state for a project-declared FPGA protocol.

#ifndef UMID_FPGA_H
#define UMID_FPGA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/time.h"
#include "ucore/types.h"

/// Local state for one MCU-to-FPGA communications endpoint.
///
/// This record deliberately contains no wire format, peer identity, contract
/// hash, transport driver, or FPGA implementation. A project-declared UPROTO
/// contract defines messages and timing; its MCU transport/parser reports
/// link qualification and complete validated messages here.
typedef struct {
    bool link_qualified;
    bool has_valid_data;
    uetd_time_us_t last_valid_message_at_us;
} umid_fpga_t;

/// Initialize endpoint bookkeeping in the disconnected, data-invalid state.
void umid_fpga_init(umid_fpga_t* endpoint);

/// Update link qualification from the selected transport/protocol layer.
/// Losing qualification also invalidates cached received data.
uetd_status_t umid_fpga_set_link_qualified(umid_fpga_t* endpoint, bool qualified);

/// Record the MCU-local receive time of a complete, validated protocol update.
/// The parser/assembler must call this only after its framing, sequence,
/// transaction, and checksum rules have passed.
uetd_status_t umid_fpga_note_valid_message(umid_fpga_t* endpoint, uetd_time_us_t received_at_us);

/// Invalidate cached data after timeout, malformed transaction, or reset.
void umid_fpga_invalidate_data(umid_fpga_t* endpoint);

/// Report whether a valid complete update has been observed on a qualified link.
bool umid_fpga_has_valid_data(const umid_fpga_t* endpoint);

/// Report whether the endpoint is currently link-qualified.
bool umid_fpga_is_connected(const umid_fpga_t* endpoint);

/// Check data age in the receiver's monotonic clock domain.
/// A timestamp from the FPGA or another MCU must never be passed as `now_us`.
uetd_status_t umid_fpga_is_data_fresh(const umid_fpga_t* endpoint, uetd_time_us_t now_us, uetd_dur_us_t maximum_age_us, bool* fresh);

/// Return the receiver-local timestamp of the most recent valid update.
uetd_status_t umid_fpga_last_valid_message_time(const umid_fpga_t* endpoint, uetd_time_us_t* received_at_us);

#ifdef __cplusplus
}
#endif

#endif /* UMID_FPGA_H */
