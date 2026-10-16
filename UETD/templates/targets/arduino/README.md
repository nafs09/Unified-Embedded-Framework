# Arduino template

`sketch.ino.tmpl` adapts generated C entry points to the selected Arduino
core's `setup()` and `loop()` callbacks. Render it with
`initialize_symbol` and `loop_symbol`; both values are C function names.

The Arduino core supplies reset, startup, and linking. The loop callback must
match the scheduling model selected for the project.
