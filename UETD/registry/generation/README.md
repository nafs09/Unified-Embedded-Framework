# First-pass host generation obligations

This catalogue records eleven host pipeline capabilities separately from
embedded C module identities. It covers typed emission, provider resolution,
target resources, memory/linker planning, codecs, builds, artifact signing,
deployment, decoding, manufacturing and qualification.

Every record is FIRST_PASS, Phase 0, design-only and generation-blocked.
Existing frontend/scaffold code does not establish a completed source-to-image
route. The `api_outline` reference identifies the current pipeline boundary;
it is not evidence that the named stage is implemented there.

`api.json` declares `generation_catalogue`; `uef-gen` exposes the read-only
`load_generation_catalogue()` loader. The normal module selector does not
consume this index. Its records cannot act as embedded module IDs.

The owning design is [Generation and qualification](../../docs/design/complete-system/GENERATION_QUALIFICATION.md).
