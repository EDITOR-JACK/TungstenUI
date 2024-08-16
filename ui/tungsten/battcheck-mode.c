#pragma once
#include "anduril/battcheck-mode.h"

uint8_t battcheck_state(Event event, uint16_t arg) {

    // 1C: OFF
    if (event == EV_1click) {
        set_level(0);
        rgb_led_update(0x21,0);
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}