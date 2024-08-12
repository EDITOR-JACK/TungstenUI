#pragma once
#include "anduril/battcheck-mode.h"

uint8_t battcheck_state(Event event, uint16_t arg) {

    // 1C: OFF
    if (event == EV_1click) {
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    // 2C: Thermal Check
    else if (event == EV_2clicks) {
        set_state(tempcheck_state, 0);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}