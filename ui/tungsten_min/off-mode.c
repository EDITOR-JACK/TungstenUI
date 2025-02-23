#pragma once
#include "anduril/off-mode.h"

static int8_t momentary = 0;

uint8_t off_state(Event event, uint16_t arg) {

    if (event == EV_enter_state) {     
        ticks_since_on = 0;
        momentary = 0;    
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
        return EVENT_HANDLED;
    }

    // (1 click initial press)
    else if (event == EV_click1_press) {
        set_level(150);
        return EVENT_HANDLED;
    }

    // 1C
    else if (event == EV_1click) {
        set_state(steady_state, 150);
        return EVENT_HANDLED;
    }

    // 1H:
    else if (event == EV_click1_hold) {
        // reset button sequence to avoid activating anything in ramp mode
        current_event = 0;
        momentary = 1;
        set_state(steady_state, 150);
    }

    // (2 clicks initial press): off, to prep for later events
    else if (event == EV_click2_press) {
        set_level(0);
        return EVENT_HANDLED;
    }

    // 4C: Battcheck
    else if (event == EV_4clicks) {
        set_state(battcheck_state, 0);
        return EVENT_HANDLED;
    }

    // 5C: Tempcheck
    else if (event == EV_5clicks) {
        set_state(tempcheck_state, 0);
        return EVENT_HANDLED;
    }

    // 6C: Lockout
    else if (event == EV_6clicks) {
        set_state(lockout_state, 0);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}