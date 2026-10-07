from __future__ import annotations
import re
from typing import Any
from uef_gen.chips.database import ChipDatabase, UnknownChipError
from uef_gen.chips.specs import ChipCapabilities, ChipSpec
from uef_gen.config.normalize import normalize_configuration
from uef_gen.diagnostics import Diagnostic, error, warning
from uef_gen.resolver.models import ResolvedPeripheral, ResolvedProject, ResolvedTask

class ResourceResolver:
    """Owns all hardware allocation; templates only consume this resolved result."""

    def __init__(self, database: ChipDatabase):
        self._database = database

    def resolve(self, config: dict[str, Any]) -> ResolvedProject:
        source_config = config
        config = normalize_configuration(config)
        diagnostics: list[Diagnostic] = []
        target = config.get("target", {})
        chip_name = target.get("chip", "") if isinstance(target, dict) else ""
        try:
            chip = self._database.get(chip_name)
        except UnknownChipError as exc:
            raise UnknownChipError(f"No chip specification is registered for '{chip_name}'. Add verified part data to the chip database.") from exc
        except AttributeError as exc:
            raise ValueError("target.chip is required before resource resolution") from exc

        claimed_pins: dict[str, str] = {}
        claimed_dma: dict[str, str] = {}
        claimed_irq_priorities: dict[tuple[str, int], str] = {}
        peripherals: list[ResolvedPeripheral] = []
        requested = config.get("peripherals", [])
        if isinstance(requested, dict):
            requested = [dict(value, _config_key=key, instance=value.get("instance", key)) if isinstance(value, dict) else {"instance": key, "_config_key": key}
                         for key, value in requested.items()]
        if not isinstance(requested, list):
            diagnostics.append(error("peripheral_config_invalid", "peripherals must be an array or mapping", "peripherals"))
            requested = []

        for index, request in enumerate(requested):
            location = f"peripherals[{index}]"
            if not isinstance(request, dict) or not isinstance(request.get("instance"), str):
                diagnostics.append(error("peripheral_instance_required", "Each peripheral allocation requires an instance name", location))
                continue
            instance_name = request["instance"]
            config_key = str(request.get("_config_key", request.get("key", request.get("name", instance_name))))
            instance = chip.peripherals.get(instance_name)
            if instance is None:
                diagnostics.append(error("peripheral_not_present", f"{instance_name} is not present on {chip.name}", location))
                continue
            expected_type = request.get("type")
            if expected_type and expected_type.casefold() != instance.type.type_name.casefold():
                diagnostics.append(error("peripheral_type_mismatch", f"{instance_name} is {instance.type.type_name}, not {expected_type}", location))

            resolved_pins: dict[str, str] = {}
            resolved_af: dict[str, int] = {}
            pin_config = request.get("pins", {})
            if not isinstance(pin_config, dict):
                diagnostics.append(error("pin_config_invalid", "pins must map peripheral signals to pin names", f"{location}.pins"))
                pin_config = {}
            for signal, pin in pin_config.items():
                signal = next(
                    (known for known in instance.pins if known.casefold() == str(signal).casefold()),
                    str(signal),
                )
                pin = str(pin).upper()
                choices = tuple(p.upper() for p in instance.pins.get(signal, ()))
                if not choices:
                    diagnostics.append(error("signal_unsupported", f"{instance_name} has no pin mapping for signal {signal}", f"{location}.pins.{signal}"))
                    continue
                if pin not in choices:
                    diagnostics.append(error("pin_not_available", f"{pin} is not valid for {instance_name}.{signal}; valid pins: {', '.join(choices)}", f"{location}.pins.{signal}"))
                    continue
                owner = claimed_pins.get(pin)
                if owner is not None:
                    diagnostics.append(error("pin_conflict", f"{pin} is assigned to both {owner} and {instance_name}.{signal}", f"{location}.pins.{signal}"))
                    continue
                claimed_pins[pin] = f"{instance_name}.{signal}"
                af = instance.af.get(f"{pin}|{signal}")
                if af is None:
                    diagnostics.append(error("alternate_function_missing", f"No verified alternate-function mapping for {instance_name}.{signal} on {pin}", f"{location}.pins.{signal}"))
                    continue
                resolved_pins[signal] = pin
                resolved_af[signal] = af

            resolved_dma: dict[str, str] = {}
            dma_config = request.get("dma", {})
            if not isinstance(dma_config, dict):
                diagnostics.append(error("dma_config_invalid", "dma must map transfer direction to a channel", f"{location}.dma"))
                dma_config = {}
            for direction, channel in dma_config.items():
                direction = direction.upper()
                if channel is False or channel is None:
                    continue
                if direction not in instance.type.dma_directions:
                    diagnostics.append(error("dma_direction_unsupported", f"{instance_name} does not support DMA {direction}", f"{location}.dma.{direction}"))
                    continue
                dma_request = instance.dma_requests.get(direction)
                if dma_request is None:
                    diagnostics.append(error("dma_request_missing", f"No verified DMAMUX request for {instance_name}.{direction}", f"{location}.dma.{direction}"))
                    continue
                if chip.family.dma.request_map and dma_request not in chip.family.dma.request_map:
                    diagnostics.append(error("dma_request_unavailable", f"DMA request {dma_request} is absent from the family DMAMUX table", f"{location}.dma.{direction}"))
                    continue
                if channel is True:
                    channel = _automatic_dma_channel(
                        dma_request,
                        chip.family.dma.request_map,
                        claimed_dma,
                    )
                    if channel is None:
                        diagnostics.append(error(
                            "dma_route_unresolved",
                            f"No free verified channel is recorded for {instance_name}.{direction} ({dma_request})",
                            f"{location}.dma.{direction}",
                        ))
                        continue
                channel = str(channel).upper()
                channel_match = re.fullmatch(r"DMA(\d+)_CH(?:ANNEL)?(\d+)", channel)
                if chip.family.dma.controller_count <= 0 or chip.family.dma.channels_per_controller <= 0:
                    diagnostics.append(error("dma_topology_unresolved", f"No verified DMA controller/channel topology is recorded for {chip.name}", f"{location}.dma.{direction}"))
                    continue
                if channel_match:
                    controller, channel_number = map(int, channel_match.groups())
                    if controller < 1 or controller > chip.family.dma.controller_count or channel_number < 1 or channel_number > chip.family.dma.channels_per_controller:
                        diagnostics.append(error("dma_channel_unavailable", f"DMA channel {channel} is not present on {chip.name}", f"{location}.dma.{direction}"))
                        continue
                elif chip.family.dma.controller_count > 0:
                    diagnostics.append(error("dma_channel_format", f"DMA channel '{channel}' must use DMA<n>_CH<n> naming for this target", f"{location}.dma.{direction}"))
                    continue
                owner = claimed_dma.get(channel)
                if owner is not None:
                    diagnostics.append(error("dma_channel_conflict", f"DMA channel {channel} is assigned to both {owner} and {instance_name}.{direction}", f"{location}.dma.{direction}"))
                else:
                    claimed_dma[channel] = f"{instance_name}.{direction}"
                    resolved_dma[direction] = channel

            irq_priority = request.get("irq_priority")
            if irq_priority is not None:
                for irq in instance.irqs:
                    key = (irq, int(irq_priority))
                    previous = claimed_irq_priorities.get(key)
                    if previous:
                        diagnostics.append(warning("irq_priority_duplicate", f"{irq} priority {irq_priority} is shared by {previous} and {instance_name}", location))
                    else:
                        claimed_irq_priorities[key] = instance_name
            peripherals.append(ResolvedPeripheral(instance_name, instance.type.type_name, resolved_pins,
                                                  resolved_af, resolved_dma, instance.irqs, config_key))
            peripheral_config = request.get("config", {})
            if isinstance(peripheral_config, dict):
                requested_clock = peripheral_config.get("baud", peripheral_config.get("frequency_hz"))
                if requested_clock is not None:
                    domain = chip.family.clock_domains.get(instance.clock_domain)
                    rate = int(requested_clock)
                    tolerance = int(peripheral_config.get("tolerance_ppm", 0))
                    if domain is None or rate <= 0:
                        diagnostics.append(error("peripheral_clock_unresolved", f"No verified clock model for {instance_name} clock domain {instance.clock_domain}", f"{location}.config"))
                    else:
                        source_hz = int(domain.get("source_hz", domain.get("max_hz", 0)))
                        divider_min = max(1, int(domain.get("divider_min", 1)))
                        divider_max = max(divider_min, int(domain.get("divider_max", divider_min)))
                        best_error = min((abs(source_hz // divider - rate) for divider in range(divider_min, divider_max + 1)), default=abs(source_hz-rate))
                        if rate > int(domain.get("max_hz", 0)) or best_error > rate * tolerance / 1_000_000:
                            diagnostics.append(error("peripheral_clock_unachievable", f"{instance_name} cannot generate {rate} Hz from {instance.clock_domain} within {tolerance} ppm", f"{location}.config"))

        clock = config.get("clock", {})
        if isinstance(clock, dict):
            for constraint in clock.get("constraints", []):
                if not isinstance(constraint, dict):
                    continue
                domain = str(constraint.get("domain", ""))
                requested_hz = int(constraint.get("frequency_hz", 0))
                domain_spec = chip.family.clock_domains.get(domain)
                if requested_hz <= 0 or domain_spec is None:
                    diagnostics.append(error("clock_constraint_unresolved", f"Clock domain '{domain}' has no verified frequency model", "clock.constraints"))
                    continue
                max_hz = int(domain_spec.get("max_hz", 0))
                source_hz = int(domain_spec.get("source_hz", max_hz))
                divider_min = max(1, int(domain_spec.get("divider_min", 1)))
                divider_max = max(divider_min, int(domain_spec.get("divider_max", divider_min)))
                best = min((abs(source_hz // d - requested_hz) for d in range(divider_min, divider_max + 1)), default=abs(max_hz-requested_hz))
                tolerance = int(constraint.get("tolerance_ppm", 0))
                allowed = requested_hz * tolerance / 1_000_000
                if requested_hz > max_hz or best > allowed:
                    diagnostics.append(error("clock_constraint_unmet", f"{domain} cannot provide {requested_hz} Hz within {tolerance} ppm", "clock.constraints"))

        memory = config.get("memory", {})
        if isinstance(memory, dict):
            flash = int(memory.get("flash_bytes", 0))
            ram = int(memory.get("sram_bytes", 0))
            if flash > chip.flash_bytes:
                diagnostics.append(error("flash_capacity_exceeded", f"Requested {flash} bytes exceeds {chip.name} flash ({chip.flash_bytes})", "memory.flash_bytes"))
            if ram > chip.sram_bytes:
                diagnostics.append(error("sram_capacity_exceeded", f"Requested {ram} bytes exceeds {chip.name} SRAM ({chip.sram_bytes})", "memory.sram_bytes"))
            limits = chip.overrides.get("memory_regions", {})
            for region_name, region_size in memory.get("regions", {}).items() if isinstance(memory.get("regions", {}), dict) else []:
                if region_name not in limits:
                    diagnostics.append(error("memory_region_unresolved", f"No verified size is recorded for memory region {region_name}", f"memory.regions.{region_name}"))
                elif int(region_size) > int(limits[region_name]):
                    diagnostics.append(error("memory_region_exceeded", f"Requested {region_size} bytes exceeds {region_name} capacity ({limits[region_name]})", f"memory.regions.{region_name}"))
            freertos_settings = target.get("freertos", {}) if isinstance(target, dict) else {}
            kernel_heap = int(freertos_settings.get("heap_bytes", 0)) if isinstance(freertos_settings, dict) else 0
            declared_heap = int(memory.get("heap_bytes", kernel_heap))
            rtos_value = config.get("rtos", "")
            rtos_name = str(rtos_value.get("name", "") if isinstance(rtos_value, dict) else rtos_value)
            if rtos_name.casefold() == "freertos" and "heap_bytes" in memory and declared_heap != kernel_heap:
                diagnostics.append(error(
                    "freertos_heap_mismatch",
                    "memory.heap_bytes and target.freertos.heap_bytes must match",
                    "memory.heap_bytes",
                ))
            required_ram = (
                int(memory.get("main_stack_bytes", 0))
                + declared_heap
                + sum(
                    int(task.get("stack_bytes", 0))
                    for task in config.get("tasks", [])
                    if isinstance(task, dict)
                    and str(task.get("context", "task")).casefold() == "task"
                )
            )
            if chip.sram_bytes > 0 and required_ram > chip.sram_bytes:
                diagnostics.append(error(
                    "sram_budget_exceeded",
                    f"Configured stacks and heap require {required_ram} bytes, exceeding {chip.name} SRAM ({chip.sram_bytes})",
                    "memory",
                ))

        rtos_config = config.get("rtos", {})
        if isinstance(rtos_config, dict):
            tick_us = int(rtos_config.get("tick_us", 1))
            rtos_name = str(rtos_config.get("name", target.get("rtos", "baremetal")))
        else:
            tick_us = 1
            rtos_name = str(rtos_config or target.get("rtos", "baremetal"))
        tasks: list[ResolvedTask] = []
        for index, task in enumerate(config.get("tasks", [])):
            if not isinstance(task, dict):
                diagnostics.append(error("task_config_invalid", "Each task must be a mapping", f"tasks[{index}]"))
                continue
            name = str(task.get("name", f"task_{index}"))
            period = int(task.get("period_us", 0))
            context = str(task.get("context", task.get("execution_context", "task"))).casefold()
            if context not in {"task", "isr"}:
                diagnostics.append(error("task_context_invalid", f"Task {name} context must be 'task' or 'isr'", f"tasks[{index}].context"))
            if period <= 0:
                diagnostics.append(error("task_period_invalid", f"Task {name} requires a positive period_us", f"tasks[{index}].period_us"))
            elif context == "task" and period < tick_us:
                diagnostics.append(error("task_period_below_tick", f"Task {name} period {period} us is below scheduler tick {tick_us} us", f"tasks[{index}].period_us"))
            tasks.append(ResolvedTask(name, period, int(task.get("priority", 0)),
                                      int(task.get("stack_bytes", 0)), context))

        required_capabilities = config.get("required_capabilities", [])
        caps = ChipCapabilities(chip)
        for capability in required_capabilities:
            if not caps.supports(str(capability)):
                diagnostics.append(error("capability_unavailable", f"Required target capability '{capability}' is unavailable on {chip.name}", "required_capabilities"))
        if not chip.verified:
            diagnostics.append(warning("chip_data_unverified", f"Chip database entry {chip.name} is marked unverified; verify package, pin, DMA, IRQ, and clock data before hardware use", "target.chip"))

        arithmetic = str(target.get("arithmetic", "float32"))
        rtos = rtos_name
        configured_modules = config.get("modules", ["uhal", "umid", "uos", "upal", "uproto"])
        modules = tuple(dict.fromkeys(["uapp", *(str(value).casefold() for value in configured_modules)]))
        defines = tuple(str(value) for value in config.get("defines", []))
        flags = tuple([*chip.family.arch.compiler_flags, *(str(value) for value in config.get("compiler_flags", []))])
        return ResolvedProject(str(config.get("project", {}).get("name", chip.name)), chip, caps,
                               arithmetic, rtos, modules, tuple(peripherals), tuple(tasks), defines,
                               flags, tuple(diagnostics), source_config)


def _automatic_dma_channel(request: str, request_map: dict[str, Any],
                           claimed: dict[str, str]) -> str | None:
    """Choose the first unclaimed channel explicitly assigned to a DMA request."""
    routes = request_map.get(request)
    if isinstance(routes, str):
        candidates = [routes]
    elif isinstance(routes, (list, tuple)):
        candidates = [str(route) for route in routes]
    elif isinstance(routes, dict):
        raw = routes.get("channels", routes.get("channel", []))
        candidates = [raw] if isinstance(raw, str) else [str(route) for route in raw]
    else:
        candidates = []
    return next((candidate for candidate in candidates if candidate.upper() not in claimed), None)
