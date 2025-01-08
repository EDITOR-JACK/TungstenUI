#pragma once
#include "anduril/off-mode.h"

//Duration (in seconds) that light remains OFF
#define OFF_DURATION 10

static int reset = 0;
static int off_ticks = 0;
static int starting_level = 150;

uint8_t off_state(Event event, uint16_t arg) {

    if (event == EV_enter_state) { 
        off_ticks = 0;
        starting_level = 150;
        if (arg == 1) {
            reset = 0;
            button_led_set(0);
        } else {
            reset = 1;
            button_led_set(2); 
        }
        return EVENT_HANDLED;
    }

    else if (event == EV_tick) {
        off_ticks++;
        if (temperature > 35) {
            starting_level = 100;
        } else {
            starting_level = 150;
        }
        if (off_ticks > 62*OFF_DURATION && !reset) {
            // RESET
            reset = 1;
            button_led_set(2);
        }
        return EVENT_HANDLED;
    }

    // Click to ON
    else if (event == EV_click1_press && reset) {
        button_led_set(0);
        set_level(starting_level);
        return EVENT_HANDLED;
    }

    // 1C
    else if (event == EV_1click && reset) {
        set_state(steady_state, starting_level);
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