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

const auto DIRECTION_POWER = D1;
const auto DIRECTION_LEFT = D2;
const auto DIRECTION_RIGHT = D3;

// Turn off outputs after this amount of time without BLE controls update
const auto SAFETY_SHUTOFF_TIME = 1000;

int16_t x = 0, y = 0;
boolean sw = false;
uint8_t counter;

static const BleUuid serviceUuid("363f1314-4a0d-4332-ab1b-b4c52469e573");
static const BleUuid controlsUuid("363f1315-4a0d-4332-ab1b-b4c52469e573");

BleCharacteristic controlsCharacteristic;

Thread *hardwareThread;

void bleConnect();
void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);
void runHardware();

void setup() {
  hardwareThread = new Thread("hardware", runHardware);
  BLE.on();
  controlsCharacteristic.onDataReceived(onDataReceived, nullptr);
}

void loop() {
  if (!BLE.connected()) {
    bleConnect();
  }

  Log.info("x=%4d y=%4d sw=%d counter=%d", x, y, sw ? 1 : 0, counter);
  delay(100);
}

void bleConnect() {
  Log.info("Starting BLE scan");
  BleAddress address;
  BleScanFilter filter;
  filter.serviceUUID(serviceUuid);
  // blocks until scan completes
  BLE.scanWithFilter(filter, [&](const BleScanResult& result) {
    address = result.address();
    BLE.stopScanning();
  });
  Log.info("Found BLE devices? %s", address.isValid() ? "yes" : "no");
  if (address.isValid()) {
    // blocks until connect completes
    auto peer = BLE.connect(address);
    if (peer.connected()) {
      Log.info("BLE connected");
      peer.getCharacteristicByUUID(controlsCharacteristic, controlsUuid);
    } else {
      Log.info("BLE connection failed");
    }
  }
}

enum CONTROL_BITS {
  CONTROL_SWITCH = 0
};

struct ControlsTx {
  int16_t x;
  int16_t y;
  uint8_t bits;
  uint8_t counter;
} __attribute__((packed));

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
  if (len != sizeof(ControlsTx)) {
    Log.info("controller / motor driver out of date! ignoring data");
    return;
  }

  ControlsTx* controls = (ControlsTx*) data;
  x = controls->x;
  y = controls->y;
  sw = controls->bits & (1 << CONTROL_SWITCH);
  counter = controls->counter;
}

void runHardware() {
  uint8_t lastCounter = 0;
  system_tick_t lastUpdate = 0;

  pinMode(DIRECTION_POWER, OUTPUT);
  pinMode(DIRECTION_LEFT, OUTPUT);
  pinMode(DIRECTION_RIGHT, OUTPUT);
  
  while (true) {
    if (counter != lastCounter) {
      lastUpdate = millis();
    }
    if (millis() - lastUpdate > SAFETY_SHUTOFF_TIME) {
      digitalWrite(DIRECTION_LEFT, LOW);
      digitalWrite(DIRECTION_RIGHT, LOW);
      analogWrite(DIRECTION_POWER, 0, 10000);

    } else {
      digitalWrite(DIRECTION_LEFT, y > 0 ? HIGH : LOW);
      digitalWrite(DIRECTION_RIGHT, y > 0 ? LOW : (y == 0 ? LOW : HIGH));
      analogWrite(DIRECTION_POWER, abs(y), 10000);
    }
  }
}
