/// @file src/uhal/arm_cm/timing.c
/// @brief Source scaffold for the V1.1 public contract in uef/uhal/uhal_core.h.
///
/// Implementation intent: Configure and read DWT or SysTick timing only after the board clock
///   is established; document wrap and resolution.
///
/// This file deliberately does not invent target behavior or algorithm bodies.
/// Replace this note with bounded, allocation-free implementations and retain
/// the public contract in the paired header.
#include <uef/uhal/uhal_core.h>

/* Keep this translation unit valid while the implementation is pending. */
typedef int uef_timing_implementation_pending_t;
