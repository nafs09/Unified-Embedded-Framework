# Target and chip data

UETD owns the family roadmap, exact-part chip records, and development-board
records. `uef-gen` reads the API-declared indexes under
`registry/targets/` and loads every indexed detail file from the located UETD
checkout. `ChipDatabase` is only an in-memory resolver index built from those
records; Python contains no parallel chip catalogue or default chip specs.

A chip record can start with sourced identity, core, package, and headline
memory facts. Keep `generation_available` false until that same record also has
the package pin/alternate-function map, peripheral instances and capabilities,
clock/reset tree, DMA/IRQ maps, memory regions, startup/linker data, and errata
review. The separate development-board record adds board clocks and reserved
pins plus its build and flash profiles. Optional module facts, such as
ESP-WROOM-32 flash and antenna details, belong inline in that board record.

The `generic-reference` entry is a non-physical fixture retained for examples;
it is deliberately marked `target_kind: pipeline_fixture`. Never add chip or
board facts to Python as a fallback: add them to the UETD record and index, then
let the resolver materialize them at runtime. See
`UETD/registry/targets/README.md` for compact record examples.
