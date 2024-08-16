#pragma once

static int8_t moonToggle = 0;

uint8_t lockout_state(Event event, uint16_t arg) {

    if (event == EV_enter_state) {
        ticks_since_on = 0;
        rgb_led_update(0x00, 0); //AUX LED off
        moonToggle = 0;
    }

    else if (event == EV_tick) {
        if (arg > HOLD_TIMEOUT) {
            go_to_standby = 1;
        }
        return EVENT_HANDLED;
    }

    // 1C (early): AUX Toggle
    else if (event == EV_click1_press) {
        if (moonToggle) {
            set_level(0);
        } else {
            set_level(1);
        } 
        moonToggle = (1-moonToggle);     
        return EVENT_HANDLED;
    }

    // 1H: Momentary AUX
    else if (event == EV_click1_hold_release) {
        set_level(0);
        moonToggle = 0;
    }

    // 2 clicks (early): turn on at least min level
    else if (event == EV_click2_press) {
        set_level(1);
    }

    // 2C: Max Level
    else if (event == EV_click2_release) {
        memorized_level = 150;
        set_state(steady_state, memorized_level);
        return EVENT_HANDLED;
    }

    // 2H: Set Level
    else if (event == EV_click2_hold) {
        set_state(steady_state, 1);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}