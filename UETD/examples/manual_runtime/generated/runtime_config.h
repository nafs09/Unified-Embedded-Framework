#ifndef MANUAL_RUNTIME_CONFIG_H
#define MANUAL_RUNTIME_CONFIG_H
/* Define these consistently for the UETD library AND every consumer.
 * The accompanying CMakeLists does this. Other builds must use the same -D values. */
#include "uapp/runtime/statemachine.h"
#if UAPP_SM_MAX_STATES < 2 || UAPP_SM_MAX_TRANSITIONS < 2
#error "UAPP state/transition capacity is too small for the generated tables"
#endif
#if UAPP_SM_EVENT_QUEUE_CAPACITY != 8 || UAPP_SM_EVENT_PAYLOAD_CAPACITY != 16
#error "Apply the generated queue/payload ABI definitions to all translation units"
#endif
#endif
