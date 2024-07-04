/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"
#include "lut.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_INFO, {	{ "app", LOG_LEVEL_INFO } });

const auto DIRECTION_PWM = A2;
const auto DIRECTION_LEFT = A0;
const auto DIRECTION_RIGHT = A1;

const auto WHEELS_PWM = A5;
const auto WHEELS_REVERSE = S4;

// Turn off outputs after this amount of time without BLE controls update
const auto SAFETY_SHUTOFF_TIME = 1000;

// Throttle map between input 0:2048 to motor 0:255
std::vector<int16_t> throttleRanges = { 0, 2048 };
std::vector<int16_t> throttleValues = { 0, 20 };

int16_t x = 0, y = 0, throttle = 0;
boolean sw = false;
uint8_t counter = 0;
bool safetyShutoff = true;

int16_t wheelsPower = 0;
bool wheelsReverse = false;

static const BleUuid serviceUuid("363f1314-4a0d-4332-ab1b-b4c52469e573");
static const BleUuid controlsUuid("363f1315-4a0d-4332-ab1b-b4c52469e573");

BleCharacteristic controlsCharacteristic;

Thread *hardwareThread;

void bleConnect();
void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);
void runHardware();

void setup() {
  Particle.function("t", [&](String arg) {
    throttleValues[1] = arg.toInt();
    return throttleValues[1];
  });

  hardwareThread = new Thread("hardware", runHardware);
  BLE.on();
  controlsCharacteristic.onDataReceived(onDataReceived, nullptr);
}

void loop() {
  if (!BLE.connected()) {
    bleConnect();
  }

  // Log.info("x=%4d y=%4d sw=%d counter=%d", x, y, sw ? 1 : 0, counter);
  static bool oldSafetyShutoff = safetyShutoff;
  if (safetyShutoff != oldSafetyShutoff) {
      Log.info(safetyShutoff ? "Safety shutoff" : "Starting motor operation");
  }
  oldSafetyShutoff = safetyShutoff;

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
  int16_t throttle;
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
  throttle = controls->throttle;
  sw = controls->bits & (1 << CONTROL_SWITCH);
  counter = controls->counter;
}

// multiple the input by itself a number of times to make controls non-linear
int16_t cube(int16_t v) {
  const auto INPUT_RANGE = 11;
  for (auto i = 0; i < 3; i++) {
    v = (v * abs(v)) >> INPUT_RANGE;
  }
  return v;
}


// Function to apply slew rate limitation
int applySlewRate(int actual, int target, int maxChange) {
    // Calculate the difference between the target power and the current power
    int difference = target - actual;

    // Limit the change to the maximum allowed change per loop
    if (difference > maxChange) {
        difference = maxChange;
    } else if (difference < -maxChange) {
        difference = -maxChange;
    }

    // Apply the limited change
    actual += difference;

    // Clamp the result to the valid motor power range [-255, 255]
    actual = std::max(-255, std::min(255, actual));

    return actual;
}

void runHardware() {
  uint8_t lastCounter = 0;
  system_tick_t lastUpdate = 0;
  system_tick_t lastSlew = 0;
  int16_t w = 0;
  
  pinMode(DIRECTION_PWM, OUTPUT);
  pinMode(DIRECTION_LEFT, OUTPUT);
  pinMode(DIRECTION_RIGHT, OUTPUT);

  pinMode(WHEELS_PWM, OUTPUT);
  pinMode(WHEELS_REVERSE, OUTPUT);
  
  while (true) {
    if (counter != lastCounter) {
      lastUpdate = millis();
      lastCounter = counter;
      safetyShutoff = false;
    }
    if (millis() - lastUpdate > SAFETY_SHUTOFF_TIME) {
      safetyShutoff = true;
    }

    // direction PWM
    auto d = cube(y) / (2048/256);
    // wheels PWM
    auto wt = interpolate(throttleRanges, throttleValues, (int16_t) abs(throttle)) * (throttle > 0 ? 1 : -1);

    if (millis() - lastSlew > 10) {
      lastSlew = millis();
      w = applySlewRate(w, wt, 10);
    }

    if (safetyShutoff) {
      digitalWrite(DIRECTION_LEFT, LOW);
      digitalWrite(DIRECTION_RIGHT, LOW);
      analogWrite(DIRECTION_PWM, 0, 10000);

      digitalWrite(WHEELS_REVERSE, LOW);
      analogWrite(WHEELS_PWM, 0, 10000);
    } else {
      digitalWrite(DIRECTION_LEFT, d > 0 ? HIGH : LOW);
      digitalWrite(DIRECTION_RIGHT, d > 0 ? LOW : (d == 0 ? LOW : HIGH));
      analogWrite(DIRECTION_PWM, abs(d), 10000);

      digitalWrite(WHEELS_REVERSE, w > 0 ? LOW : HIGH);
      analogWrite(WHEELS_PWM, abs(w), 10000);
    }
  }
}
