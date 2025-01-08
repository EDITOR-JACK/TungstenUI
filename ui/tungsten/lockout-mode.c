#pragma once
#include "anduril/lockout-mode.h"

uint8_t lockout_state(Event event, uint16_t arg) {

    if (event == EV_enter_state) {
        #ifdef USE_BUTTON_LED
        button_led_set(0);  //Button LED Off
        #endif
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
        // if low (but not critical) voltage
        if ((voltage) && (voltage < VOLTAGE_RED)) {
            rgb_led_update(0x30, arg); //AUX LED Red Blink
        } else {
            rgb_led_update(0x00, 0); //AUX LED Off
        }
        return EVENT_HANDLED;
    }

    // 6C: Off
    else if (event == EV_click6_press) {
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}