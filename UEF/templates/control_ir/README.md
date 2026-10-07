# ControlIR template dispatch

`../../registry/control_ir/registry.json` is the UEF-owned dispatch index for NEXUS
ControlIR nodes. It maps each supported subtype/type key to an individual
manifest under `registry/control_ir/manifest/`. Those detail files declare output
templates, project paths, required UEF modules, and manual actions. UCON and
UPROTO share this registry; there is no separate algorithm or protocol
catalogue in `uef-gen`.

`uef-gen` passes the raw ControlIR document, raw node, resolved target/project
context, module list, UEF API metadata, and stable node path to the selected
template. It does not translate graph data into another algorithm model,
synthesize missing fields, or infer hardware actions. Every registered node
must provide a unique C-safe `model.symbol_prefix` from its ControlIR producer.

An entry's `manual_actions` array is the source of truth for project work such
as a board driver binding, a protocol dialect/stack choice, ISR/task wiring, or
hardware validation. `uef-gen` returns each action as a path-qualified warning
and preserves the action in `manifest/manual_actions.json` for other consumers.

Current registered entries include the initial UPROTO wrappers and the PID and
lead-lag UCON wrappers. The broader UCON catalogue contains fail-closed module
outlines only: each unregistered algorithm has named operation functions and a
focused TODO per function, but no output template and no selectable generation
contract. Do not add a dispatch entry until the typed public contract,
numerical behavior, generic C implementation, metadata, and any required
instance templates have been reviewed together.
