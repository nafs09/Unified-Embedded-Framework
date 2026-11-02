/// @file src/upal/transaction.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "upal/transaction.h"

uetd_status_t upal_transaction_begin(upal_transaction_t* transaction, upal_transaction_token_t token, ucore_deadline_t deadline, size_t borrowed_bytes) {
    /* TODO: Validate idle state, non-repeating token and finite deadline; admit ownership once. */
    (void)transaction;
    (void)token;
    (void)deadline;
    (void)borrowed_bytes;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_transaction_request_cancel(upal_transaction_t* transaction, upal_transaction_token_t token) {
    /* TODO: Request cancellation without releasing buffers or manufacturing a terminal completion. */
    (void)transaction;
    (void)token;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_transaction_complete(upal_transaction_t* transaction, const upal_transaction_completion_t* completion) {
    /* TODO: Require matching token, legal byte count and proven quiescence before releasing ownership. */
    (void)transaction;
    (void)completion;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t upal_transaction_query(const upal_transaction_t* transaction, upal_transaction_state_t* state) {
    /* TODO: Return a coherent serialized snapshot without altering the transaction. */
    (void)transaction;
    (void)state;
    return UETD_NOT_SUPPORTED;
}

