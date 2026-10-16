# Module build routes

Each `route.tmpl` is a Jinja template that renders one CMake target for the
module listed in its detail manifest. The rendered target compiles the
manifest's implementation sources, publishes its public headers and UETD
include root, and links the module's declared dependencies.

Set `UETD_ROOT` to the UETD checkout or copied source directory before
including a route. Include dependency routes before the route that uses them.
The target names are `uetd_module_<module_id_with_underscores>` and the alias
names are `UETD::Module_<module_id_with_underscores>`.

An enclosing target profile can provide `UETD_MODULE_INCLUDE_DIRS`,
`UETD_MODULE_COMPILE_DEFINITIONS`, `UETD_MODULE_COMPILE_OPTIONS`, and
`UETD_MODULE_LINK_LIBRARIES` before including routes. These values let the
profile supply device headers, ABI selections, compiler requirements, and
external libraries without embedding them in portable module templates.

Render a route with the UETD template renderer and an empty value map; the
route's manifest-derived values are part of the template. For example:

```python
from pathlib import Path
from uef_gen.templates import render_uetd_template

route = render_uetd_template(
    Path("UETD/templates"),
    "modules/ucon/control/classical/pid/route.tmpl",
    {},
)
```

`index.json` maps module IDs to template and manifest paths. `catalog.md` is a
human-readable view of the same module list. Regenerate both indexes and the
routes after module registry or detail-manifest changes:

```powershell
python tools/generate_module_templates.py
```

These CMake routes package declared UETD sources for a build. They do not
create algorithm instances or change `authoring.generation.template_paths`,
module availability, or target readiness. UETD project generation continues
to enforce the availability and scaffold status recorded in each manifest.
