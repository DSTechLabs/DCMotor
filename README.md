
# DC Motor Control

This is a single C++ class that provides full control of a non-encoded DC motor.

This class is designed to be included in the firmware of MCU's such as Arduino and ESP32.  It will interface with most DC PWM motor drivers.

## Features:
- Can be used by directly calling class methods -OR- sending text commands
- Optionally uses hard limits by specifying one or two limit switch pins
- Speed is specified as a percentage (0 - 100)
- Direction is specified as CW or CCW
- Provides smooth ramp-up / ramp-down motion
- Allows for 10 different ramp slopes
- Allows for continuous motion with any speed change (including reversing direction)
- Includes Emergency Stop (E-Stop)

---
### If you use this code, please <a href="https://buymeacoffee.com/dstechlabs" target="_blank">Buy Me A Coffee ☕</a>

