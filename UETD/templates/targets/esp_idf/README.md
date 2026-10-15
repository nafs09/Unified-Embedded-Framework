# ESP-IDF / classic ESP32 target integration template scaffold

ESP-IDF owns the classic ESP32 reset/startup flow, bootloader, linker
fragments, partition table, compiler flags, image conversion, and normal
`esptool` programming path. uef-gen should generate an IDF component/project
adapter and feed UETD sources into IDF's component model; it should not invent
a second standalone linker/startup pipeline.

## Generated files to design

- Root `CMakeLists.txt`: call the pinned ESP-IDF project setup and declare the
  application; do not mix with generic cross-CMake setup.
- `main/CMakeLists.txt`: register selected UETD/generated sources and include
  directories with `idf_component_register`, target requirements, and compile
  definitions.
- `sdkconfig.defaults`: only values proven by the board profile; retain an
  explicit review marker for flash mode, flash frequency, partition table,
  console UART, bootloader, and CPU frequency.
- `partitions.csv`: derive offset/size from the ESP-WROOM-32 module flash and
  user-selected OTA/data policy; never assume 4 MiB for every WROOM variant.
- `manifest/target_build.json`: ESP-IDF release, target (classic ESP32 Xtensa),
  component closure, IDF Python environment, image formats, partition hash,
  and exact flash profile.

TODO(esp-idf-template): coordinate `idf.py build`, artifact discovery, image
hash/address extraction, `esptool` identity/program/verify/reset, and serial
console selection through UETD-pinned package records. Keep monitor attach
read-only: no reset-line toggle when monitoring unless the board profile and
user explicitly request that behavior.
