# UETD-managed third-party sources

## FreeRTOS Kernel

FreeRTOS Kernel 11.3.1 is staged in `FreeRTOS-Kernel-11.3.1/`. Its
`freertos-kernel.lock.json` records the official upstream repository, release
tag, resolved commit, and MIT license path. The directory is a regular vendored
copy rather than a Git submodule; the copied files have not been checksum-
compared with the upstream commit.

`uetd-integration.json` selects the core source files and provides a generic
`arm_cm4f_gcc` profile using the upstream `portable/GCC/ARM_CM4F` port. This
profile describes a CPU/compiler pairing. It does not qualify any particular
MCU or board. The generated firmware project still supplies a matching
`FreeRTOSConfig.h`, ARM cross compiler, ABI/FPU flags, startup and interrupt
configuration, linker script, and board-specific tick setup.

The UETD module's FreeRTOS backend can therefore build only when a generated
project supplies those settings. `uef-gen` still refuses target generation
because no exact target record is generation-available. Bare-metal projects
do not use this dependency. FatFS and SafeRTOS remain consumer-supplied.

The upstream repository declares partner/community port repositories as
optional submodules. They are not needed for the included kernel core or
ARM_CM4F GCC port; add one only when a selected target requires it.

Upstream: [FreeRTOS Kernel V11.3.1](https://github.com/FreeRTOS/FreeRTOS-Kernel/releases/tag/V11.3.1).
