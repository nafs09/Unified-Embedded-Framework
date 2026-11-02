/// @file include/usec/provider.h
/// @brief First-pass scaffold: Shared entropy, key and crypto provider boundary.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/CONNECTIVITY_SECURITY.md#security-service-boundary.
#ifndef UETD_USEC_PROVIDER_H
#define UETD_USEC_PROVIDER_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/span.h"

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
typedef struct { uetd_u32_t slot; uetd_u64_t generation; } usec_key_handle_t;
typedef enum { USEC_HASH = 0, USEC_MAC, USEC_VERIFY, USEC_SIGN, USEC_AEAD_SEAL, USEC_AEAD_OPEN } usec_operation_t;
typedef struct { usec_operation_t operation; uetd_u32_t algorithm_profile; usec_key_handle_t key; ucore_read_span_t input, nonce, associated_data, expected_tag; } usec_request_t;
typedef struct { uetd_u64_t principal, target; uetd_u32_t operation, context; } usec_authorization_t;
typedef struct { void* provider_context; uetd_u32_t maximum_random_bytes; bool production_entropy_required; } usec_config_t;
typedef struct { void* provider_context; uetd_u64_t generation; bool initialized; } usec_provider_t;

/// Bind a vetted pinned provider, declared algorithms, trust/key storage and production entropy health policy.
uetd_status_t usec_init(usec_provider_t* security, const usec_config_t* config);
/// Require entropy/DRBG readiness and a bounded request; simulation data must not masquerade as production entropy.
uetd_status_t usec_random(usec_provider_t* security, ucore_write_span_t output);
/// Validate algorithm-specific lengths/key use and constant-time provider requirements; authenticated decryption cannot publish unauthenticated plaintext.
uetd_status_t usec_execute(usec_provider_t* security, const usec_request_t* request, ucore_write_span_t output, size_t* written);
/// Resolve principal/capability/target/context using product policy; default denial must never be reported as permission.
uetd_status_t usec_authorize(usec_provider_t* security, const usec_authorization_t* request);
/// Validate key generation/use policy, quiesce users and invalidate the handle without leaking material.
uetd_status_t usec_key_destroy(usec_provider_t* security, usec_key_handle_t key);

#ifdef __cplusplus
}
#endif
#endif /* UETD_USEC_PROVIDER_H */
