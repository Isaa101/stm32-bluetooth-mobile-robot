# STM32 Bluetooth Mobile Robot

![Robot](media/robot.jpg)

A two-wheel mobile robot built around an STM32L152 microcontroller, combining Bluetooth teleoperation, ultrasonic ranging, motor control and proximity feedback on physical hardware.

## Demo

The robot can be remotely controlled from a mobile device using five commands:

- Forward
- Backward
- Left
- Right
- Stop

[Watch the full hardware demo](media/demo.mp4)

## System Overview

The STM32 integrates several peripherals and external devices:

- Bluetooth communication through UART
- Ultrasonic distance measurement using timer capture
- Potentiometer acquisition using ADC
- Two-wheel motor actuation through GPIO
- Distance-dependent buzzer feedback
- Periodic sensor acquisition using timers
- Interrupt-based peripheral handling

## Tech Stack

**STM32L152 · Embedded C · STM32 HAL · UART · ADC · Timers · GPIO · Interrupts · Bluetooth · Ultrasonic sensing**

## Firmware Architecture

The firmware uses peripheral callbacks to handle asynchronous events:

- UART receive callback for Bluetooth commands
- ADC conversion-complete callback
- Timer callbacks for periodic measurements
- Timer input capture for ultrasonic echo timing

Bluetooth commands are translated into differential-drive motor actions, while ultrasonic measurements are used to generate proximity-dependent acoustic feedback.

## Firmware

The main firmware representation is available at:

`firmware/reconstructed_main.c`

### Reconstruction note

The exact final source file used in the original project was not preserved.

The firmware published here was reconstructed from the original final presentation, preserved development code and the recorded working hardware demo. It represents the documented architecture and behaviour rather than the exact historical source file.

See [`docs/reconstruction_notes.md`](docs/reconstruction_notes.md) for details.

## Results

The final physical robot successfully demonstrated:

- Wireless Bluetooth teleoperation
- Forward, backward and turning motion
- Ultrasonic sensing while the robot was operating
- Proximity-dependent acoustic feedback
- Simultaneous integration of sensing, communication and actuation on the STM32

## Team

Three-person collaborative project.

Firmware development, peripheral configuration, hardware integration, testing and debugging were carried out collaboratively rather than dividing the project into isolated individual subsystems.

## What I Learned

This project provided practical experience integrating sensors, actuators and communication peripherals on a physical robotic platform, with particular emphasis on embedded firmware, interrupt-driven peripherals and hardware/software debugging.
