#pragma once
#include "anduril/off-mode.h"

static int8_t momentary = 0;
static int8_t AUXtoggle = 0;

uint8_t off_state(Event event, uint16_t arg) {

    if (event == EV_enter_state) {     
        ticks_since_on = 0;
        momentary = 0; 
        AUXtoggle = 0; 
        button_led_set(1); //Button LED Low
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
        } else if (!AUXtoggle) {
            rgb_led_update(0x00, 0); //AUX LED Off
        }
        return EVENT_HANDLED;
    }

    // 1C: Toggle AUX LEDs
    else if (event == EV_click1_press) {
        if (AUXtoggle) {
            rgb_led_update(0x00, 0); //AUX LED Off
            button_led_set(1); //Button LED Low
        } else {
            rgb_led_update(0x20, 0); //AUX LED Red
            button_led_set(0); //Button LED Off
        } 
        AUXtoggle = (1-AUXtoggle); 
        
        return EVENT_HANDLED;
    }

    // 1H: Ramp
    else if (event == EV_click1_hold) {
        set_state(steady_state, 1);
    }

    // (2 clicks initial press): go to max, allow abort for triple click
    else if (event == EV_click2_press) {
        set_level(150);
        button_led_set(2); //Button LED High
        return EVENT_HANDLED;
    }

    // 2C: Max Level
    else if (event == EV_2clicks) {
        set_state(steady_state, 150);
        return EVENT_HANDLED;
    }

    // 2H: Max Level Momentary
    else if (event == EV_click2_hold) {
        // reset button sequence to avoid activating anything in ramp mode
        current_event = 0;
        momentary = 1;
        set_state(steady_state, 150);
        return EVENT_HANDLED;
    }

    // (3 clicks initial press): off, to prep for later events
    else if (event == EV_click3_press) {
        set_level(0);
        button_led_set(1); //Button LED Low
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
        blip();
        set_state(lockout_state, 0);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}