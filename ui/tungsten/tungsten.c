/********* load up MCU info, like ROM size and such *********/
#include "arch/mcu.h"

/********* User-configurable options *********/
#include "anduril/config-default.h"
#ifdef USE_EXTRA_BATTCHECK_DIGIT
    #undef USE_EXTRA_BATTCHECK_DIGIT
#endif
#define DEFAULT_AUTOLOCK_TIME 5
#define HOLD_TIMEOUT 15
#define RELEASE_TIMEOUT 15

/********* specific settings for known driver types *********/

#include incfile(CFG_H)

#ifdef HWDEF_H
#include incfile(HWDEF_H)
#endif

/********* Include headers which need to be before FSM *********/

// enable FSM features needed by basic ramping functions
#include "anduril/ramp-mode-fsm.h"

#ifdef USE_FACTORY_RESET
#include "anduril/factory-reset-fsm.h"
#endif

#ifdef USE_BATTCHECK_MODE
#include "anduril/battcheck-mode-fsm.h"
#endif

#ifdef USE_LOCKOUT_MODE
#include "anduril/lockout-mode-fsm.h"
#endif

// enable FSM features needed by strobe modes
#include "anduril/strobe-modes-fsm.h"

// figure out how many bytes of eeprom are needed,
// based on which UI features are enabled
// (include this one last)
#include "anduril/load-save-config-fsm.h"


/********* bring in FSM / SpaghettiMonster *********/
#define USE_IDLE_MODE  // reduce power use while awake and no tasks are pending

#include "fsm/spaghetti-monster.h"

/********* does this build target have special code to include? *********/
#ifdef CFG_C
#include incfile(CFG_C)
#endif

#ifdef HWDEF_C
#include incfile(HWDEF_C)
#endif


/********* Include all the regular app headers *********/

#include "anduril/off-mode.h"
#include "anduril/ramp-mode.h"
#include "anduril/config-mode.h"
#include "anduril/aux-leds.h"
#include "anduril/misc.h"

#ifdef USE_SUNSET_TIMER
#include "anduril/sunset-timer.h"
#endif

#ifdef USE_VERSION_CHECK
#include "anduril/version-check-mode.h"
#endif

#ifdef USE_BATTCHECK_MODE
#include "anduril/battcheck-mode.h"
#endif

#ifdef USE_BEACON_MODE
#include "anduril/beacon-mode.h"
#endif

#ifdef USE_THERMAL_REGULATION
#include "anduril/tempcheck-mode.h"
#endif

#ifdef USE_LOCKOUT_MODE
#include "anduril/lockout-mode.h"
#endif

#if (defined(USE_MOMENTARY_MODE) || defined(USE_TACTICAL_MODE))
#include "anduril/momentary-mode.h"
#endif

#ifdef USE_TACTICAL_MODE
#include "anduril/tactical-mode.h"
#endif

// allow the channel mode handler even when only 1 mode
// (so a tint ramp light could still use 3H even if there's no other mode)
#if defined(USE_CHANNEL_MODES)
#include "anduril/channel-modes.h"
#endif

#ifdef USE_FACTORY_RESET
#include "anduril/factory-reset.h"
#endif

// this one detects its own enable/disable settings
#include "anduril/strobe-modes.h"

#ifdef USE_SOS_MODE
#include "anduril/sos-mode.h"
#endif

#ifdef USE_SMOOTH_STEPS
#include "anduril/smooth-steps.h"
#endif

// this should be last, so other headers have a chance to declare values
#include "anduril/load-save-config.h"


/********* Include all the app logic source files *********/
// (is a bit weird to do things this way,
//  but it saves a lot of space by letting us use the -fwhole-program flag)

#include "tungsten/off-mode.c"
#include "tungsten/ramp-mode.c"
#include "anduril/load-save-config.c"
#include "anduril/config-mode.c"
#include "tungsten/aux-leds.c"
#include "anduril/misc.c"

#ifdef USE_SUNSET_TIMER
#include "anduril/sunset-timer.c"
#endif

#ifdef USE_VERSION_CHECK
#include "anduril/version-check-mode.c"
#endif

#ifdef USE_BATTCHECK_MODE
#include "tungsten/battcheck-mode.c"
#endif

#ifdef USE_BEACON_MODE
#include "anduril/beacon-mode.c"
#endif

#ifdef USE_THERMAL_REGULATION
#include "tungsten/tempcheck-mode.c"
#endif

#ifdef USE_LOCKOUT_MODE
#include "tungsten/lockout-mode.c"
#endif

#if (defined(USE_MOMENTARY_MODE) || defined(USE_TACTICAL_MODE))
#include "anduril/momentary-mode.c"
#endif

#ifdef USE_TACTICAL_MODE
#include "anduril/tactical-mode.c"
#endif

#if defined(USE_CHANNEL_MODES)
#include "anduril/channel-modes.c"
#endif

#ifdef USE_FACTORY_RESET
#include "anduril/factory-reset.c"
#endif

#ifdef USE_STROBE_STATE
#include "anduril/strobe-modes.c"
#endif

#ifdef USE_SOS_MODE
#include "anduril/sos-mode.c"
#endif

#ifdef USE_SMOOTH_STEPS
#include "anduril/smooth-steps.c"
#endif


// runs one time at boot, when power is connected
void setup() {

    #ifndef START_AT_MEMORIZED_LEVEL

        // regular e-switch light, no hard clicky power button

        #ifdef USE_WEAK_BATTERY_PROTECTION
            // try to measure the battery strength
            // (must be done *before* factory reset,
            //  because reset tries to use full power,
            //  and a weak battery can't do that)
            detect_weak_battery();
        #else
            // blink at power-on to let user know power is connected
            blink_once();
        #endif

        load_config();

        push_state(tempcheck_state, 0);

    #else  // if START_AT_MEMORIZED_LEVEL

        // dual switch: e-switch + power clicky
        // power clicky acts as a momentary mode
        load_config();

        #if defined(USE_CHANNEL_MODES)
        // add channel mode functions underneath every other state
        push_state(channel_mode_state, 0);
        #endif

        if (button_is_pressed())
            // hold button to go to moon
            push_state(steady_state, 1);
        else
            // otherwise use memory
            push_state(steady_state, memorized_level);

    #endif  // ifdef START_AT_MEMORIZED_LEVEL

}


// runs repeatedly whenever light is "on" (not in standby)
void loop() {

    // "current_state" is volatile, so cache it to reduce code size
    StatePtr state = current_state;

    #ifdef USE_AUX_RGB_LEDS_WHILE_ON
    // display battery charge on RGB button during use
    if (state == steady_state)
        rgb_led_voltage_readout(actual_level > USE_AUX_RGB_LEDS_WHILE_ON);
    #endif

    if (0) {}  // placeholder

    #ifdef USE_VERSION_CHECK
    else if (state == version_check_state) {
        version_check_iter();
    }
    #endif

    #ifdef USE_STROBE_STATE
    else if ((state == strobe_state)
         #if defined(USE_MOMENTARY_MODE) || defined(USE_TACTICAL_MODE)
         // also handle momentary strobes
         || ((0
              #ifdef USE_MOMENTARY_MODE
              || (state == momentary_state)
              #endif
              #ifdef USE_TACTICAL_MODE
              || (state == tactical_state)
              #endif
             )
             && (momentary_mode == 1) && (momentary_active))
         #endif
         ) {
        strobe_state_iter();
    }
    #endif  // #ifdef USE_STROBE_STATE

    #ifdef USE_BORING_STROBE_STATE
    else if (state == boring_strobe_state) {
        boring_strobe_state_iter();
    }
    #endif

    #ifdef USE_BATTCHECK
    else if (state == battcheck_state) {
        nice_delay_ms(500);  // wait a moment for a more accurate reading
        battcheck();
        nice_delay_ms(1000);
    }
    #endif

    #ifdef USE_THERMAL_REGULATION
    else if (state == tempcheck_state) {
        blink_num(temperature);
        nice_delay_ms(1000);
    }
    #endif

    #ifdef USE_BEACON_MODE
    else if (state == beacon_state) {
        beacon_mode_iter();
    }
    #endif

    #if defined(USE_SOS_MODE) && defined(USE_SOS_MODE_IN_BLINKY_GROUP)
    else if (state == sos_state) {
        sos_mode_iter();
    }
    #endif

    #ifdef USE_SMOOTH_STEPS
    else if (cfg.smooth_steps_style && smooth_steps_in_progress) {
        smooth_steps_iter();
    }
    #endif

    #ifdef USE_IDLE_MODE
    else {
        // doze until next clock tick
        idle_mode();
    }
    #endif

}


// instead of handling EV_low_voltage in each mode,
// it's handled globally here to make the code smaller and simpler
void low_voltage() {

    // "current_state" is volatile, so cache it to reduce code size
    StatePtr state = current_state;

    // TODO: turn off aux LED(s) when power is really low

    if (0) {}  // placeholder

    #ifdef USE_STROBE_STATE
    // "step down" from strobe to something low
    else if (state == strobe_state) {
        set_state(steady_state, RAMP_SIZE/6);
    }
    #endif

    // in normal mode, step down or turn off
    else if (state == steady_state) {
        if (actual_level > 1) {
            uint8_t lvl = (actual_level >> 1) + (actual_level >> 2);
            set_level_and_therm_target(lvl);
        }
        else {
            set_state(off_state, 0);
        }
    }
    // all other modes, just turn off when voltage is low
    else {
        set_state(off_state, 0);
    }

}