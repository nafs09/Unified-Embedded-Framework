#include "uos_uapp_test.h"
#include "tasks.h"
#include "callbacks.h"
#include <setjmp.h>
static jmp_buf stop;
static uos_task_fn_t entry;
static void* task_context;
static unsigned creates, steps, delays;
static bool fail_create, fail_operation;
uos_status_t uos_task_create_static(uos_task_t* task, const char* name, uos_task_fn_t fn, void* ctx,
                                    uetd_u32_t priority, void* stack, uetd_u32_t bytes) {
    CHECK(task != NULL && !strcmp(name, "owner") && priority == 2 && bytes == 1024);
    CHECK((uintptr_t)stack % UOS_STORAGE_ALIGN == 0); ++creates;
    if (fail_create) { return UOS_BACKEND_ERROR; }
    entry = fn; task_context = ctx; return UOS_OK;
}
uos_status_t uos_time_now(uos_time_t* now) { *now = 1000; return UOS_OK; }
uos_status_t uos_task_delay_until(uos_time_t* next, uos_dur_t period) {
    CHECK(period == 1000 && *next == 2000 + delays * 1000); ++delays; *next += period;
    if (delays == 2) { longjmp(stop, 1); }
    return UOS_OK;
}
uetd_status_t owner_step(void* ctx) { CHECK(ctx == &steps); ++steps; return fail_operation ? UETD_ERROR : UETD_OK; }
UETD_NORETURN void manual_runtime_os_failure(uos_status_t status) { (void)status; CHECK(false); exit(1); }
UETD_NORETURN void manual_runtime_component_failure(uetd_status_t status) { CHECK(status == UETD_ERROR); longjmp(stop, 2); }
int main(int argc, char** argv) {
    (void)argv; fail_create = argc > 1; void* contexts[] = {&steps};
    const uos_status_t expected = fail_create ? UOS_BACKEND_ERROR : UOS_OK;
    CHECK(manual_tasks_create(contexts) == expected);
    CHECK(manual_tasks_create(contexts) == expected && creates == 1);
    if (fail_create) { return 0; }
    if (setjmp(stop) == 0) { entry(task_context); CHECK(false); }
    CHECK(steps == 2 && delays == 2); fail_operation = true;
    if (setjmp(stop) == 0) { entry(task_context); CHECK(false); }
    CHECK(steps == 3 && delays == 2);
    puts("Generated task allocation, cadence, startup failure and callback failure passed"); return 0;
}
