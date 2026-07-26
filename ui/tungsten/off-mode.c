#pragma once
#include "anduril/off-mode.h"

// Preset levels for Moonlight, Low, High
uint8_t LVLS[3] = { 5, 40, 90};

// Transition fade timing (higher = slower)
uint8_t FadeTime = 8;

// Was thermal throttling required on last activation?
bool overheat = false;

// Set level smooth maybe
void off_state_set_level(uint8_t level);

uint8_t off_state(Event event, uint16_t arg) {

    // turn emitter off when entering state
    if (event == EV_enter_state) {
        // Turn off
        off_state_set_level(0);
        // Update aux LEDs now to avoid waiting for sleep
        #ifdef USE_INDICATOR_LED
        indicator_led_update(cfg.indicator_led_mode & 0x03, arg);
        #elif defined(USE_AUX_RGB_LEDS)
        rgb_led_update(cfg.rgb_led_off_mode, arg);
        #endif
        // don't go to sleep while animating
        arg |= smooth_steps_in_progress;
        ticks_since_on = 0;
        // sleep while off  (lower power use)
        // (unless delay requested; give the ADC some time to catch up)
        if (! arg) { 
            go_to_standby = 1; 
            // Reset channel mode
            channel_mode = 0;
        }
        return EVENT_HANDLED;
    }

    // go back to sleep eventually if we got bumped but didn't leave "off" state
    else if (event == EV_tick) {
        if (arg > HOLD_TIMEOUT && (! smooth_steps_in_progress)) {
            go_to_standby = 1;
        }
        return EVENT_HANDLED;
    }

    // blink the indicator LED, maybe
    else if (event == EV_sleep_tick) {
        if (ticks_since_on < 255) ticks_since_on ++;

        // Voltage low, not critical (for Lithium only)
        if ((voltage <= VOLTAGE_RED) && (voltage > VOLTAGE_LOW) && (arg <= 40)) {
            // Blink AUX Red (or indicator LED) for 5 seconds
            #ifdef USE_INDICATOR_LED
            indicator_led_update(3, arg);
            #elif defined(USE_AUX_RGB_LEDS)
            rgb_led_update(0x30, arg);
            #endif
        } 
        /*else if (overheat && (arg <= 6)) {
            // Blink AUX Blue
            #if defined(USE_AUX_RGB_LEDS)
            rgb_led_update(0x34, arg);
            #endif
        } */
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

    // 1H -> Moonlight
    // (Stay off until hold timing complete, then come on at moonlight level)
    else if (event == EV_click1_hold) {
        if (cfg.channel_mode == 1) {
            channel_mode = 1;
            off_state_set_level(MAX_LEVEL);
        } else {
            off_state_set_level(LVLS[0]);
        }
        return EVENT_HANDLED;
    }

    // (Releasing will now enter steady state in moonlight mode)
    else if (event == EV_click1_hold_release) {
        set_state(steady_state, LVLS[0]);
        return EVENT_HANDLED;
    }



    // 1C -> LOW
    // (start transition to LOW level if first click not held)
    else if (event == EV_click1_release) {
        off_state_set_level(LVLS[1]);
        return EVENT_HANDLED;
    }

    // (confirm LOW level if no second click)
    else if (event == EV_1click) {
        set_state(steady_state, LVLS[1]);
        return EVENT_HANDLED;
    }



    // 2C -> HIGH
    // (start transition to HIGH level immediately on second click)
    else if (event == EV_click2_press) {
        // immediately cancel any animations in progress
        smooth_steps_in_progress = 0;
        off_state_set_level(LVLS[2]);
        return EVENT_HANDLED;
    }

    // (confirm HIGH level if second click not held)
    else if (event == EV_2clicks) {
        set_state(steady_state, LVLS[2]);
        return EVENT_HANDLED;
    }



    // 2H -> TURBO
    else if (event == EV_click2_hold) {
        // immediately cancel any animations in progress
        smooth_steps_in_progress = 0;
        set_state(steady_state, MAX_LEVEL);
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