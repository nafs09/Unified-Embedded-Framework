# FreeRTOS task scaffold

This source scaffold outlines creation of UOS tasks and synchronization
objects with statically allocated stacks, control blocks, queues, and buffers.
Scheduler startup follows successful object creation.

The FreeRTOS port, board startup, task priorities, stack sizes, synchronization
policy, and bounded startup-failure path remain to be supplied by a specific
target configuration. The example is not built by default and is not ready to
run on hardware.
