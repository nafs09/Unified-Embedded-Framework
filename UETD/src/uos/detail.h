/* Internal to the selected UOS backend; not an application include. */
#ifndef UOS_DETAIL_H
#define UOS_DETAIL_H
#include "uos/uos.h"
uos_status_t uos_clock_delay(uos_dur_t duration);
uos_status_t uos_backend_task_ready(void);
#endif
