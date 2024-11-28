#pragma once
#include "anduril/off-mode.h"

uint8_t off_state(Event event, uint16_t arg) {

    if (event == EV_enter_state) {     
        ticks_since_on = 0;
        // sleep while off (unless delay requested)
        if (! arg) { go_to_standby = 1; }
        return EVENT_HANDLED;
    }

    // go back to sleep eventually if we got bumped but didn't leave "off" state
    else if (event == EV_tick) {
        if (arg > HOLD_TIMEOUT) {
            go_to_standby = 1;
        }
        return EVENT_HANDLED;
    }

    else if (event == EV_sleep_tick) {
        if (ticks_since_on < 255) ticks_since_on ++;
        // if low (but not critical) voltage
        if ((voltage) && (voltage < VOLTAGE_RED)) {
            rgb_led_update(0x30, arg); //AUX LED Red Blink
        } else {
            rgb_led_update(0x00, 0); //AUX LED Off
        }
        return EVENT_HANDLED;
    }

    // Click instantly enters ON state
    else if (event == EV_click1_press) {
        set_state(steady_state, 1);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}