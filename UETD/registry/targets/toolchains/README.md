# Target tool package registry

This registry is the UETD-owned source for toolchain, SDK, build-system, and
flash-tool package pins. `uef-gen` must read it through `api.json`; it must
not keep another target-tool version catalogue in Python or project YAML.

## Add or complete a package

1. Create `manifest/<tool-id>.json` and add one `{id, path}` row to
   `registry.json`.
2. Keep a new record at `status: "scaffold_only"` until every supported host
   has an exact upstream release, package URL, SHA-256, extraction rule,
   executable map, license identifier, and any interactive acceptance step.
3. Set `status: "ready"` only after a clean install and version check work for
   every declared host. The schema requires at least one complete package for
   ready records.
4. Let board `build.toolchain` and `flash.backend` refer to these stable IDs.
   Keep machine-specific install paths out of UETD; resolve them under a
   user-selected tools root at runtime.

## Package record shape

- `purpose`: compiler, SDK, flasher, build-system, debugger, or support.
- `host_platforms`: the host OS/architecture variants actually supplied.
- `tools`: executable names consumers need, without absolute paths.
- `packages`: exact versioned assets. Each asset declares binary/source kind,
  URL, SHA-256, host support, extraction details, executable paths, and license
  handling.
- `dependencies`: IDs of other UETD package records, not pip's transitive
  dependencies.

The seeded records are intentionally non-installable examples for the target
compiler/SDKs and the host tools already named by uef-gen's build, flash,
discovery, and monitor backends: Arm GNU, AVR-GCC, Arduino CLI plus AVR/megaAVR
cores, STM32Cube G4/F7/H7 packages, ESP-IDF, CMake, Ninja, avrdude,
STM32CubeProgrammer/ST-LINK, esptool, OpenOCD, pyOCD, J-Link, dfu-util,
TI UniFlash, pySerial, and PyUSB. Fill in vendor-published versions
and hashes as packages are selected; do not turn guessed URLs or unverified
checksums into pins.

The corresponding consumer-side source seams are
`uef-gen/src/uef_gen/toolchains/catalog.py` (registry load/validation),
`bootstrap.py` (host selection, dependency plan, install/verify), and
`profiles.py` (board build/flash profile to a local package/executable
selection). A package marked `ready` is not sufficient by itself: the selected
board profile must also declare a compatible build backend, flash backend,
artifact format, and exact hardware settings.
