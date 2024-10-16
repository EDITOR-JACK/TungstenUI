#pragma once
#include "anduril/aux-leds.h"

// mode: 0bPPPPCCCC where PPPP is the pattern and CCCC is the color
// arg: time slice number
void rgb_led_update(uint8_t mode, uint16_t arg) {
    static uint8_t frame = 0;  // track state of animation mode

    // turn off aux LEDs when battery is empty
    // (but if voltage==0, that means we just booted and don't know yet)
    uint8_t volts = voltage;  // save a few bytes by caching volatile value
    if ((volts) && (volts < VOLTAGE_LOW)) {
        rgb_led_set(0);
        #ifdef USE_BUTTON_LED
        button_led_set(0);
        #endif
        return;
    }

    uint8_t pattern = (mode>>4);  // off, low, high, blinking, ... more?
    uint8_t color = mode & 0x0f;

    const uint8_t *colors = rgb_led_colors + 1;
    uint8_t actual_color = 0;
    if (color < 7) {  // normal color
        actual_color = pgm_read_byte(colors + color);
    }

    // pick a brightness from the animation sequence
    if (pattern == 3) {
        static const uint8_t animation[] = {1, 2, 2, 1, 
                                            0, 0, 0, 0,  
                                            0, 0, 0, 0};
        frame = (frame + 1) % sizeof(animation);
        pattern = animation[frame];
    }
    uint8_t result;
    #ifdef USE_BUTTON_LED
    uint8_t button_led_result;
    #endif
    switch (pattern) {
        case 0:  // off
            result = 0;
            #ifdef USE_BUTTON_LED
            button_led_result = 0;
            #endif
            break;
        case 1:  // low
            result = actual_color;
            #ifdef USE_BUTTON_LED
            button_led_result = 1;
            #endif
            break;
        default:  // high
            result = (actual_color << 1);
            #ifdef USE_BUTTON_LED
            button_led_result = 2;
            #endif
            break;
    }
    rgb_led_set(result);
    #ifdef USE_BUTTON_LED
    button_led_set(button_led_result);
    #endif
}