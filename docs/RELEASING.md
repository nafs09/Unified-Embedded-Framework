# Release procedure

This document is a release checklist for the maintainers. The Unified Embedded
Framework (UEF) consists of UETD and `uef-gen`. They have separate package
version declarations and may require coordinated releases because `uef-gen`
reads UETD's live API and registry contracts.

## Before tagging

1. Confirm the release scope and whether it includes UETD, uef-gen, or both.
2. Check that the chosen UETD API/registry version is accepted by the uef-gen
   version being released. Update compatibility metadata and user-facing
   documentation together.
3. Review `CHANGELOG.md`, both project READMEs, migration notes, and known
   limitations. Identify scaffolds, unavailable targets, and unverified
   hardware paths plainly.
4. Run the checks supported by the repository and record exact commands and
   results. Keep host builds, cross-compilation, static analysis, and physical
   hardware validation distinct in the release notes.
5. Check licensing per component. UETD and `uef-gen` together make up UEF and
   are licensed under GPL-3.0-only in the root [`LICENSE.md`](../LICENSE.md).
   Third-party dependencies retain their own notices. Keep package metadata
   and release assets within those boundaries.

## Tag and publish

Use a signed or otherwise authenticated tag according to the maintainer's
repository settings. Tag each package using its declared version and state in
the release notes which subproject(s) the tag covers. Include installation
instructions, supported combinations, validation evidence, migration notes,
and any known security or safety limitations. Attach checksums for binary
artifacts if any are published.

After publication, verify that the GitHub release page, package metadata,
README installation commands, and version compatibility checks point to the
same release. Add any follow-up work to the appropriate subproject's
implementation inventory.
