/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

const auto JOYSTICK_GND = A0;
const auto JOYSTICK_POWER = A1;
const auto JOYSTICK_X = A2;
const auto JOYSTICK_Y = A5;
const auto JOYSTICK_SW = S4;

const auto JOYSTICK_MID = 2048;

const auto UPDATE_INTERVAL = 100;

int16_t x = 0, y = 0;
boolean sw = false;
uint8_t counter = 0;

static const BleUuid serviceUuid("363f1314-4a0d-4332-ab1b-b4c52469e573");
static const BleUuid controlsUuid("363f1315-4a0d-4332-ab1b-b4c52469e573");

BleCharacteristic controlsCharacteristic("controls", BleCharacteristicProperty::NOTIFY, controlsUuid, serviceUuid);

enum CONTROL_BITS {
  CONTROL_SWITCH = 0
};

struct ControlsTx {
  int16_t x;
  int16_t y;
  uint8_t bits;
  uint8_t counter;
} __attribute__((packed));

void setup() {
  pinMode(JOYSTICK_POWER, OUTPUT);
  digitalWrite(JOYSTICK_POWER, HIGH);
  pinMode(JOYSTICK_GND, OUTPUT);
  digitalWrite(JOYSTICK_GND, LOW);
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_SW, INPUT_PULLUP);

  BLE.addCharacteristic(controlsCharacteristic);

  BLE.on();
  BleAdvertisingData data;
  data.appendServiceUUID(serviceUuid);
  BLE.advertise(&data);
}

int16_t scale(int val) {
  return (int16_t) (val - JOYSTICK_MID);
}

void loop() {
  static system_tick_t lastUpdate = 0;

  x = scale(analogRead(JOYSTICK_X));
  y = scale(analogRead(JOYSTICK_Y));
  sw = digitalRead(JOYSTICK_SW);
  Log.info("x=%4d y=%4d sw=%d", x, y, sw ? 1 : 0);

  if (BLE.connected() && millis() - lastUpdate > UPDATE_INTERVAL) {
    uint8_t bits = sw ? (1 << CONTROL_SWITCH) : 0;
    counter++;
    ControlsTx data = { x, y, bits, counter };
    controlsCharacteristic.setValue(data);
    lastUpdate = millis();
  }
}
