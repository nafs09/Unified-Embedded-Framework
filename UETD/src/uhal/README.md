# UHAL implementation map

Read [the porting and target pipeline guide](../../docs/design/UHAL_PORTING.md)
for the contracts, migration notes, exact implementation tasks and current limits.

| Scope | Directory | Status |
|---|---|---|
| CPU | `arch/cortex_m` | CPU primitives; private exact-device CMSIS boundary. Register-model tested. |
| CPU | `arch/avr` | Development headings, not selectable. |
| Platform | `platform/host` | Host simulation and nested thread-safe critical sections. |
| Platform | `platform/avr_atmega` | Exact AVR GPIO/clock/memory/time work area. |
| Platform/SDK | `platform/esp_idf` | ESP-IDF wrappers work area for supported ESP SoCs; not selectable. |
| Incomplete examples | `platform/cortex_m_scaffold` | GPIO, clock, memory and time TODOs; excluded from hardware builds. |
| Metadata | `targets` | Family capability records; not peripheral drivers. |

`src/uhal/sources.cmake` composes architecture and platform sources using `UETD_ARCH`
and `UETD_PLATFORM`. `CORTEX_M/CUSTOM` requires exact CMSIS/device headers and
`UETD_UHAL_PLATFORM_SOURCES`; CPU type alone cannot supply GPIO or a clock tree.
Unknown/unfinished architectures never fall back to simulation.

Physical targets are exact chips plus assembly, startup/build and programming
settings. A development-board record is an optional preset. UEF-PL's `hardware`
block can supply custom settings without a catalogue board. SDK templates live
under `templates/targets`; flash recipes and host tools are not CPU drivers.

Legacy module IDs `uhal/arm_cm` and `uhal/x86` remain stable registry aliases;
the actual source paths now describe architecture versus platform scope.
