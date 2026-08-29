# Firmware reconstruction assumptions

This firmware is a reconstruction of the final robot implementation.

## Verified from original project material

- STM32L152 microcontroller
- Bluetooth teleoperation using UART
- Commands: stop, forward, backward, left and right
- Ultrasonic distance measurement using timer input capture
- ADC acquisition from a potentiometer
- Periodic measurements approximately every 300 ms
- Distance-dependent buzzer behaviour
- Interrupt-based UART, ADC and timer handling

## Not yet verified exactly

- Complete final GPIO pin assignment
- Exact motor driver pin mapping
- Exact constants used in every distance/ADC threshold
- Complete original source-code formatting and variable names