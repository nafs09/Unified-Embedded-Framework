/// @file src/uhal/x86/timing.c
/// @brief Source scaffold for the V1.1 public contract in uef/uhal/uhal_core.h.
///
/// Implementation intent: Provide monotonic host timestamps and state their resolution; avoid
///   wall-clock time for elapsed-time calculations.
///
/// This file deliberately does not invent target behavior or algorithm bodies.
/// Replace this note with bounded, allocation-free implementations and retain
/// the public contract in the paired header.
#include <uef/uhal/uhal_core.h>

/* Keep this translation unit valid while the implementation is pending. */
typedef int uef_timing_implementation_pending_t;
