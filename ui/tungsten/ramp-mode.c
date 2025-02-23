#pragma once
#include "anduril/ramp-mode.h"

uint8_t ramp_start = 1;
uint8_t off_ready = 0;

uint8_t steady_state(Event event, uint16_t arg) {  

    // Enter State
    if (event == EV_enter_state) {
        memorized_level = arg;
        set_level_and_therm_target(arg);
        ramp_start = 1;
        off_ready = 0;
        return EVENT_HANDLED;
    }

    // 1 click (early): OFF
    else if (event == EV_click1_press) {
        set_level_and_therm_target(0);
        off_ready = 1;
        return EVENT_HANDLED;
    }

    // 1C: OFF
    else if (event == EV_1click && off_ready) {
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    // 1H: Ramp Again
    else if (event == EV_click1_hold && !ramp_start) {
        memorized_level = nearest_level((int16_t)actual_level + 1);
        set_level_and_therm_target(memorized_level);
        return EVENT_HANDLED;
    }

    // 2C: Turbo
    else if (event == EV_click2_press) {
        set_level_and_therm_target(150);
        return EVENT_HANDLED;
    }

    // Stop ramping on release
    else if (((event & (B_CLICK | B_PRESS)) == (B_CLICK)) && ramp_start) {
        ramp_start = 0;
        return EVENT_HANDLED;
    }

    // Ramping
    else if (event == EV_tick){
        if (ramp_start) {
            memorized_level = nearest_level((int16_t)actual_level + 1);
            set_level_and_therm_target(memorized_level);  
        }
        return EVENT_HANDLED;
    }

    // overheating: drop by an amount proportional to how far we are above the ceiling
    else if (event == EV_temperature_high) {
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
        if (actual_level < target_level) {
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

uint8_t nearest_level(int16_t target) {
    if (target < 1) return 1;
    if (target > 150) return 150;
    return target;
}

void set_level_and_therm_target(uint8_t level) {
    target_level = level;
    set_level(level);
}

//Looks like these need to be defined here in order for FSM to compile...

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

void manual_memory_save() {
    cfg.manual_memory = actual_level;
    #if NUM_CHANNEL_MODES > 1
        cfg.manual_memory_channel_mode = channel_mode;
    #endif
    #ifdef USE_CHANNEL_MODE_ARGS
        for (uint8_t i=0; i<NUM_CHANNEL_MODES; i++)
          cfg.manual_memory_channel_args[i] = cfg.channel_mode_args[i];
    #endif
}
#endif  // ifdef USE_MANUAL_MEMORY

#ifdef USE_SUNSET_TIMER
void reset_sunset_timer() {
    if (sunset_timer) {
        sunset_timer_orig_level = actual_level;
        sunset_timer_peak = sunset_timer;
        sunset_ticks = 0;
    }
}
#endif

