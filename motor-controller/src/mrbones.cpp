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
std::vector<int16_t> throttleValues = {0,  0,  40,  100,  200,  255};

int16_t TURN_FACTOR = 200;
int16_t TURN_P = 4;

int16_t turn = 0;
int32_t throttle = 0;
bool safetyShutoff = true;
int16_t wt = 0;

int32_t turnAngle = 0;
uint8_t encoderState = 0;
int16_t turnPower = 0;

Thread *hardwareThread;

void runHardware();

void setup()
{
  Particle.function("f", [&](String arg) {
    TURN_FACTOR = arg.toInt();
    return TURN_FACTOR;
  });
  Particle.function("p", [&](String arg) {
    TURN_P = arg.toInt();
    return TURN_P;
  });

  hardwareThread = new Thread("hardware", runHardware);
  gamepad.begin();
}

void loop()
{
  gamepad.process();
  safetyShutoff = !gamepad.valid();

  if (gamepad.valid())
  {
    Log.info("x=%5d turn=%4d angle=%4ld pow=%4d t=%4ld wt=%4d", gamepad.x1, turn, turnAngle, turnPower, throttle, wt);
    delay(100);
  }
}

// multiple the input by itself a number of times to make controls non-linear
float cube(float v)
{
  for (auto i = 0; i < 3; i++)
  {
    v = v * abs(v);
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

uint8_t readEncoder();
void encoderInterrupt();

void runHardware()
{
  system_tick_t lastSlew = 0;
  int16_t w = 0;
  uint8_t oldDpad = 0;

  pinMode(DIRECTION_PWM, OUTPUT);
  pinMode(DIRECTION_LEFT, OUTPUT);
  pinMode(DIRECTION_RIGHT, OUTPUT);
  pinMode(DIRECTION_ENCODER1, INPUT_PULLUP);
  pinMode(DIRECTION_ENCODER2, INPUT_PULLUP);

  pinMode(WHEELS_PWM, OUTPUT);
  pinMode(WHEELS_REVERSE, OUTPUT);

  encoderState = readEncoder();

  attachInterrupt(DIRECTION_ENCODER1, encoderInterrupt, CHANGE);
  attachInterrupt(DIRECTION_ENCODER2, encoderInterrupt, CHANGE);

  while (true)
  {
    // trim the turn angle with the D-pad
    const auto DPAD_LEFT = 7;
    const auto DPAD_RIGHT = 3;
    uint8_t dpad = gamepad.dpad;
    if (oldDpad == 0 && dpad != 0) {
      SINGLE_THREADED_SECTION();
      if (dpad == DPAD_LEFT) {
        turnAngle += 10;
      }
      if (dpad == DPAD_RIGHT) {
        turnAngle -= 10;
      }
    }
    oldDpad = dpad;

    // direction controller
    turn = (uint16_t)(((int32_t)gamepad.x1 - 32768) / TURN_FACTOR);
    int16_t error = turnAngle - turn;
    turnPower = std::min<int16_t>(std::max<int16_t>(error * TURN_P, -255), 255);

    // wheels PWM
    throttle = (int32_t)gamepad.rightTrigger - gamepad.leftTrigger;
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
      digitalWrite(DIRECTION_LEFT, turnPower > 0 ? HIGH : LOW);
      digitalWrite(DIRECTION_RIGHT, turnPower > 0 ? LOW : (turnPower == 0 ? LOW : HIGH));
      analogWrite(DIRECTION_PWM, abs(turnPower), 10000);

      digitalWrite(WHEELS_REVERSE, w > 0 ? LOW : HIGH);
      analogWrite(WHEELS_PWM, abs(w), 10000);
    }
  }
}

uint8_t readEncoder() {
  return ((uint8_t)digitalRead(DIRECTION_ENCODER1) << 1) | ((uint8_t) digitalRead(DIRECTION_ENCODER2));
}

void encoderInterrupt() {
  // bits 2-3 are old state and bits 0-1 are new state
  encoderState = ((encoderState & 0x03) << 2) | readEncoder();

  switch (encoderState) {
    case 0b0001:
    case 0b0111:
    case 0b1110:
    case 0b1000:
      // forward rotation
      turnAngle++;
      break;
    case 0b0010:
    case 0b1011:
    case 0b1101:
    case 0b0100:
      // backward rotation
      turnAngle--;
      break;
    default:
      // other combinations are either no movement or unexpected case. don't move
      break;
  }
}