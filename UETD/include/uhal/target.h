/// @file include/uhal/target.h
/// @brief Architecture-neutral target types and compiler attributes.

#ifndef UETD_UHAL_TARGET_H
#define UETD_UHAL_TARGET_H

#include <ucore/types.h>
#include <uhal/target_config.h>

/* Platform interrupt IDs; no vendor headers or fake CPU intrinsics here. */
typedef int32_t uhal_irq_t;
#define UHAL_IRQ_INVALID ((uhal_irq_t)-1)

#if defined(__GNUC__) || defined(__clang__)
#define UETD_SECTION(name) __attribute__((section(name)))
#define UETD_ALIGNED(bytes) __attribute__((aligned(bytes)))
#else
#define UETD_SECTION(name)
#define UETD_ALIGNED(bytes) __declspec(align(bytes))
#endif

#endif /* UETD_UHAL_TARGET_H */
