#ifndef MANUAL_RUNTIME_TASKS_H
#define MANUAL_RUNTIME_TASKS_H
#include "uos/uos.h"
#define MANUAL_RUNTIME_TASK_COUNT 1u
/* Startup only, before the scheduler. Each context must outlive its task.
 * A partial creation failure must prevent scheduler startup. No retry/delete is
 * attempted because already-created kernel objects cannot safely be recreated. */
uos_status_t manual_tasks_create(void* const contexts[MANUAL_RUNTIME_TASK_COUNT]);
#endif
