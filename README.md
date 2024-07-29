# Mr Bones

Power Wheels motor controller with Xbox BLE gamepad control, powered by the Particle Photon 2.

## Hardware

- [MLToys motor and gearboxes](https://www.mltoys.com/collections/power-wheels-jeep-wranglers/products/stage-ii-motors-gearboxes-for-jeep-wrangler)
- [REV Core Hex Motor](https://www.revrobotics.com/rev-41-1300/) for turning the steering column.
- [L298 7A Dual H-Bridge Motor Driver](https://www.amazon.com/dp/B0C73DC9CZ) for the wheel motors.
- [PN00218-CYT13 Cytron Motor Driver](https://www.amazon.com/dp/B07V8G7J3W) for the steering motor.
- [DC24V to 12V 5A Converter](https://www.amazon.com/dp/B09XWG81FZ)
- [24V Automatic Reset Circuit Breaker Circuit Breaker](https://www.amazon.com/dp/B08FB7YJZW)
- [6S LiPo battery](https://www.amazon.com/gp/product/B0B1D9GQ5M)

## motor-driver firmware

Controls the motors and receives controls from the Xbox controller.

Hardware configuration:

- Direction left (forward): A0
- Direction right (reverse): A1
- Direction PWM: A2
- Direction encoder 1 (yellow): D2
- Direction encoder 2 (orange): D3
- Wheels PWM: A5
- Wheels reverse: S4

12V to 5V buck converter
- Red: 12V (DC24V to 12V)
- Black: Ground (DC24V to 12V)
- Blue: Ground
- Orange: VUSB