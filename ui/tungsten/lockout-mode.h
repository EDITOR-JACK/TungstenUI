#pragma once

uint8_t lockout_state(Event event, uint16_t arg);

#define DEFAULT_AUTOLOCK_TIME 1 // autolock time in minutes, 0 = disabled