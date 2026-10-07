/// @file include/uef/uapp/uapp_fault.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UAPP_FAULT_H
#define UAPP_FAULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/uhal/uhal_fault.h"

/* Fault codes — application-specific upper bits; UEF uses lower 16 bits */
#define UAPP_FAULT_STACK_OVERFLOW     0x0001u
#define UAPP_FAULT_SENSOR_TIMEOUT     0x0002u
#define UAPP_FAULT_ACTUATOR_FAULT     0x0003u
#define UAPP_FAULT_COMM_LOSS          0x0004u
#define UAPP_FAULT_OVER_TEMPERATURE   0x0005u
#define UAPP_FAULT_OVER_CURRENT       0x0006u
#define UAPP_FAULT_UNDER_VOLTAGE      0x0007u
#define UAPP_FAULT_OVER_VOLTAGE       0x0008u
/* 0x1000 and above: application-defined */

/* Log a recoverable fault and trigger recovery action */
void uapp_fault_report(uef_u32_t fault_code, const char* detail);

/* Trigger an unrecoverable fault — saves to RTC backup, asserts IWDG */
UEF_NORETURN void uapp_fault_fatal(uef_u32_t fault_code, const char* detail);

/* Read fault log from last boot (via RTC backup registers) */
bool     uapp_fault_log_available(void);
uef_u32_t uapp_fault_log_code(void);
uef_u32_t uapp_fault_log_pc(void);
uef_u32_t uapp_fault_log_lr(void);

#ifdef __cplusplus
}
#endif

#endif /* UAPP_FAULT_H */
