# TungstenUI Flashlight Firmware

Based on Anduril - using FSM toolkit to design a minimal, functional flashlight UI for EDC.

## Features

### 3 States (LOCKOUT/OFF/ON)

- OFF 1C -> AUX Toggle
- OFF 1H -> Set Ramp Level
- OFF 2C -> Max Level
- OFF 2H -> Momentary Max Level
- OFF 4C -> Battery Voltage Check
- OFF 5C -> Temperature Check

- ON 1C -> OFF
- ON 1H/2H -> Ramp Brightness Up/Down
- ON 2C -> Max Level (Turbo)

- TEMPCHECK 5H -> Calibrate Temperature

## TODO

- Low voltage indicator
- General cleanup & format this doc better (read up on Markdown)
- Move key config to central location (AUX modes, etc)
- Info (battery/temp) blinkout to use AUX leds
- Beacon mode (using AUX leds?)
- Custom AUX PCB w/ amber leds