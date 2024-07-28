/*
 * Project myProject
 * Author: Your Name
 * Date:
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"
#include "gamepad-ble.h"
#include "lut.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_NONE, {{"app", LOG_LEVEL_INFO}});

Gamepad gamepad;

const auto DIRECTION_PWM = A2;
const auto DIRECTION_LEFT = A0;
const auto DIRECTION_RIGHT = A1;
const auto DIRECTION_ENCODER1 = D2;
const auto DIRECTION_ENCODER2 = D3;

const auto WHEELS_PWM = A5;
const auto WHEELS_REVERSE = S4;

// Turn off outputs after this amount of time without BLE controls update
const auto SAFETY_SHUTOFF_TIME = 1000;

// Throttle map between input 0:1024 to motor 0:255
std::vector<int16_t> throttleRanges = {0, 10, 500, 750, 1000, 1024};
std::vector<int16_t> throttleValues = {0, 0, 40, 60, 80, 150};

int16_t turn = 0;
int32_t throttle = 0;
bool safetyShutoff = true;
int16_t wt = 0;

int32_t turnAngle = 0;

Thread *hardwareThread;

void runHardware();

void setup()
{
  Particle.function("t", [&](String arg) {
    throttleValues[1] = arg.toInt();
    return throttleValues[1];
  });

  attachInterrupt(DIRECTION_ENCODER1, 
  

  hardwareThread = new Thread("hardware", runHardware);
  gamepad.begin();
}

void loop()
{
  gamepad.process();
  safetyShutoff = !gamepad.valid();

  if (gamepad.valid())
  {
    Log.info("x=%5d turn=%3d t=%4ld wt=%4d", gamepad.x1, turn, throttle, wt);
    delay(100);
  }
}

// multiple the input by itself a number of times to make controls non-linear
int32_t cube(int32_t v, int inputRange)
{
  if (v == (2 << inputRange) - 1)
  {
    return v;
  }
  for (auto i = 0; i < 3; i++)
  {
    v = (v * abs(v)) >> inputRange;
  }
  return v;
}

// Function to apply slew rate limitation
int applySlewRate(int actual, int target, int maxChange)
{
  // Calculate the difference between the target power and the current power
  int difference = target - actual;

  // Limit the change to the maximum allowed change per loop
  if (difference > maxChange)
  {
    difference = maxChange;
  }
  else if (difference < -maxChange)
  {
    difference = -maxChange;
  }

  // Apply the limited change
  actual += difference;

  // Clamp the result to the valid motor power range [-255, 255]
  actual = std::max(-255, std::min(255, actual));

  return actual;
}

void runHardware()
{
  system_tick_t lastSlew = 0;
  int16_t w = 0;

  pinMode(DIRECTION_PWM, OUTPUT);
  pinMode(DIRECTION_LEFT, OUTPUT);
  pinMode(DIRECTION_RIGHT, OUTPUT);

  pinMode(WHEELS_PWM, OUTPUT);
  pinMode(WHEELS_REVERSE, OUTPUT);

  while (true)
  {
    // direction PWM
    turn = (int16_t)-cube((int32_t)gamepad.x1 - 32768, 15) / (32768 / 256);

    // wheels PWM
    throttle = (int32_t)gamepad.leftTrigger - gamepad.rightTrigger;
    wt = interpolate(throttleRanges, throttleValues, (int16_t)abs(throttle)) * (throttle > 0 ? 1 : -1);

    if (millis() - lastSlew > 10)
    {
      lastSlew = millis();
      w = applySlewRate(w, wt, 10);
    }

    if (safetyShutoff)
    {
      digitalWrite(DIRECTION_LEFT, LOW);
      digitalWrite(DIRECTION_RIGHT, LOW);
      analogWrite(DIRECTION_PWM, 0, 10000);

      digitalWrite(WHEELS_REVERSE, LOW);
      analogWrite(WHEELS_PWM, 0, 10000);
    }
    else
    {
      digitalWrite(DIRECTION_LEFT, turn > 0 ? HIGH : LOW);
      digitalWrite(DIRECTION_RIGHT, turn > 0 ? LOW : (turn == 0 ? LOW : HIGH));
      analogWrite(DIRECTION_PWM, abs(turn), 10000);

      digitalWrite(WHEELS_REVERSE, w > 0 ? LOW : HIGH);
      analogWrite(WHEELS_PWM, abs(w), 10000);
    }
  }
}
