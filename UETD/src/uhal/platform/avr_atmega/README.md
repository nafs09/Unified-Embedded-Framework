# ATmega platform work area

Implement the exact part's DDR/PORT/PIN, clock/reset, interrupt-source routing,
retained-memory policy and monotonic timer here. AVR CPU operations live in
`../../arch/avr/`. Arduino runtime/build integration belongs to the Arduino
adapter, not this chip-family implementation. No physical development board is
required: the chip plus a custom assembly and programmer profile is sufficient
once its implementation and metadata are complete.

The `.c.todo` files are not compiled. See [the porting guide](../../../../docs/design/UHAL_PORTING.md).
