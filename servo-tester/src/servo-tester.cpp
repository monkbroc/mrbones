/*
 * Project myProject
 * Author: Your Name
 * Date:
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"
#include "gamepad-ble.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_NONE, {{"app", LOG_LEVEL_INFO}});

Gamepad gamepad;

const auto SERVO_GND = A0;
const auto SERVO_POWER = A1;
const auto SERVO_OUT = A2;

Servo mouth;

retained float mouthPos = 0;

Thread *hardwareThread;

void runHardware();

void setup()
{
  hardwareThread = new Thread("hardware", runHardware);
  gamepad.begin();
}

void loop()
{
  gamepad.process();
}

void runHardware()
{
  pinMode(SERVO_GND, OUTPUT);
  pinMode(SERVO_POWER, OUTPUT);

  digitalWrite(SERVO_GND, LOW);
  digitalWrite(SERVO_POWER, HIGH);

  mouth.attach(SERVO_OUT);

  system_tick_t lastUpdate = 0;
  while (true)
  {
    if (millis() - lastUpdate > 10) {
      float delta = -((int32_t)gamepad.y1 - 32768) / 32768.0 * 2;
      if (abs(delta) > 0.2) {
        mouthPos += delta;
      }
      lastUpdate = millis();
    }
    if (mouthPos < 0) {
      mouthPos = 0;
    } else if (mouthPos > 180) {
      mouthPos = 180;
    }

    mouth.write((int)mouthPos);
  }
}