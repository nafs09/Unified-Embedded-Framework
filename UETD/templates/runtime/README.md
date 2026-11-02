# Manual runtime templates

These Jinja2 templates generate C11 state-machine tables, typed callback
declarations, shared ABI configuration, a CMake project and optional static
FreeRTOS tasks/configuration from explicit JSON values. They are independent
of the UEF-PL compiler and its unfinished firmware-emission path.

Use [`tools/render_manual_runtime.py`](../../tools/render_manual_runtime.py)
to validate inputs before rendering. The
[worked example](../../examples/manual_runtime/README.md) includes a checked-in
render and application callbacks. The
[first-pass guide](../../docs/UOS_UAPP_FIRST_PASS.md) explains providers,
ownership, migration and the remaining dependencies.

Service source/header selection routes live separately under `templates/modules`.
Provider callbacks for diagnostics, retained history, product communication and
secure update remain application code; templates do not invent their semantics.
