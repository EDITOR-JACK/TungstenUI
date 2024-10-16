#pragma once
#include "anduril/lockout-mode.h"

uint8_t lockout_state(Event event, uint16_t arg) {

    if (event == EV_enter_state) {
        rgb_led_update(0x00, 0);  //AUX LED Off
        button_led_set(0);  //Button LED Off
        ticks_since_on = 0;
    }

    else if (event == EV_tick) {
        if (arg > HOLD_TIMEOUT) {
            go_to_standby = 1;
        }
        return EVENT_HANDLED;
    }

    else if (event == EV_sleep_tick) {
        if (ticks_since_on < 255) ticks_since_on ++;
        return EVENT_HANDLED;
    }

    // 6C: Off
    else if (event == EV_6clicks) {
        rgb_led_update(0x22, 0); //Blip green AUX
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}