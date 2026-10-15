This packaged schema directory mirrors the repository-level schemas and is
included in wheels. The two copies are required to keep schema references
available both in a checkout and an installed standalone uef-gen package.

`artifact.schema.json` defines the versioned build-sidecar contract. Its
`segments` array carries each programmed image's exact role, hash, format, and
half-open flash range; this is required for ESP-IDF bootloader/partition/app
image sets and other multi-image targets. `deploy/artifact_store.py` owns
hash/path validation and durable sidecar I/O. Update the schema and loader
together when adding fields; do not accept unknown versions silently.
