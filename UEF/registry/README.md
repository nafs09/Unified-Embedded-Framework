# UEF registries

Every tileable UEF catalogue lives entirely under this directory. Each catalogue
has one registry.json index and a manifest subtree with one detail file per
record, plus catalogue-specific schemas where they are defined. Registry paths in JSON are repository-root-relative and are declared by
uef_api.json where a consumer needs to locate the catalogue.

    registry/
      targets/
        families/
          registry.json
          registry.schema.json
          manifest/<vendor>/<family>.json
        chips/
          registry.json
          registry.schema.json
          record.schema.json
          manifest/<vendor>/<part>.json
      modules/
        registry.json
        manifest/<module path>.json
      control_ir/
        registry.json
        manifest/
          algorithms/<family>/<algorithm>.json
          aliases/<alias>.json
          control_nodes/<node>.json

To add a record, add its detail file under that catalogue's manifest subtree
and add one reference to registry.json. Keep shared selection rules and
catalogue-wide policy in the index. Do not create root-level aliases, pointer
documents, or duplicate copies of detail data.

uef_api.json is the public contract that points to the module, target-family,
chip-base, and ControlIR indexes. It is not a catalogue of every record.
uef-gen reads those paths and manifests from the live UEF checkout; it does
not own a second copy of UEF data.
