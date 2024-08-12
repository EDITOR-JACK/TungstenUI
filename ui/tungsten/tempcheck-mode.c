#pragma once
#include "anduril/tempcheck-mode.h"

uint8_t tempcheck_state(Event event, uint16_t arg) {
    // 1C: OFF
    if (event == EV_1click) {
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    // 7H: thermal config mode
    else if (event == EV_click7_hold) {
        push_state(thermal_config_state, 0);
        return EVENT_HANDLED;
    }
    return EVENT_NOT_HANDLED;
}

void thermal_config_save(uint8_t step, uint8_t value) {
    if (value) {
        // item 1: calibrate room temperature
        if (step == 1) {
            int8_t rawtemp = temperature - cfg.therm_cal_offset;
            cfg.therm_cal_offset = value - rawtemp;
            adc_reset = 2;  // invalidate all recent temperature data
        }
    }
}

uint8_t thermal_config_state(Event event, uint16_t arg) {
    return config_state_base(event, arg,
                             1, thermal_config_save);
}