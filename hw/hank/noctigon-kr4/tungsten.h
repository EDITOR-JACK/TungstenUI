// Noctigon KR4 config options for Anduril
// (and Emisar D4v2.5, which uses KR4 driver plus a button LED)
// Copyright (C) 2020-2023 Selene ToyKeeper
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "hank/noctigon-kr4/hwdef.h"
#include "hank/anduril.h"

// brightness w/ SST-20 4000K LEDs:
// 0/1023: 0.35 lm
// 1/1023: 2.56 lm
// max regulated: 1740 lm
// FET: ~3700 lm
#define RAMP_SIZE 150

// nice low lows, but might have visible ripple on some lights:
// maxreg at 130, dynamic PWM: level_calc.py 5.01 2 149 7135 1 0.3 1740 FET 1 10 3190 --pwm dyn:64:16384:255
// (plus one extra level at the beginning for moon)
#define PWM1_LEVELS  0,1,1,2,2,3,4,5,6,7,8,9,11,12,14,16,17,19,22,24,26,29,31,34,37,40,43,46,49,53,56,60,63,67,71,74,78,82,86,89,93,96,99,103,105,108,110,112,114,115,116,116,115,114,112,109,106,101,95,89,81,71,60,48,34,19,20,21,22,23,24,26,27,28,30,31,32,34,36,37,39,41,43,45,47,49,51,53,56,58,61,63,66,69,72,75,78,81,84,88,91,95,99,103,107,111,115,119,124,129,133,138,143,149,154,159,165,171,177,183,189,196,203,210,217,224,231,239,247,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0
#define PWM2_LEVELS  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,20,30,41,52,63,75,87,99,112,125,138,151,165,179,194,208,224,239,255
#define PWM_TOPS     16383,16383,11750,14690,9183,12439,13615,13955,13877,13560,13093,12529,13291,12513,12756,12769,11893,11747,12085,11725,11329,11316,10851,10713,10518,10282,10016,9729,9428,9298,8971,8794,8459,8257,8043,7715,7497,7275,7052,6753,6538,6260,5994,5798,5501,5271,5006,4758,4525,4268,4030,3775,3508,3263,3010,2752,2517,2256,1998,1763,1512,1249,994,749,497,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
#define MIN_THERM_STEPDOWN 66  // should be above highest dyn_pwm level

// less ripple, but lows are a bit higher than ideal:
// maxreg at 130, dynamic PWM: level_calc.py 5.01 2 149 7135 1 0.3 1740 FET 1 10 3190 --pwm dyn:64:4096:255
// (plus one extra level at the beginning for moon)
//#define PWM1_LEVELS 0,1,1,1,1,1,1,2,2,2,2,3,3,3,4,4,5,5,6,6,7,8,8,9,10,11,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,32,33,33,34,34,34,34,34,34,33,32,31,30,28,26,24,21,19,20,21,22,23,24,26,27,28,30,31,32,34,36,37,39,41,43,45,47,49,51,53,56,58,61,63,66,69,72,75,78,81,84,88,91,95,99,103,107,111,115,119,124,129,133,138,143,149,154,159,165,171,177,183,189,196,203,210,217,224,231,239,247,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0
//#define PWM2_LEVELS 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,20,30,41,52,63,75,87,99,112,125,138,151,165,179,194,208,224,239,255
//#define PWM_TOPS 4095,4095,3760,3403,3020,2611,2176,3582,3062,2515,1940,3221,2761,2283,2998,2584,3004,2631,2899,2555,2735,2836,2538,2606,2636,2638,2387,2382,2361,2328,2286,2238,2185,2129,2070,2010,1949,1887,1826,1766,1706,1648,1591,1536,1482,1429,1379,1329,1242,1199,1122,1084,1016,953,895,842,791,723,659,602,549,482,422,367,302,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255

#define MAX_1x7135 130
#define DEFAULT_LEVEL 50
#define HALFSPEED_LEVEL 12
#define QUARTERSPEED_LEVEL 4

#define RAMP_SMOOTH_FLOOR 11  // low levels may be unreliable
#define RAMP_SMOOTH_CEIL  130
// 11 30 [50] 70 90 110 [130]
#define RAMP_DISCRETE_FLOOR 11
#define RAMP_DISCRETE_CEIL  RAMP_SMOOTH_CEIL
#define RAMP_DISCRETE_STEPS 7

// safe limit ~30% power / ~1300 lm (can sustain 900 lm)
#define SIMPLE_UI_FLOOR RAMP_DISCRETE_FLOOR
#define SIMPLE_UI_CEIL 120
#define SIMPLE_UI_STEPS 5

// stop panicking at ~1300 lm
#define THERM_FASTER_LEVEL 120

#define THERM_CAL_OFFSET 5

#undef DEFAULT_THERM_CEIL
#define DEFAULT_THERM_CEIL 60

// the power regulator is a bit slow, so push it harder for a quick response from off
#define DEFAULT_JUMP_START_LEVEL 21
#define BLINK_BRIGHTNESS DEFAULT_LEVEL
#define BLINK_ONCE_TIME 12

// show each channel while it scroll by in the menu
#define USE_CONFIG_COLORS

// there is usually no lighted button, so
// blink numbers on the main LEDs by default (but allow user to change it)
#define DEFAULT_BLINK_CHANNEL  CM_MAIN

// slow down party strobe; this driver can't pulse for 1ms or less
// (only needed on no-FET build)
//#define PARTY_STROBE_ONTIME 2

// use aux red + aux blue for police strobe
#define USE_POLICE_COLOR_STROBE_MODE
#define POLICE_STROBE_USES_AUX
#define POLICE_COLOR_STROBE_CH1        CM_AUXRED
#define POLICE_COLOR_STROBE_CH2        CM_AUXBLU

// the default of 26 looks a bit rough, so increase it to make it smoother
#define CANDLE_AMPLITUDE 33

// don't blink while ramping
#ifdef BLINK_AT_RAMP_MIDDLE
#undef BLINK_AT_RAMP_MIDDLE
#endif

// can't reset the normal way because power is connected before the button
#define USE_SOFT_FACTORY_RESET