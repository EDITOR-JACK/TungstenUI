// Hank Emisar/Noctigon Lume-X1-C config options for TungstenUI
// Copyright (C) 2018-2026 Selene ToyKeeper, Loneoceans
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// For flashlights using the Loneoceans Lume-X1-C boost driver (AVR32DD20)
// - Same firmware for 6V, 9V, or 12V configs

// same as loneoceans lume-x1-c but with Hank-specific defaults
#include "loneoceans/lume-x1-c/anduril.h"
#include "hank/anduril.h"

#undef DEFAULT_THERM_CEIL
#define DEFAULT_THERM_CEIL 50

// disable beacontower mode
#ifdef USE_BEACONTOWER_MODE
#undef USE_BEACONTOWER_MODE
#endif

// set smooth ramping by default
#ifdef RAMP_STYLE
#undef RAMP_STYLE
#endif

// reset to anduril default number of steps
#ifdef RAMP_DISCRETE_STEPS
#undef RAMP_DISCRETE_STEPS
#endif
#ifdef SIMPLE_UI_STEPS
#undef SIMPLE_UI_STEPS
#endif

#ifdef USE_EXTRA_BATTCHECK_DIGIT
#undef USE_EXTRA_BATTCHECK_DIGIT
#endif

// default aux LED config: off
#ifdef RGB_LED_OFF_DEFAULT
#undef RGB_LED_OFF_DEFAULT
#define RGB_LED_OFF_DEFAULT     0x00
#endif

// Preset levels for Moonlight, Low, High
uint8_t LVLS[3] = { 5, 40, 90};