# Target and chip data

UEF owns the family roadmap and exact-part chip-base records. uef-gen reads the
API-declared indexes at `registry/targets/families/registry.json` and
`registry/targets/chips/registry.json`, then loads each detail file referenced by those
indexes. The family registry covers the staged vendor/series roadmap. The
chip registry currently points to 38 exact-part records with vendor-sourced
identity, core, package, headline memory and peripheral-family facts. The
readers validate index schemas, chip-record schemas, unique part names,
non-generation policy, and each chip's `family_id` link. `jsonschema` is a
required runtime dependency; if it is missing, catalogue loading fails with a
direct error.

A chip-base record is not a complete ChipSpec. It has no generation permission
and cannot pass hardware resource resolution. Complete target enablement needs
part- and package-specific pin/alternate-function maps, peripheral instances,
clock/reset trees, DMA requests, interrupts, memory regions, startup/linker
data, errata review, and a board binding.

ChipDatabase remains the resolver's registry for complete records. The
generic-reference record is only for pipeline examples and is explicitly
unverified. Do not copy UEF catalogue data into this package or promote a
chip-base row into DEFAULT_DATABASE until its missing resource contracts have
been verified.
