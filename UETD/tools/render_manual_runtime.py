#!/usr/bin/env python3
"""Render UOS/UAPP C templates from explicit JSON, independently of uef-gen.

No C expressions are evaluated or accepted as configuration. Callbacks are
external identifiers. Output paths are fixed or derived from validated names.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import re
from pathlib import Path
from jinja2 import Environment, FileSystemLoader, StrictUndefined

ROOT = Path(__file__).resolve().parents[1]
KEYWORDS = set("auto break case char const continue default do double else enum extern float for goto if inline int long register restrict return short signed sizeof static struct switch typedef union unsigned void volatile while _Alignas _Alignof _Atomic _Bool _Complex _Generic _Imaginary _Noreturn _Static_assert _Thread_local".split())
RESERVED_FILES = {"callbacks", "runtime_config", "tasks", "freertosconfig", "cmakelists", "con", "prn", "aux", "nul"} | {f"{prefix}{n}" for prefix in ("com", "lpt") for n in range(1, 10)}


def keys(value: dict, allowed: str, label: str) -> None:
    unexpected = set(value) - set(allowed.split())
    if unexpected:
        raise ValueError(f"unknown {label} fields: {sorted(unexpected)}")


def unique_object(pairs: list[tuple[str, object]]) -> dict:
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON field: {key}")
        result[key] = value
    return result

def identifier(value: object) -> str:
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z][A-Za-z0-9_]*", value) or value in KEYWORDS:
        raise ValueError(f"invalid C identifier: {value!r}")
    return value

def integer(value: object, lower: int, upper: int, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not lower <= value <= upper:
        raise ValueError(f"{label} must be an integer in [{lower}, {upper}]")
    return value

def prepare(document: dict) -> dict:
    if not isinstance(document, dict) or type(document.get("schema_version")) is not int or document["schema_version"] != 1:
        raise ValueError("expected schema_version 1 object")
    keys(document, "schema_version backend machines tasks freertos event_queue_capacity event_payload_capacity", "configuration")
    backend = document.get("backend", "BAREMETAL")
    if backend not in {"BAREMETAL", "FREERTOS"}:
        raise ValueError("backend must be BAREMETAL or FREERTOS")
    machines = document.get("machines", [])
    tasks = document.get("tasks", [])
    if not isinstance(machines, list) or not 1 <= len(machines) <= 128 or not isinstance(tasks, list) or len(tasks) > 64:
        raise ValueError("provide 1..128 machines and at most 64 tasks")
    if backend == "BAREMETAL" and tasks:
        raise ValueError("bare-metal has a caller-owned loop, not RTOS tasks")
    names: set[str] = set()
    callbacks: dict[str, str] = {}
    def callback(value: object, signature: str) -> str | None:
        if value is None:
            return None
        name = identifier(value)
        if name in {"NULL", "true", "false", "hooks", "transitions", "context", "next", "status", "os_status"} or name.startswith(("manual_generated_", "uapp_", "uos_", "uetd_", "UAPP_", "UOS_", "UETD_")):
            raise ValueError(f"callback uses a reserved runtime symbol: {name}")
        if name in callbacks and callbacks[name] != signature:
            raise ValueError(f"callback {name} has conflicting signatures")
        callbacks[name] = signature
        return name
    result = []
    for raw in machines:
        if not isinstance(raw, dict):
            raise ValueError("machine must be an object")
        keys(raw, "name states initial events transitions on_fault on_unhandled", "machine")
        name = identifier(raw.get("name"))
        if name.lower() in RESERVED_FILES:
            raise ValueError(f"machine name collides with a reserved output: {name}")
        if name in names or name.upper() in {n.upper() for n in names}:
            raise ValueError(f"duplicate machine name {name}")
        names.add(name)
        states = raw.get("states")
        if not isinstance(states, list) or not 1 <= len(states) <= 255:
            raise ValueError("machine requires 1..255 states")
        parsed_states, state_names = [], set()
        for state in states:
            if not isinstance(state, dict):
                raise ValueError("state must be an object")
            keys(state, "name enter exit", "state")
            state_name = identifier(state.get("name"))
            if state_name.upper() in state_names:
                raise ValueError(f"duplicate state {state_name}")
            state_names.add(state_name.upper())
            parsed_states.append(dict(name=state_name, enter=callback(state.get("enter"), "hook"), exit=callback(state.get("exit"), "hook")))
        state_ids = {state["name"]: i for i, state in enumerate(parsed_states)}
        if raw.get("initial") not in state_ids:
            raise ValueError("initial state must name a declared state")
        events = raw.get("events", {})
        if not isinstance(events, dict) or len(events) > 256:
            raise ValueError("events must be a name-to-ID object")
        event_ids, event_names = set(), set()
        for event, value in events.items():
            identifier(event); integer(value, 0, 255, "event ID")
            if value in event_ids or event.upper() in event_names:
                raise ValueError("duplicate event ID/name")
            event_ids.add(value); event_names.add(event.upper())
        rows, unconditional = [], set()
        transitions = raw.get("transitions", [])
        if not isinstance(transitions, list) or len(transitions) > 255:
            raise ValueError("at most 255 transition declarations are supported")
        for transition in transitions:
            if not isinstance(transition, dict):
                raise ValueError("transition must be an object")
            keys(transition, "from to event action guard", "transition")
            source, destination, event = transition.get("from"), transition.get("to"), transition.get("event")
            if destination not in state_ids or event not in events or (source != "any" and source not in state_ids):
                raise ValueError("transition refers to unknown state/event")
            action = callback(transition.get("action"), "action")
            guard = callback(transition.get("guard"), "guard")
            for source_id in state_ids.values() if source == "any" else [state_ids[source]]:
                key = (source_id, events[event])
                if key in unconditional:
                    raise ValueError("transition is unreachable after an unconditional row")
                if guard is None:
                    unconditional.add(key)
                rows.append(dict(source=source_id, destination=state_ids[destination], event=events[event], action=action, guard=guard))
        if len(rows) > 255:
            raise ValueError("expanded transition count exceeds 255")
        result.append(dict(name=name, macro=name.upper(), states=parsed_states, events=events, rows=rows,
                           initial=state_ids[raw["initial"]], fault=callback(raw.get("on_fault"), "fault"),
                           unhandled=callback(raw.get("on_unhandled"), "unhandled")))
    parsed_tasks, task_names = [], set()
    for raw in tasks:
        if not isinstance(raw, dict):
            raise ValueError("task must be an object")
        keys(raw, "name operation priority stack_bytes period_us", "task")
        name = identifier(raw.get("name"))
        if name in task_names:
            raise ValueError("duplicate task name")
        task_names.add(name)
        parsed_tasks.append(dict(name=name, operation=callback(raw.get("operation"), "operation"),
            priority=integer(raw.get("priority"), 0, 254, "priority"),
            stack_bytes=integer(raw.get("stack_bytes"), 64, 0x1000000, "stack_bytes"),
            period_us=integer(raw.get("period_us"), 1, 0xFFFFFFFFFFFFFFFE, "period_us")))
        if parsed_tasks[-1]["operation"] is None:
            raise ValueError("task operation is required")
    generated = {f"{m['name']}_{suffix}" for m in result for suffix in ("init", "post", "process", "reset", "state", "t")}
    generated |= {"manual_tasks_create", "manual_runtime_os_failure", "manual_runtime_component_failure"}
    generated |= {f"{t['name']}_{suffix}" for t in parsed_tasks for suffix in ("task", "stack", "entry")}
    for machine in result:
        generated |= {f"{machine['macro']}_STATE_{s['name'].upper()}" for s in machine["states"]}
        generated |= {f"{machine['macro']}_EVENT_{e.upper()}" for e in machine["events"]}
        generated.add(f"{machine['macro']}_INITIALIZER")
    if generated.intersection(callbacks):
        raise ValueError("callback collides with a generated symbol")
    freertos = document.get("freertos", {})
    if not isinstance(freertos, dict):
        raise ValueError("freertos settings must be an object")
    keys(freertos, "tick_hz timer_priority minimum_stack_words timer_stack_words timer_queue_length", "FreeRTOS")
    timer_priority = integer(freertos.get("timer_priority", 1), 0, 254, "timer_priority")
    return dict(backend=backend, machines=result, tasks=parsed_tasks, callbacks=callbacks,
                tick_hz=integer(freertos.get("tick_hz", 1000), 1, 1000000, "tick_hz"),
                minimum_stack=integer(freertos.get("minimum_stack_words", 256), 1, 65535, "minimum_stack_words"),
                timer_stack=integer(freertos.get("timer_stack_words", 512), 1, 65535, "timer_stack_words"),
                timer_queue=integer(freertos.get("timer_queue_length", 8), 1, 65535, "timer_queue_length"),
                timer_priority=timer_priority,
                max_priorities=max(2, timer_priority + 1, max((t["priority"] + 1 for t in parsed_tasks), default=1)),
                state_capacity=max(len(m["states"]) for m in result),
                transition_capacity=max(len(m["rows"]) for m in result),
                queue_capacity=integer(document.get("event_queue_capacity", 8), 1, 255, "event_queue_capacity"),
                payload_capacity=integer(document.get("event_payload_capacity", 16), 1, 255, "event_payload_capacity"))

def render(document: dict) -> dict[str, str]:
    values = prepare(document)
    environment = Environment(loader=FileSystemLoader(ROOT / "templates/runtime"), undefined=StrictUndefined,
                              keep_trailing_newline=True, autoescape=False)
    outputs = {}
    for machine in values["machines"]:
        for extension in ("h", "c"):
            outputs[f"{machine['name']}.{extension}"] = environment.get_template(f"statemachine.{extension}.tmpl").render(**values, machine=machine)
    for name in ("callbacks.h", "runtime_config.h", "CMakeLists.txt"):
        outputs[name] = environment.get_template(name + ".tmpl").render(**values)
    if values["tasks"]:
        for name in ("tasks.h", "tasks.c"):
            outputs[name] = environment.get_template(name + ".tmpl").render(**values)
    if values["backend"] == "FREERTOS":
        outputs["FreeRTOSConfig.h"] = environment.get_template("FreeRTOSConfig.h.tmpl").render(**values)
    return outputs

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("configuration", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    outputs = render(json.loads(args.configuration.read_text(encoding="utf-8-sig"), object_pairs_hook=unique_object))
    # No replacement of user-edited outputs. The manifest permits repeat rendering
    # only while every old output still matches the renderer's previous bytes.
    folder = args.output.resolve()
    manifest_path = folder / "manual-runtime.json"
    if manifest_path.is_symlink():
        raise ValueError("output manifest must not be a symbolic link")
    previous = json.loads(manifest_path.read_text()) if manifest_path.exists() else {}
    for name in set(previous) | set(outputs):
        path = folder / name
        if path.parent != folder or path.is_symlink():
            raise ValueError("output manifest paths must be plain files in the output directory")
        if path.exists() and hashlib.sha256(path.read_bytes()).hexdigest() != previous.get(name):
            raise ValueError(f"refusing to replace edited output {path}; render into a fresh directory")
    folder.mkdir(parents=True, exist_ok=True)
    for name in set(previous) - set(outputs):
        (folder / name).unlink(missing_ok=True)
    hashes = {}
    for name, text in outputs.items():
        data = text.encode("utf-8"); (folder / name).write_bytes(data); hashes[name] = hashlib.sha256(data).hexdigest()
    manifest_path.write_text(json.dumps(hashes, indent=2) + "\n", encoding="utf-8")
    print(f"Rendered {len(outputs)} files into {folder}")

if __name__ == "__main__":
    main()
