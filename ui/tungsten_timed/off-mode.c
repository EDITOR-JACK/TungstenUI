#pragma once
#include "anduril/off-mode.h"

//Duration (in seconds) that light remains OFF
#define OFF_DURATION 30

static int reset = 0;
static int off_ticks = 0;

uint8_t off_state(Event event, uint16_t arg) {

    if (event == EV_enter_state) { 
        off_ticks = 0;
        if (arg == 1) {
            reset = 1;
            button_led_set(2);
        } else {
            reset = 0;
            button_led_set(0);
        }
        return EVENT_HANDLED;
    }

    else if (event == EV_tick) {
        off_ticks++;
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
        set_state(steady_state, 150);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}