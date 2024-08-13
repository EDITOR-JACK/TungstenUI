#pragma once
#include "anduril/ramp-mode.h"

#ifdef USE_SUNSET_TIMER
#include "anduril/sunset-timer.h"
#endif

#ifdef USE_SMOOTH_STEPS
#include "anduril/smooth-steps.h"
#endif


uint8_t steady_state(Event event, uint16_t arg) {
    static int8_t ramp_direction = 1;
    #if (B_TIMING_OFF == B_RELEASE_T)
    // if the user double clicks, we need to abort turning off,
    // and this stores the level to return to
    static uint8_t level_before_off = 0;
    #endif

    // turn LED on when we first enter the mode
    if (event == EV_enter_state) {
        // remember this level, unless it's 2C turbo
        if (arg < 150)
            memorized_level = arg;
        // use the requested level even if not memorized
        arg = nearest_level(arg);
        set_level_and_therm_target(arg);
        ramp_direction = 1;
        return EVENT_HANDLED;
    }
    #if (B_TIMING_OFF == B_RELEASE_T)
    // 1 click (early): off, if configured for early response
    else if (event == EV_click1_release) {
        level_before_off = actual_level;
        set_level_and_therm_target(0);
        return EVENT_HANDLED;
    }
    // 2 clicks (early): abort turning off, if configured for early response
    else if (event == EV_click2_press) {
        set_level_and_therm_target(level_before_off);
        return EVENT_HANDLED;
    }
    #endif  // if (B_TIMING_OFF == B_RELEASE_T)
    // 1 click: off
    else if (event == EV_1click) {
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }
    // 2 clicks: go to/from highest level
    else if (event == EV_2clicks) {
        if (actual_level < 150) {
            set_level_and_therm_target(150);
        }
        else {
            set_level_and_therm_target(memorized_level);
        }
        return EVENT_HANDLED;
    }

    // hold: change brightness (brighter, dimmer)
    // click, hold: change brightness (dimmer)
    else if ((event == EV_click1_hold) || (event == EV_click2_hold)) {
        // fix ramp direction on first frame if necessary
        if (!arg) {
            // click, hold should always go down if possible
            if (event == EV_click2_hold) { ramp_direction = -1; }
            // make it ramp down instead, if already at max
            else if (actual_level >= 150) { ramp_direction = -1; }
            // make it ramp up if already at min
            // (off->hold->stepped_min->release causes this state)
            else if (actual_level <= 1) { ramp_direction = 1; }
        }
        // if the button is stuck, err on the side of safety and ramp down
        else if ((arg > TICKS_PER_SECOND * 5) && (actual_level >= 150)) {
            ramp_direction = -1;
        }
        memorized_level = nearest_level((int16_t)actual_level + ramp_direction);

        //blink at min or max level
        if ((memorized_level != actual_level) && (
                0  // for easier syntax below
                || (memorized_level == 150)
                || (memorized_level == 1)
                )) {
            blip();
        }
        set_level_and_therm_target(memorized_level);
        return EVENT_HANDLED;
    }
    // reverse ramp direction on hold release
    else if ((event == EV_click1_hold_release)
             || (event == EV_click2_hold_release)) {
        ramp_direction = -ramp_direction;
        #ifdef START_AT_MEMORIZED_LEVEL
        save_config_wl();
        #endif
        return EVENT_HANDLED;
    }

    else if (event == EV_tick) {
        // un-reverse after 1 second
        if (arg == AUTO_REVERSE_TIME) ramp_direction = 1;

        #ifdef USE_SUNSET_TIMER
        // reduce output if shutoff timer is active
        if (sunset_timer) {
            uint8_t dimmed_level = sunset_timer_orig_level * sunset_timer / sunset_timer_peak;
            uint8_t dimmed_level_next = sunset_timer_orig_level * (sunset_timer-1) / sunset_timer_peak;
            uint8_t dimmed_level_delta = dimmed_level - dimmed_level_next;
            dimmed_level -= dimmed_level_delta * (sunset_ticks/TICKS_PER_SECOND) / 60;
            if (dimmed_level < 1) dimmed_level = 1;

            #ifdef USE_SET_LEVEL_GRADUALLY
            set_level_gradually(dimmed_level);
            target_level = dimmed_level;
            #else
            set_level_and_therm_target(dimmed_level);
            #endif
        }
        #endif  // ifdef USE_SUNSET_TIMER

        #ifdef USE_SET_LEVEL_GRADUALLY
        int16_t diff = gradual_target - actual_level;
        static uint16_t ticks_since_adjust = 0;
        ticks_since_adjust++;
        if (diff) {
            uint16_t ticks_per_adjust = 256 / GRADUAL_ADJUST_SPEED;
            if (diff < 0) {
                //diff = -diff;
                if (actual_level > THERM_FASTER_LEVEL) {
                    #ifdef THERM_HARD_TURBO_DROP
                    ticks_per_adjust >>= 2;
                    #endif
                    ticks_per_adjust >>= 2;
                }
            } else {
                // rise at half speed
                ticks_per_adjust <<= 1;
            }
            while (diff) {
                ticks_per_adjust >>= 1;
                //diff >>= 1;
                diff /= 2;  // because shifting produces weird behavior
            }
            if (ticks_since_adjust > ticks_per_adjust)
            {
                gradual_tick();
                ticks_since_adjust = 0;
            }
        }
        #endif  // ifdef USE_SET_LEVEL_GRADUALLY
        return EVENT_HANDLED;
    }

    #ifdef USE_THERMAL_REGULATION
    // overheating: drop by an amount proportional to how far we are above the ceiling
    else if (event == EV_temperature_high) {
        #if 0
        blip();
        #endif
        #ifdef THERM_HARD_TURBO_DROP
        //if (actual_level > THERM_FASTER_LEVEL) {
        if (actual_level == MAX_LEVEL) {
            #ifdef USE_SET_LEVEL_GRADUALLY
            set_level_gradually(THERM_FASTER_LEVEL);
            target_level = THERM_FASTER_LEVEL;
            #else
            set_level_and_therm_target(THERM_FASTER_LEVEL);
            #endif
        } else
        #endif
        if (actual_level > MIN_THERM_STEPDOWN) {
            int16_t stepdown = actual_level - arg;
            if (stepdown < MIN_THERM_STEPDOWN) stepdown = MIN_THERM_STEPDOWN;
            else if (stepdown > MAX_LEVEL) stepdown = MAX_LEVEL;
            #ifdef USE_SET_LEVEL_GRADUALLY
            set_level_gradually(stepdown);
            #else
            set_level(stepdown);
            #endif
        }
        return EVENT_HANDLED;
    }
    // underheating: increase slowly if we're lower than the target
    //               (proportional to how low we are)
    else if (event == EV_temperature_low) {
        #if 0
        blip();
        #endif
        if (actual_level < target_level) {
            //int16_t stepup = actual_level + (arg>>1);
            int16_t stepup = actual_level + arg;
            if (stepup > target_level) stepup = target_level;
            else if (stepup < MIN_THERM_STEPDOWN) stepup = MIN_THERM_STEPDOWN;
            #ifdef USE_SET_LEVEL_GRADUALLY
            set_level_gradually(stepup);
            #else
            set_level(stepup);
            #endif
        }
        return EVENT_HANDLED;
    }
    #ifdef USE_SET_LEVEL_GRADUALLY
    // temperature is within target window
    // (so stop trying to adjust output)
    else if (event == EV_temperature_okay) {
        // if we're still adjusting output...  stop after the current step
        if (gradual_target > actual_level)
            gradual_target = actual_level + 1;
        else if (gradual_target < actual_level)
            gradual_target = actual_level - 1;
        return EVENT_HANDLED;
    }
    #endif  // ifdef USE_SET_LEVEL_GRADUALLY
    #endif  // ifdef USE_THERMAL_REGULATION

    ////////// Every action below here is blocked in the simple UI //////////
    // That is, unless we specifically want to enable 3C for smooth/stepped selection in Simple UI
    #if defined(USE_SIMPLE_UI) && !defined(USE_SIMPLE_UI_RAMPING_TOGGLE)
    if (cfg.simple_ui_active) {
        return EVENT_NOT_HANDLED;
    }
    #endif

    // 3 clicks: toggle smooth vs discrete ramping
    // (and/or 6 clicks when there are multiple channel modes)
    // (handle 3C here anyway, when all but 1 mode is disabled)
    else if ((event == EV_3clicks)
        #if NUM_CHANNEL_MODES > 1
             || (event == EV_6clicks)
        ) {
            // detect if > 1 channel mode is enabled,
            // and if so, fall through so channel mode code can handle it
            // otherwise, change the ramp style
            if (event == EV_3clicks) {
                uint8_t enabled = 0;
                for (uint8_t m=0; m<NUM_CHANNEL_MODES; m++)
                    enabled += channel_mode_enabled(m);
                if (enabled > 1)
                    return EVENT_NOT_HANDLED;
            }
        #else
        ) {
        #endif

        cfg.ramp_style = !cfg.ramp_style;
        save_config();
        #ifdef START_AT_MEMORIZED_LEVEL
        save_config_wl();
        #endif
        blip();
        memorized_level = nearest_level(actual_level);
        set_level_and_therm_target(memorized_level);
        #ifdef USE_SUNSET_TIMER
        reset_sunset_timer();
        #endif
        return EVENT_HANDLED;
    }

    return EVENT_NOT_HANDLED;
}

uint8_t nearest_level(int16_t target) {
    if (target < 1) return 1;
    if (target > 150) return 150;
    return target;
}

void set_level_and_therm_target(uint8_t level) {
    target_level = level;
    set_level(level);
}