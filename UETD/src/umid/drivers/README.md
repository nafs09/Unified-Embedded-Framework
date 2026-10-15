# UMID driver implementations

The sensor and measurement implementation scaffolds live in this directory.
The public contracts are under `include/umid/drivers/` and are included by
the UMID umbrella header at `include/umid/umid.h`.

Add concrete device implementations manually here, grouped by UMID component
as needed (for example, `imu/<vendor>_<part>.c`). Use UPAL APIs rather than
accessing target registers directly. Preserve UMID's normalized output units,
caller-owned/static storage, bounded transfer behavior, timestamps, and error
propagation. Application-level sensor simulation and device discovery are
outside UETD's physical driver layer; no per-device registry or generation
pipeline is defined here.

`CMakeLists.txt` recursively collects `src/umid/**/*.c`, so new driver
sources enter the source build automatically. `uef-gen` uses explicit source
lists in the existing UMID component manifests; update the relevant manifest
when a generated firmware project needs a manually authored driver.

The `umid/fpga` module tracks MCU-side link qualification and the local receive
time/validity of complete protocol updates. The project's protocol declaration
owns the fields, wire encoding, sequencing, integrity checks, and message
freshness policy; UETD generates the MCU-side encoder/parser from that contract.
This module adds no peer identity or contract-ID/hash handshake. Target
transport bindings are application-specific. The FPGA design and bitstream stay
outside UETD.
