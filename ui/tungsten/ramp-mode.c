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
        // remember this level, unless it's moon or turbo
        if ((arg > mode_min) && (arg < mode_max))
            memorized_level = arg;
        // use the requested level even if not memorized
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
        // ramp infrequently in stepped mode
        if (cfg.ramp_style && (arg % HOLD_TIMEOUT != 0))
            return EVENT_HANDLED;
        #ifdef USE_RAMP_SPEED_CONFIG
            // ramp slower if user configured things that way
            if ((! cfg.ramp_style) && (arg % ramp_speed))
                return EVENT_HANDLED;
        #endif
        #ifdef USE_SMOOTH_STEPS
            // if a brightness transition is already happening,
            // don't interrupt it
            // (like 2C for full turbo then 1H to smooth ramp down
            //  ... without this clause, it flickers because it trips
            //  the "blink at ramp ceil" clause below, over and over)
            if (smooth_steps_in_progress) return EVENT_HANDLED;
        #endif
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
        else if ((arg > TICKS_PER_SECOND * 5
                    #ifdef USE_RAMP_SPEED_CONFIG
                    // FIXME: count from time actual_level hits mode_max,
                    //   not from beginning of button hold
                    * ramp_speed
                    #endif
                    ) && (actual_level >= 150)) {
            ramp_direction = -1;
        }
        memorized_level = nearest_level((int16_t)actual_level + (ramp_direction));

        if ((memorized_level != actual_level) && (
                0  // for easier syntax below
                #ifdef BLINK_AT_RAMP_CEIL
                // FIXME: only blink at top when going up, not down
                || (memorized_level == 150)
                #endif
                #ifdef BLINK_AT_RAMP_FLOOR
                || (memorized_level == 0)
                #endif
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
        #ifdef THERM_HARD_TURBO_DROP
        //if (actual_level > THERM_FASTER_LEVEL) {
        if (actual_level == 150) {
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
            else if (stepdown > 150) stepdown = 150;
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

    return EVENT_NOT_HANDLED;
}

// find the ramp level closest to the target,
// using only the levels which are allowed in the current state
uint8_t nearest_level(int16_t target) {
    // using int16_t here saves us a bunch of logic elsewhere,
    // by allowing us to correct for numbers < 0 or > 255 in one central place

    // ensure all globals are correct
    ramp_update_config();

    // bounds check
    uint8_t mode_min = 0;
    uint8_t mode_max = 150;
    uint8_t num_steps = cfg.ramp_stepss[1
    #ifdef USE_SIMPLE_UI
        + cfg.simple_ui_active
    #endif
        ];
    // special case for 1-step ramp... use halfway point between floor and ceiling
    if (cfg.ramp_style && (1 == num_steps)) {
        uint8_t mid = (mode_max + mode_min) >> 1;
        return mid;
    }
    if (target < mode_min) return mode_min;
    if (target > mode_max) return mode_max;
    // the rest isn't relevant for smooth ramping
    if (! cfg.ramp_style) return target;

    uint8_t ramp_range = mode_max - mode_min;
    ramp_discrete_step_size = ramp_range / (num_steps-1);
    uint8_t this_level = mode_min;

    for(uint8_t i=0; i<num_steps; i++) {
        this_level = mode_min + (i * (uint16_t)ramp_range / (num_steps-1));
        int16_t diff = target - this_level;
        if (diff < 0) diff = -diff;
        if (diff <= (ramp_discrete_step_size>>1))
            return this_level;
    }
    return this_level;
}

// ensure ramp globals are correct
void ramp_update_config() {
    uint8_t which = cfg.ramp_style;
    #ifdef USE_SIMPLE_UI
    if (cfg.simple_ui_active) { which = 2; }
    #endif

    ramp_floor = cfg.ramp_floors[which];
    ramp_ceil  = cfg.ramp_ceils[which];
}

#if defined(USE_THERMAL_REGULATION) || defined(USE_SMOOTH_STEPS)
void set_level_and_therm_target(uint8_t level) {
    #ifdef USE_THERMAL_REGULATION
    target_level = level;
    #endif
    #ifdef USE_SMOOTH_STEPS
        // if adjusting by more than 1 ramp level,
        // animate the step change (if smooth steps enabled)
        uint8_t diff = (level > actual_level)
            ? (level - actual_level) : (actual_level - level);
        if (smooth_steps_in_progress
            || (cfg.smooth_steps_style && (diff > 1)))
            set_level_smooth(level, 4);
        else
    #endif
    set_level(level);
}
#else
#define set_level_and_therm_target(level) set_level(level)
#endif