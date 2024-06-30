# Mr Bones

Power Wheels motor controller with BLE remote control, powered by two Particle Photon 2.

## motor-driver

Controls the motors and receives controls over BLE.

Hardware configuration:

- Direction PWM: D1
- Direction left (forward): D2
- Direction right (reverse): D3
- Wheels PWM: A5
- Wheels direction: S4

## ble-controller

Reads joysticks and sends controls over BLE.

Hardware configuration:

- Joystick GND: A0
- Joystick 3.3V: A1
- Joystick X: A2
- Joystick Y: A5
- Joystick switch: S4
