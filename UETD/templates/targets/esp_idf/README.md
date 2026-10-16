# ESP-IDF templates

`CMakeLists.txt.tmpl` creates an ESP-IDF project and requires a valid
`project_name`.

`main/CMakeLists.txt.tmpl` registers `main.c` and additional `sources`, with
`include_dirs` and component `requirements`.
`main/main.c.tmpl` exposes the supplied `project_entrypoint` as `app_main()`.

`components/uetd/CMakeLists.txt.tmpl` registers the selected UETD `sources`,
`include_dirs`, and component `requirements`. Paths are relative to the
component directory and must be present in the generated project.

ESP-IDF owns reset, startup, linker fragments, partitions, and image creation;
these templates leave those responsibilities with the framework.
