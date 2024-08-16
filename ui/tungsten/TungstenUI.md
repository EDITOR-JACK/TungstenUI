# TungstenUI Flashlight Firmware

Based on Anduril - using FSM toolkit to design a minimal, functional flashlight UI for EDC.

## Features

### 3 States (LOCKOUT/OFF/ON)

- LOCKOUT 1H -> Momentary Low
- LOCKOUT 1C -> Low
- LOCKOUT 2C -> Set Max Level
- LOCKOUT 2H -> Set New Level

- OFF 1C -> Set Level
- OFF 1H -> Momentary Set Level
- OFF 2C -> Max Level (Turbo)
- OFF 2H -> Momentary Turbo
- OFF 3C -> Lockout Manually
- OFF 4C -> Battery Voltage Check
- OFF 5C -> Temperature Check

- ON 1C -> OFF
- ON 1H/2H -> Ramp Brightness Up/Down
- ON 2C -> Max Level (turbo)

## TODO

- Info (battery/temp) blinkout to use AUX leds
- Beacon mode (using AUX leds?)
- Custom AUX PCB w/ amber leds