/// @file examples/adc_hrtim_triggered/main.c
/// @brief Hardware example scaffold: adc_hrtim_triggered.
///
/// TODO: Configure an HRTIM-triggered ADC/DMA sample path. Record trigger phase, channel order,
/// sample time, timer frequency, cache rules, and shutdown-on-fault behavior.
#include "uef/upal/upal_adc.h"
#include "uef/upal/upal_hrtim.h"

int main(void) {
    /* TODO(adc-hrtim-triggered main): Initialize the selected board and verify its HRTIM
     * clock, trigger phase, ADC channel order, and DMA-safe static sample buffer. Configure
     * the trigger/ADC/DMA chain with outputs disabled, start acquisition, and publish only
     * completed buffers from the documented ISR/task context. Report overruns and stop the
     * chain safely on setup or transfer failure; keep target-specific timing in board data.
     */
    return 0;
}
