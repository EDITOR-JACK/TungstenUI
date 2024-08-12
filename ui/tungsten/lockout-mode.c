#pragma once

uint8_t lockout_state(Event event, uint16_t arg) {
    // momentary moon mode during lockout
    // button held
    if ((event & (B_CLICK | B_PRESS)) == (B_CLICK | B_PRESS)) {
        set_level(1);
    }
    // button released
    else if ((event & (B_CLICK | B_PRESS)) == (B_CLICK)) {
        set_level(0);
    }

    if (event == EV_enter_state) {
        ticks_since_on = 0;
        rgb_led_update(0x00, 0); //AUX LED off
    }

    else if (event == EV_tick) {
        if (arg > HOLD_TIMEOUT) {
            go_to_standby = 1;
        }
        return EVENT_HANDLED;
    }

    // 2C: Memorized Level
    else if (event == EV_2clicks) {
        set_state(steady_state, memorized_level);
        return EVENT_HANDLED;
    }

    // 3C: Max Level
    else if (event == EV_3clicks) {
        set_state(steady_state, memorized_level);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}