# Manual UOS/UAPP template example

This uses explicit JSON values, independently of the real generation pipeline.
The checked-in [generated](generated/) directory is a ready-to-inspect output
of [runtime.json](runtime.json). [host_main.c](host_main.c) supplies its
application callbacks and exercises start/stop behavior without hardware.

From the UETD root, with Python and Jinja2 installed:

```powershell
python tools/render_manual_runtime.py examples/manual_runtime/runtime.json examples/manual_runtime/generated
cmake -S examples/manual_runtime/generated -B ../build/manual-runtime -DUETD_ROOT="$PWD" -DMANUAL_APPLICATION_SOURCE="$PWD/examples/manual_runtime/host_main.c"
cmake --build ../build/manual-runtime --config Debug
../build/manual-runtime/Debug/manual_application.exe
```

For your project, copy runtime.json and supply your own state/event/callback
names. Render into a fresh directory in the separate project. Generated C
contains tables and wrappers; callbacks belong to your application files.
The renderer refuses to replace edited generated output. It updates unchanged
managed output on repeat runs. Templates can also be filled manually.

For FreeRTOS, select `"backend": "FREERTOS"` and add tasks:

```json
"tasks": [{
  "name": "owner",
  "operation": "owner_step",
  "priority": 2,
  "stack_bytes": 1024,
  "period_us": 1000
}]
```

The output adds tasks.h/c and FreeRTOSConfig.h. Supply uos_port_config.h and the
configured kernel port. Bind UOS time before task creation; create all tasks and
check success before starting the scheduler. The operation returns UETD status;
the generated task routes failures to application-owned nonreturning handlers.
Its first activation is immediate and subsequent releases preserve cadence.
A partial task-creation failure prevents scheduler startup; no unsafe retry is
attempted. Stack sizes and interrupt priorities require target validation.

State names are zero-based in declaration order. Events have explicit u8 IDs.
`"from": "any"` expands in source order, including self transitions. A self
transition exits and re-enters. Unconditional rows cannot hide later rows.
Callbacks receive the application context passed to the machine init wrapper.
Only the owner calls init/post/process/reset, and each process consumes at most
one event. Overflow and callback errors must be handled.

Read [the first-pass guide](../../docs/UOS_UAPP_FIRST_PASS.md) before
connecting the example to physical outputs.
