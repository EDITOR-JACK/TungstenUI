#pragma once
#include "anduril/off-mode.h"

// Transition fade timing (higher = slower)
uint8_t FadeTime = 10;

// Was RED moonlight mode just active?
bool redMoon = false;

// Set level smooth maybe
void off_state_set_level(uint8_t level);

uint8_t off_state(Event event, uint16_t arg) {

    // turn emitter off when entering state
    if (event == EV_enter_state) {

        if (redMoon) {
           // Turn off immediately 
           set_level(0);
        } else {
            // Turn off smoothly
            off_state_set_level(0);
        }

        redMoon = false;
        
        // don't go to sleep while animating
        #ifdef USE_SMOOTH_STEPS
        arg |= smooth_steps_in_progress;
        #endif
        if (! arg) { 
            // Sleep
            go_to_standby = 1; 
            // Reset channel mode
            channel_mode = 0;
        }
        return EVENT_HANDLED;
    }

    // go back to sleep eventually if we got bumped but didn't leave "off" state
    else if (event == EV_tick) {
        if (arg > HOLD_TIMEOUT
            #ifdef USE_SMOOTH_STEPS 
            && (! smooth_steps_in_progress)
            #endif
        ) {
            go_to_standby = 1;
        }
        return EVENT_HANDLED;
    }

    // blink the indicator LED, maybe
    else if (event == EV_sleep_tick) {

        // Voltage low, not critical (for Lithium only)
        if ((voltage <= VOLTAGE_RED) && (voltage > VOLTAGE_LOW) && (arg <= 40)) {
            // Blink AUX Red (or indicator LED) for 5 seconds
            #ifdef USE_INDICATOR_LED
            indicator_led_update(3, arg);
            #elif defined(USE_AUX_RGB_LEDS)
            rgb_led_update(0x30, arg);
            #endif
        } 
        else {
            // use configured AUX settings
            #ifdef USE_INDICATOR_LED
            indicator_led_update(cfg.indicator_led_mode & 0x03, arg);
            #elif defined(USE_AUX_RGB_LEDS)
            rgb_led_update(cfg.rgb_led_off_mode, arg);
            #endif
        }
        
        return EVENT_HANDLED;
    }

    //---------- OPERATIONS START ----------

    // 1C/1H -> Moonlight Ramp
    else if (event == EV_1click || event == EV_click1_hold) {
        set_state(steady_state, LVLS[0]);
        return EVENT_HANDLED;
    }

    // 3C -> Red Moon (RED AUX HIGH)
    else if (event == EV_3clicks) {
        redMoon = true;
        set_state(steady_state, MAX_LEVEL);
        return EVENT_HANDLED;
    }

    // 3H -> AUX toggle (LOW RED)
    else if (event == EV_click3_hold) {
        if (cfg.rgb_led_off_mode == 0x00) {
            //Set RGB AUX config to LOW (0x1_) and RED (0x_0)
            cfg.rgb_led_off_mode = 0x10;
        } else {
            //Set RGB AUX config to OFF (0x00)
            cfg.rgb_led_off_mode = 0x00;
        }
        save_config();
        return EVENT_HANDLED;
    }

    // 4C -> Indicator LED toggle
    else if (event == EV_click4_press) {
        if (cfg.indicator_led_mode == 0) {
            //Set Indicator LED config to LOW
            cfg.indicator_led_mode = 1;
        } else {
            //Set Indicator LED config to OFF
            cfg.indicator_led_mode = 0;
        }
        save_config();
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}

void off_state_set_level(uint8_t level) {
    // this pattern gets used a few times, so reduce duplication
    #ifdef USE_SMOOTH_STEPS
        if (cfg.smooth_steps_style) set_level_smooth(level, FadeTime);
        else
    #endif
    set_level(level);
}