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

    // If we allowed 3C in Simple UI, now block further actions
    #if defined(USE_SIMPLE_UI) && defined(USE_SIMPLE_UI_RAMPING_TOGGLE)
    if (cfg.simple_ui_active) {
        return EVENT_NOT_HANDLED;
    }
    #endif

    // 3H: momentary turbo (on lights with no tint ramping)
    // (or 4H when tint ramping is available)
    else if ((event == EV_click3_hold)
            #ifdef USE_CHANNEL_MODE_ARGS
            || (event == EV_click4_hold)
            #endif
        ) {
        #ifdef USE_CHANNEL_MODE_ARGS
            // ramp tint if tint exists in this mode
            if ((event == EV_click3_hold)
                && (channel_has_args(channel_mode)))
                return EVENT_NOT_HANDLED;
        #endif
        if (! arg) {  // first frame only, to allow thermal regulation to work
            #ifdef USE_2C_STYLE_CONFIG
            uint8_t tl = style_2c ? MAX_LEVEL : turbo_level;
            set_level_and_therm_target(tl);
            #else
            set_level_and_therm_target(turbo_level);
            #endif
        }
        return EVENT_HANDLED;
    }
    else if ((event == EV_click3_hold_release)
            #ifdef USE_CHANNEL_MODE_ARGS
            || (event == EV_click4_hold_release)
            #endif
        ) {
        #ifdef USE_CHANNEL_MODE_ARGS
            // ramp tint if tint exists in this mode
            if ((event == EV_click3_hold_release)
                && (channel_has_args(channel_mode)))
                return EVENT_NOT_HANDLED;
        #endif
        set_level_and_therm_target(memorized_level);
        return EVENT_HANDLED;
    }

    #ifdef USE_MOMENTARY_MODE
    // 5 clicks: shortcut to momentary mode
    else if (event == EV_5clicks) {
        memorized_level = actual_level;  // allow turbo in momentary mode
        set_level(0);
        set_state(momentary_state, 0);
        return EVENT_HANDLED;
    }
    #endif

    #ifdef USE_RAMP_CONFIG
    // 7H: configure this ramp mode
    else if (event == EV_click7_hold) {
        push_state(ramp_config_state, 0);
        return EVENT_HANDLED;
    }
    #endif

    #ifdef USE_MANUAL_MEMORY
    else if (event == EV_10clicks) {
        // turn on manual memory and save current brightness
        manual_memory_save();
        save_config();
        blink_once();
        return EVENT_HANDLED;
    }
    else if (event == EV_click10_hold) {
        #ifdef USE_RAMP_EXTRAS_CONFIG
        // let user configure a bunch of extra ramp options
        push_state(ramp_extras_config_state, 0);
        #else  // manual mem, but no timer
        // turn off manual memory; go back to automatic
        if (0 == arg) {
            cfg.manual_memory = 0;
            save_config();
            blink_once();
        }
        #endif
        return EVENT_HANDLED;
    }
    #endif  // ifdef USE_MANUAL_MEMORY

    return EVENT_NOT_HANDLED;
}


#ifdef USE_RAMP_CONFIG
void ramp_config_save(uint8_t step, uint8_t value) {

    // 0 = smooth ramp, 1 = stepped ramp, 2 = simple UI's ramp
    uint8_t style = cfg.ramp_style;
    #ifdef USE_SIMPLE_UI
    if (current_state == simple_ui_config_state)  style = 2;
    #endif

    #if defined(USE_SIMPLE_UI) && defined(USE_2C_STYLE_CONFIG)
    // simple UI config is weird...
    // has some ramp extras after floor/ceil/steps
    if (4 == step) {
        cfg.ramp_2c_style_simple = value;
    }
    else
    #endif

    // save adjusted value to the correct slot
    if (value) {
        // ceiling value is inverted
        if (step == 2) value = MAX_LEVEL + 1 - value;

        // which option are we configuring?
        // TODO? maybe rearrange definitions to avoid the need for this table
        //       (move all ramp values into a single array?)
        uint8_t *steps[] = { cfg.ramp_floors, cfg.ramp_ceils, cfg.ramp_stepss };
        uint8_t *option;
        option = steps[step-1];
        option[style] = value;
    }
}

uint8_t ramp_config_state(Event event, uint16_t arg) {
    #ifdef USE_RAMP_SPEED_CONFIG
    const uint8_t num_config_steps = 3;
    #else
    uint8_t num_config_steps = cfg.ramp_style + 2;
    #endif
    return config_state_base(event, arg,
                             num_config_steps, ramp_config_save);
}

#ifdef USE_SIMPLE_UI
uint8_t simple_ui_config_state(Event event, uint16_t arg) {
    #if defined(USE_2C_STYLE_CONFIG)
    #define SIMPLE_UI_NUM_MENU_ITEMS 4
    #else
    #define SIMPLE_UI_NUM_MENU_ITEMS 3
    #endif
    return config_state_base(event, arg,
                             SIMPLE_UI_NUM_MENU_ITEMS,
                             ramp_config_save);
}
#endif
#endif  // #ifdef USE_RAMP_CONFIG

#ifdef USE_RAMP_EXTRAS_CONFIG
void ramp_extras_config_save(uint8_t step, uint8_t value) {
    if (0) {}

    #ifdef USE_MANUAL_MEMORY
    // item 1: disable manual memory, go back to automatic
    else if (manual_memory_config_step == step) {
        cfg.manual_memory = 0;
    }

    #ifdef USE_MANUAL_MEMORY_TIMER
    // item 2: set manual memory timer duration
    // FIXME: should be limited to (65535 / SLEEP_TICKS_PER_MINUTE)
    //   to avoid overflows or impossibly long timeouts
    //   (by default, the effective limit is 145, but it allows up to 255)
    else if (manual_memory_timer_config_step == step) {
        cfg.manual_memory_timer = value;
    }
    #endif
    #endif  // ifdef USE_MANUAL_MEMORY

    #ifdef USE_RAMP_AFTER_MOON_CONFIG
    // item 3: ramp up after hold-from-off for moon?
    // 0 = yes, ramp after moon
    // 1+ = no, stay at moon
    else if (dont_ramp_after_moon_config_step == step) {
        cfg.dont_ramp_after_moon = value;
    }
    #endif

    #ifdef USE_2C_STYLE_CONFIG
    // item 4: Anduril 1 2C turbo, or Anduril 2 2C ceiling?
    // 1 = Anduril 1, 2C turbo
    // 2+ = Anduril 2, 2C ceiling
    else if (ramp_2c_style_config_step == step) {
        cfg.ramp_2c_style = value;
    }
    #endif

    #ifdef USE_SMOOTH_STEPS
    else if (smooth_steps_style_config_step == step) {
        cfg.smooth_steps_style = value;
    }
    #endif
}

uint8_t ramp_extras_config_state(Event event, uint16_t arg) {
    return config_state_base(event, arg,
        ramp_extras_config_num_steps - 1,
        ramp_extras_config_save);
}
#endif

#ifdef USE_GLOBALS_CONFIG
void globals_config_save(uint8_t step, uint8_t value) {
    if (0) {}
    #if defined(USE_CHANNEL_MODE_ARGS) && defined(USE_STEPPED_TINT_RAMPING)
    else if (step == tint_style_config_step) { cfg.tint_ramp_style = value; }
    #endif
    #ifdef USE_JUMP_START
    else if (step == jump_start_config_step) { cfg.jump_start_level = value; }
    #endif
}

uint8_t globals_config_state(Event event, uint16_t arg) {
    return config_state_base(event, arg,
        globals_config_num_steps - 1,
        globals_config_save);
}
#endif

uint8_t nearest_level(int16_t target) {
    if (target < 1) return 1;
    if (target > 150) return 150;
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

void set_level_and_therm_target(uint8_t level) {
    target_level = level;
    set_level(level);
}

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

