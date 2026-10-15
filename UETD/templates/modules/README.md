# Standalone module template scope

`index.json` and the adjacent `route.tmpl` files enumerate every module in
`registry/modules/registry.json`. They are scope checklists for the standalone
`.uef` project route. Each checklist names the source and public headers from
that module's detail manifest so the eventual route can reuse UETD's implementation.
[`catalog.md`](catalog.md) is the compact, sortable-by-domain overview; `index.json`
contains the full machine-readable mapping, including API symbols and template candidates.

These files contain comments only. They are deliberately non-renderable and
are not added to any module's `authoring.generation.template_paths`. The
standalone frontend and project-generation pipeline are not yet connected to
these files. NEXUS-specific template paths are private and are not part of the
public module workflow.

Every catalog entry gets a checklist so that missing decisions are visible;
this does **not** mean every entry ultimately needs its own generated source
file. Each checklist asks whether the item needs a component binding or is
only a dependency/library selection. Modules whose manifest stage or source
tree is explicitly `future/` are placed under a matching `future/` directory
here too.

The generator records unresolved authoring and generation work without making
scaffold-only or target-incomplete modules selectable. Add real template paths
to the module manifest only after its typed authoring contract, C API mapping,
target requirements, outputs, and manual actions have been reviewed together.

Regenerate the inventory after module registry or detail-manifest changes:

```powershell
python tools/generate_module_template_scaffolds.py
```
