# TungstenUI Flashlight Firmware

Based on Anduril - using FSM toolkit to design a minimal, functional flashlight UI for EDC.

## Features

### 3 States (LOCKOUT/OFF/ON)

LOCKOUT 1H -> Momentary Low
LOCKOUT 2C -> Set Max Level
LOCKOUT 2H -> Set New Level

OFF 1C -> Max (or Set) Level
OFF 2C -> Max Level (turbo)
OFF 3C -> Lockout Manually
OFF 4C -> Battery Voltage Check (2C -> Temp Check)

ON 1C -> OFF
ON 1H/2H -> Ramp Brightness Up/Down
ON 2C -> Max Level (turbo)

## TODO

- Blink Red AUX when battery low
- Blink Green AUX when powered on (boot)
- Info (battery/temp) blinkout to use AUX leds
- Breathing AUX leds (PWM)?
- AUX LED lower voltage??
- AUX leds come on immediately as main emitter fades off?
- Beacon mode (using AUX leds?)
- Momentary from OFF (use memorized level & temperature regulation)
- Custom AUX PCB w/ amber leds