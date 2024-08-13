#pragma once
#include "anduril/off-mode.h"

uint8_t off_state(Event event, uint16_t arg) {

    // turn emitter off when entering state
    if (event == EV_enter_state) {
        set_level(0);
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
        // lock the light after being off for N minutes
            uint16_t ticks = cfg.autolock_time * SLEEP_TICKS_PER_MINUTE;
            if ((cfg.autolock_time > 0)  && (arg > ticks)) {
                set_state(lockout_state, 0);
            }
        return EVENT_HANDLED;
    }

    // (1 click initial press): go to memorized level, but allow abort for double click
    else if (event == EV_click1_press) {
        set_level(memorized_level);
        return EVENT_HANDLED;
    }

    // 1C: Memorized Level
    else if (event == EV_1click) {
        set_state(steady_state, memorized_level);
        return EVENT_HANDLED;
    }

    // 1H: Memorized Level
    else if (event == EV_click1_hold) {
        // reset button sequence to avoid activating anything in ramp mode
        current_event = 0;
        set_state(steady_state, memorized_level);
        return EVENT_HANDLED;
    }

    // 2C: Max Level
    else if (event == EV_2clicks) {
        set_state(steady_state, MAX_LEVEL);
        return EVENT_HANDLED;
    }

    // (3 clicks initial press): off, to prep for later events
    else if (event == EV_click3_press) {
        set_level(0);
        return EVENT_HANDLED;
    }

    // 3C: Lockout Manually
    else if (event == EV_3clicks) {
        set_state(lockout_state, 0);
        return EVENT_HANDLED;
    }

    // 4C: Battcheck
    else if (event == EV_4clicks) {
        set_state(battcheck_state, 0);
        return EVENT_HANDLED;
    }

    // 13C: Factory Reset (reboot)
    else if (event == EV_click13_hold) {
        reboot();
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}