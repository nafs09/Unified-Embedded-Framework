/// @file include/upal/display/display_panel.h
/// @brief First-pass scaffold: Bounded panel profile and initialization sequencing.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/MIPI_DISPLAY_LINK.md#required-provider-and-panel-profile.
#ifndef UETD_UPAL_DISPLAY_PANEL_H
#define UETD_UPAL_DISPLAY_PANEL_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/display/display.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Intended implementation: serialized task/main owner; no hidden heap.
 * Configurations, provider context and backing storage outlive the object.
 * Explicitly asynchronous operations retain buffers only after admission and
 * until terminal completion; synchronous spans are borrowed only for the call.
 * Schema IDs bind typed, versioned layouts/units; they do not permit arbitrary
 * native structs as a wire representation. Output/state commits and callback
 * context must be reviewed against the linked design before implementation.
 */
typedef enum { UPAL_PANEL_DCS = 0, UPAL_PANEL_GENERIC, UPAL_PANEL_DELAY, UPAL_PANEL_RESET, UPAL_PANEL_SUPPLY } upal_panel_step_kind_t;
typedef struct { upal_panel_step_kind_t kind; const uetd_u8_t* bytes; size_t length; uetd_u32_t delay_us; uetd_u16_t resource; bool asserted; } upal_panel_step_t;
typedef struct { const upal_panel_step_t* steps; size_t step_count; uetd_u32_t maximum_startup_us; uetd_u32_t profile_revision; upal_display_info_t display; void* provider_context; } upal_panel_profile_t;
typedef struct { void* provider_context; size_t next_step; uetd_u64_t generation; bool initialized; } upal_panel_t;

/// Validate bounded immutable command data, resource references, waits and compatible display formats.
uetd_status_t upal_panel_validate(const upal_panel_profile_t* profile);
/// Bind the panel and its transport/power providers; do not begin unbounded synchronous initialization.
uetd_status_t upal_panel_init(upal_panel_t* panel, const upal_panel_profile_t* profile);
/// Begin the deadline-governed reset/supply/command sequence and enforce its ordering.
uetd_status_t upal_panel_start(upal_panel_t* panel);
/// Advance bounded script work; delay steps yield rather than busy-waiting.
uetd_status_t upal_panel_poll(upal_panel_t* panel, uetd_u16_t work_budget);
/// Expose the adapter-owned display facade only after the panel profile has initialized successfully.
uetd_status_t upal_panel_get_display(upal_panel_t* panel, upal_display_t** display);
/// Quiesce transfers before ordered display-off/supply teardown and invalidate presentation state.
uetd_status_t upal_panel_stop(upal_panel_t* panel);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPAL_DISPLAY_PANEL_H */
