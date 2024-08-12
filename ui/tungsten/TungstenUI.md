# TungstenUI Flashlight Firmware

Based on Anduril - using FSM toolkit to design a minimal, functional flashlight UI for EDC.

## Features

### 3 States (OFF/STANDBY/RAMP)

- 1C STANDBY -> Memory level
- 1H STANDBY -> Ramp from min level
- 2C STANDBY -> Max level
(+1 Click from OFF)
- 2C OFF -> Memory level
- 2H OFF -> Ramp from min level
- 3C OFF -> Max level

- 1H OFF -> Momentary Min level
- 3C STANDBY -> OFF
- 4C STANDBY -> Voltage Readout
- 2C Voltage Readout -> Temperature Readout

## TODO

- Blink Red AUX when battery low
- Blink Green AUX when powered on (boot)
- Info (battery/temp) blinkout to use AUX leds
- Breathing AUX leds (PWM)?
- AUX LED lower voltage??
- AUX leds come on immediately as main emitter fades off?
- Smoother fading between modes/levels?
- Slightly slower ramping speed
- Beacon mode?
- Temp sensor calibration
- Beacon/strobe (using AUX leds?)
- Momentary/tactical mode
- Custom AUX PCB w/ amber leds