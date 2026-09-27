# UHAL target implementations

Select exactly one target directory per UEF build:

- `arm_cm`: CMSIS-based Cortex-M implementation points. Supply a real device
  header, board clock/startup, register map, and linker configuration.
- `x86`: host simulation support. Simulated timing, GPIO, and backup values
  are not hardware models and must not be used as embedded behavior.

Portable UPAL, UMID, UPROTO, UOS, and UAPP sources include public UEF headers,
never a vendor device header. Add family- or board-specific source beneath
this directory when the selected target needs a distinct implementation.
