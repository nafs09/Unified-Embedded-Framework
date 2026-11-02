# Incomplete peripheral examples

These sources contain TODOs for an exact SoC/platform. They are intentionally
excluded from `CORTEX_M/CUSTOM` builds; they must not make a hardware library
appear implemented. CPU primitives live under `../../arch/cortex_m/`.

A platform supplies clock queries/gates/reset, GPIO mapping, memory and retained
storage, CPU frequency, an optional extended cycle counter and the monotonic
microsecond clock/delays. Startup, linker layout, chip capabilities and the
physical assembly must agree. See [the porting guide](../../../../docs/design/UHAL_PORTING.md).
