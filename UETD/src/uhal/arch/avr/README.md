# AVR architecture work area

Implement CPU-specific interrupt-state save/restore, barriers, atomic fallbacks
and reset/fault primitives here. The `.c.todo` files are headings, not sources.
The corresponding chip-specific GPIO, clocks, retained memory and monotonic
timer work belongs in `../../platform/avr_atmega/`.

Public headers now use `uhal_irq_t` and backend critical-section functions;
there is no CMSIS/PRIMASK fallback. An AVR implementation must save SREG, mask
interrupts briefly and restore the caller's state. Thirty-two-bit operations
need protection on an eight-bit CPU. AVR has no BASEPRI priority mask.

AVR-GCC/avr-libc startup and an optional Arduino core are separate runtime/build
choices. Arduino is not the architecture. See the full [porting guide](../../../../docs/design/UHAL_PORTING.md)
for MCU identity, timer ownership, memory budgets and programmer requirements.
