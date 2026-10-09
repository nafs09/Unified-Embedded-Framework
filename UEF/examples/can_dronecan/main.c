/// @file examples/can_dronecan/main.c
/// @brief Hardware example scaffold: can_dronecan.
///
/// TODO: Configure CAN filters and the selected DroneCAN/UAVCAN library binding. Record node
/// identity, transfer-ID policy, memory limits, and bus-off recovery.
#include "uef/uproto/uproto_uavcan.h"

int main(void) {
    /* TODO(can-dronecan main): Initialize the selected CAN controller with verified
     * bitrate/timing and filters, then bind a statically sized node/transport instance
     * from the selected DroneCAN/UAVCAN stack. Run bounded receive/spin work in task
     * context, publish node health, and enter a defined stopped/degraded state on bus-off
     * or stack errors; document node ID and buffer ownership in board configuration.
     */
    return 0;
}
