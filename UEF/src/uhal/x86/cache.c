/// @file src/uhal/x86/cache.c
/// @brief Host memory is coherent; DMA cache behavior is not simulated.

#include "uef/uhal/uhal_cache.h"

/* UHAL_HAS_DCACHE is zero on the host target, so public cache macros erase calls at compile time.
 * The simulator does not model DMA cache effects. */
