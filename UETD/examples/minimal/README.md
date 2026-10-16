# Minimal lifecycle example

This portable example demonstrates UAPP component initialization, start, and
stop ordering without configuring board hardware or peripherals. It is the
example built when UETD is configured with `-DUETD_BUILD_EXAMPLE=ON`.

The example exercises lifecycle calls only; it does not demonstrate a target
port, scheduling policy, persistence, or an application-specific safe state.
