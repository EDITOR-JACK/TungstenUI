#pragma once
#include "anduril/ramp-mode.h"

uint8_t steady_state(Event event, uint16_t arg) {
    static int8_t ramp_direction = 1;
    
    // Enter State
    if (event == EV_enter_state) {
        ramp_direction = 1;
        memorized_level = arg;
        // Use RED channel instead of Moonlight?
        if (redMoon) {
            channel_mode = 1;
        } else {
            channel_mode = 0;
        }
        set_level_and_therm_target(arg);
        
        return EVENT_HANDLED;
    }

    // 1C -> OFF (or when momentary turbo from OFF is released)
    else if (event == EV_1click) {
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    // 1H/2H -> Change Brightness (2C -> +1 level)
    if (((event == EV_click1_hold) || (event == EV_click2_hold) || (event == EV_click2_release)) && !redMoon) {

        // ramp slower
        if (arg % 2 && arg != 0) {
            return EVENT_HANDLED;
        }
        // set ramp direction on first frame
        if (!arg) {
            if (event == EV_click2_hold) { ramp_direction = -1; }
            else { ramp_direction = 1; }
        }

        memorized_level = nearest_level((int16_t)actual_level + ramp_direction);

        set_level_and_therm_target(memorized_level);

        return EVENT_HANDLED;
    }

    // 3C -> Just handle it here to prevent channel switching
    else if (event == EV_3clicks) {
        return EVENT_HANDLED;
    }

    else if (event == EV_tick) {

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

    return EVENT_NOT_HANDLED;
}

// find the ramp level closest to the target, using only the levels which are allowed
uint8_t nearest_level(int16_t target) {

    // ensure all globals are correct
    ramp_update_config();

    // bounds check
    uint8_t mode_min = 1;
    uint8_t mode_max = MAX_LEVEL;

    if (target < mode_min) return mode_min;
    if (target > mode_max) return mode_max;

    return target;
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

#ifdef USE_MANUAL_MEMORY
void manual_memory_restore() {
    memorized_level = cfg.manual_memory;
    #if NUM_CHANNEL_MODES > 1
        channel_mode = cfg.channel_mode = cfg.manual_memory_channel_mode;
    #endif
    #ifdef USE_CHANNEL_MODE_ARGS
        for (uint8_t i=0; i<NUM_CHANNEL_MODES; i++)
          cfg.channel_mode_args[i] = cfg.manual_memory_channel_args[i];
    #endif
}
#endif  // ifdef USE_MANUAL_MEMORY