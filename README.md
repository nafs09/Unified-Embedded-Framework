# Embedded projects

Standalone embedded projects used by NEXUS are kept beside, not inside, the
NEXUS host source tree. The NEXUS-side integration lives in `../Code` and
communicates with `uef-gen` through ControlIR JSON and a process boundary.

- `UEF/`: C-first reusable embedded framework and runtime
- `uef-gen/`: standalone Python embedded project construction tool

The folders are independent projects; they do not import NEXUS modules.
