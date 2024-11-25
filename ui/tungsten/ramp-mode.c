#pragma once
#include "anduril/ramp-mode.h"

uint8_t steady_state(Event event, uint16_t arg) {
    static int8_t ramp_direction = 1;
    static uint8_t level_before_off = 0;

    // turn LED on when we first enter the mode
    if (event == EV_enter_state) {
        set_level_and_therm_target(arg);
        ramp_direction = 1;
        return EVENT_HANDLED;
    }

    // 1 click (early): OFF
    else if (event == EV_click1_press) {
        level_before_off = actual_level;
        set_level_and_therm_target(0);
        return EVENT_HANDLED;
    }

    // 1C: OFF
    else if (event == EV_1click) {
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    // 2C: Turbo
    else if (event == EV_2clicks) {
        set_level_and_therm_target(150);
        return EVENT_HANDLED;
    }

    // hold: change brightness (brighter, dimmer)
    // click, hold: change brightness (dimmer)
    else if ((event == EV_click1_hold) || (event == EV_click2_hold)) {
        // fix ramp direction on first frame if necessary
        if (!arg) {
            set_level_and_therm_target(level_before_off);
            // click, hold should always go down if possible
            if (event == EV_click2_hold) { ramp_direction = -1; }
            // make it ramp down instead, if already at max
            else if (actual_level >= 150) { ramp_direction = -1; }
            // make it ramp up if already at min
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

