# ESP-IDF platform work area

This is an SDK/platform adapter, not an ISA. It can share supported API wrappers
across Xtensa and RISC-V ESP SoCs while the target record identifies the exact
CPU/SoC. Classic ESP32 is Xtensa LX6; ESP32-C3 is RISC-V. The `.c.todo` files are
unimplemented headings and are not selected by CMake or firmware generation.

The public contracts no longer require CMSIS. Implement them against a pinned
ESP-IDF release, including SMP-safe exclusion, allocated IRQ mappings,
monotonic time, reset/GPIO/clock services and truthful capability/error behavior.
Do not export Cortex-M exception handlers. ESP-IDF owns startup, bootloader,
linker fragments, partitions and its FreeRTOS kernel. UETD's vanilla FreeRTOS
adapter requires an IDF-specific integration, including stack-unit review.

Physical assembly settings can describe any bare SoC/module/custom PCB. A
DevKit record is only a preset. Record module/external flash and wiring with
that assembly, not as nonexistent on-chip SoC resources. Keep build/image and
esptool deployment adapters separate. See [the porting guide](../../../../docs/design/UHAL_PORTING.md).
