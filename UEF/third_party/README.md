# UEF-managed third-party sources

UEF keeps reusable upstream source dependencies in this directory when their
kernel/runtime sources are part of the selected UEF module. The dependency
lock records the exact upstream commit and license; the integration manifest
records the source files and include directories UEF supports for each
target/compiler profile.

## FreeRTOS Kernel

The FreeRTOS Kernel directory is intentionally not populated yet. Before
enabling `uos/freertos` in a released UEF checkout:

1. Add the upstream source at `freertos-kernel/` without editing its contents.
2. Record the full upstream commit SHA, MIT license identifier, and exact
   license-text file path(s) in `freertos-kernel.lock.json`; do not use a
   floating branch or tag. Generated projects and UEF install packages retain
   those license files beside the selected dependency source.
3. Populate `freertos-kernel/uef-integration.json` with the common kernel
   sources and one reviewed profile for each supported architecture/compiler
   pairing. Each profile lists its port source, heap implementation, and
   include directories.
4. Verify `FreeRTOSConfig.h` requirements, interrupt priority rules, tick
   source, ISR yield behavior, and static-allocation configuration for that
   target. Keep the board-specific values in the generated firmware project.
5. Build and exercise the port on hardware before marking the profile
   supported in the UEF manifest.

Until those records are complete, uef-gen must refuse to publish a
FreeRTOS-targeted project. Bare-metal projects remain independent of this
directory. FatFS and SAFERTOS remain consumer-supplied dependencies.
