#include "uapp/uapp.h"
#include "uos/uos.h"
#include "uos/deferred.h"
uos_task_t task = UOS_OBJECT_INITIALIZER;
uos_queue_t queue = UOS_OBJECT_INITIALIZER;
uapp_statemachine_t machine = UAPP_SM_INITIALIZER;
uapp_product_session_t session = UAPP_PRODUCT_SESSION_INITIALIZER;
uapp_diagnostics_t diagnostics = UAPP_DIAGNOSTICS_INITIALIZER;
uapp_update_session_t update = UAPP_UPDATE_SESSION_INITIALIZER;
uos_deferred_t deferred = UOS_DEFERRED_INITIALIZER;
