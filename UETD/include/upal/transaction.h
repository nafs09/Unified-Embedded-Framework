/// @file include/upal/transaction.h
/// @brief First-pass scaffold: Common asynchronous I/O token and ownership contract.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/FOUNDATIONS.md#driver-transaction-and-dma-ownership.
#ifndef UETD_UPAL_TRANSACTION_H
#define UETD_UPAL_TRANSACTION_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/deadline.h"

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
typedef struct { uetd_u64_t id; uetd_u32_t generation; } upal_transaction_token_t;
typedef enum { UPAL_TRANSACTION_IDLE = 0, UPAL_TRANSACTION_ACTIVE, UPAL_TRANSACTION_CANCELLING, UPAL_TRANSACTION_TERMINAL } upal_transaction_state_t;
typedef struct { upal_transaction_token_t token; ucore_deadline_t deadline; upal_transaction_state_t state; size_t borrowed_bytes; } upal_transaction_t;
typedef struct { upal_transaction_token_t token; uetd_status_t result; size_t transferred_bytes; bool buffers_released; } upal_transaction_completion_t;

/// Validate idle state, non-repeating token and finite deadline; admit ownership once.
uetd_status_t upal_transaction_begin(upal_transaction_t* transaction, upal_transaction_token_t token, ucore_deadline_t deadline, size_t borrowed_bytes);
/// Request cancellation without releasing buffers or manufacturing a terminal completion.
uetd_status_t upal_transaction_request_cancel(upal_transaction_t* transaction, upal_transaction_token_t token);
/// Require matching token, legal byte count and proven quiescence before releasing ownership.
uetd_status_t upal_transaction_complete(upal_transaction_t* transaction, const upal_transaction_completion_t* completion);
/// Return a coherent serialized snapshot without altering the transaction.
uetd_status_t upal_transaction_query(const upal_transaction_t* transaction, upal_transaction_state_t* state);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPAL_TRANSACTION_H */
