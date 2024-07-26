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

SerialLogHandler logHandler(LOG_LEVEL_NONE, {	{ "app", LOG_LEVEL_INFO } });

BlePeerDevice peer;

// BleUuid hidUuid(0x1812);
// BleService hidService;
BleUuid reportUuid(0x2A4D);
BleCharacteristic inputReportCharacteristic;

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);

void setup() {
  BLE.on();
}

void loop() {
  if (!BLE.connected()) {
    BleAddress addr("40:8E:2C:61:9e:20");
    Log.info("Connecting to %s...", addr.toString().c_str());
    peer = BLE.connect(addr);
    if (peer.connected()) {
      Log.info("successfully connected");
      BLE.startPairing(peer);
      while (BLE.isPairing(peer)) {
        delay(100);
      }
      if (BLE.isPaired(peer)) {
        Log.info("paired");
      } else {
        Log.info("pairing failed");
      }

      for (auto& ch : peer.characteristics()) {
        Log.info("characteristic uuid=%s props=%u desc=%s", ch.UUID().toString().c_str(), ch.properties().value(), ch.description().c_str());
        if (ch.UUID() == reportUuid) {
          Log.info("found report");
        }
        if (ch.properties() & BleCharacteristicProperty::NOTIFY) {
          Log.info("found notify");
        }
        if (ch.UUID() == reportUuid && (ch.properties() & BleCharacteristicProperty::NOTIFY)) {
          inputReportCharacteristic = ch;
          inputReportCharacteristic.onDataReceived(onDataReceived, NULL);
          Log.info("Connected to input report characteristic");
        }
      }

      // peer.getServiceByUUID(hidService, hidUuid);
      // auto characteristics = peer.discoverCharacteristicsOfService(hidService);
      // Log.info("%d characteristics", characteristics.size());
      // for (auto it = characteristics.begin(); it != characteristics.end(); ++it) {
      //   Log.info("characteristic %s", it->UUID().toString().c_str());
      // }
      // 2A4A HID info
      // 2A4C HID control point
      // 2A4B Report map
      // 2A4D Report (Input, output, feature total 2 characteristics)
      // 2A4D
    } else {
      Log.info("connection failed");
    }
  }

  delay(500);
}

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
  Log.info("input %d bytes", len);
  while (len >= 16) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    len -= 16;
  }
  if (len == 15) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
  }
  if (len == 14) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13]);
  }
  if (len == 13) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12]);
  }
  if (len == 12) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
  }
  if (len == 11) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10]);
  }
  if (len == 10) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
  }
  if (len == 9) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
  }
  if (len == 8) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  }
  if (len == 7) {
    Log.info("%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
  }
  if (len == 6) {
    Log.info("%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5]);
  }
  if (len == 5) {
    Log.info("%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4]);
  }
  if (len == 4) {
    Log.info("%02x%02x%02x%02x", data[0], data[1], data[2], data[3]);
  }
  if (len == 3) {
    Log.info("%02x%02x%02x", data[0], data[1], data[2]);
  }
  if (len == 2) {
    Log.info("%02x%02x", data[0], data[1]);
  }
  if (len == 1) {
    Log.info("%02x", data[0]);
  }
}

// const auto DIRECTION_PWM = A2;
// const auto DIRECTION_LEFT = A0;
// const auto DIRECTION_RIGHT = A1;

// const auto WHEELS_PWM = A5;
// const auto WHEELS_REVERSE = S4;

// // Turn off outputs after this amount of time without BLE controls update
// const auto SAFETY_SHUTOFF_TIME = 1000;

// // Throttle map between input 0:2048 to motor 0:255
// std::vector<int16_t> throttleRanges = { 0, 200, 1000, 1500, 2000, 2048 };
// std::vector<int16_t> throttleValues = { 0, 0, 40, 60, 80, 150 };

// int16_t x = 0, y = 0, throttle = 0;
// boolean sw = false;
// uint8_t counter = 0;
// bool safetyShutoff = true;

// int16_t d = 0, wt = 0;

// static const BleUuid serviceUuid("363f1314-4a0d-4332-ab1b-b4c52469e573");
// static const BleUuid controlsUuid("363f1315-4a0d-4332-ab1b-b4c52469e573");

// BleCharacteristic controlsCharacteristic;

// Thread *hardwareThread;

// void bleConnect();
// void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);
// void runHardware();

// void setup() {
//   Particle.function("t", [&](String arg) {
//     throttleValues[1] = arg.toInt();
//     return throttleValues[1];
//   });

//   hardwareThread = new Thread("hardware", runHardware);
//   BLE.on();
//   controlsCharacteristic.onDataReceived(onDataReceived, nullptr);
// }

// void loop() {
//   if (!BLE.connected()) {
//     bleConnect();
//   }

//   // Log.info("x=%4d y=%4d sw=%d counter=%d", x, y, sw ? 1 : 0, counter);
//   Log.info("t=%4d wt=%4d", throttle, wt);
//   static bool oldSafetyShutoff = safetyShutoff;
//   if (safetyShutoff != oldSafetyShutoff) {
//       Log.info(safetyShutoff ? "Safety shutoff" : "Starting motor operation");
//   }
//   oldSafetyShutoff = safetyShutoff;

//   delay(100);
// }

// void bleConnect() {
//   Log.info("Starting BLE scan");
//   BleAddress address;
//   BleScanFilter filter;
//   filter.serviceUUID(serviceUuid);
//   // blocks until scan completes
//   BLE.scanWithFilter(filter, [&](const BleScanResult& result) {
//     address = result.address();
//     BLE.stopScanning();
//   });
//   Log.info("Found BLE devices? %s", address.isValid() ? "yes" : "no");
//   if (address.isValid()) {
//     // blocks until connect completes
//     auto peer = BLE.connect(address);
//     if (peer.connected()) {
//       Log.info("BLE connected");
//       peer.getCharacteristicByUUID(controlsCharacteristic, controlsUuid);
//     } else {
//       Log.info("BLE connection failed");
//     }
//   }
// }

// enum CONTROL_BITS {
//   CONTROL_SWITCH = 0
// };

// struct ControlsTx {
//   int16_t x;
//   int16_t y;
//   int16_t throttle;
//   uint8_t bits;
//   uint8_t counter;
// } __attribute__((packed));

// void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
//   if (len != sizeof(ControlsTx)) {
//     Log.info("controller / motor driver out of date! ignoring data");
//     return;
//   }

//   ControlsTx* controls = (ControlsTx*) data;
//   x = controls->x;
//   y = controls->y;
//   throttle = controls->throttle;
//   sw = controls->bits & (1 << CONTROL_SWITCH);
//   counter = controls->counter;
// }

// // multiple the input by itself a number of times to make controls non-linear
// int16_t cube(int16_t v) {
//   const auto INPUT_RANGE = 11;
//   for (auto i = 0; i < 3; i++) {
//     v = (v * abs(v)) >> INPUT_RANGE;
//   }
//   return v;
// }


// // Function to apply slew rate limitation
// int applySlewRate(int actual, int target, int maxChange) {
//     // Calculate the difference between the target power and the current power
//     int difference = target - actual;

//     // Limit the change to the maximum allowed change per loop
//     if (difference > maxChange) {
//         difference = maxChange;
//     } else if (difference < -maxChange) {
//         difference = -maxChange;
//     }

//     // Apply the limited change
//     actual += difference;

//     // Clamp the result to the valid motor power range [-255, 255]
//     actual = std::max(-255, std::min(255, actual));

//     return actual;
// }

// void runHardware() {
//   uint8_t lastCounter = 0;
//   system_tick_t lastUpdate = 0;
//   system_tick_t lastSlew = 0;
//   int16_t w = 0;
  
//   pinMode(DIRECTION_PWM, OUTPUT);
//   pinMode(DIRECTION_LEFT, OUTPUT);
//   pinMode(DIRECTION_RIGHT, OUTPUT);

//   pinMode(WHEELS_PWM, OUTPUT);
//   pinMode(WHEELS_REVERSE, OUTPUT);
  
//   while (true) {
//     if (counter != lastCounter) {
//       lastUpdate = millis();
//       lastCounter = counter;
//       safetyShutoff = false;
//     }
//     if (millis() - lastUpdate > SAFETY_SHUTOFF_TIME) {
//       safetyShutoff = true;
//     }

//     // direction PWM
//     d = cube(y) / (2048/256);
//     // wheels PWM
//     wt = interpolate(throttleRanges, throttleValues, (int16_t) abs(throttle)) * (throttle > 0 ? 1 : -1);

//     if (millis() - lastSlew > 10) {
//       lastSlew = millis();
//       w = applySlewRate(w, wt, 10);
//     }

//     if (safetyShutoff) {
//       digitalWrite(DIRECTION_LEFT, LOW);
//       digitalWrite(DIRECTION_RIGHT, LOW);
//       analogWrite(DIRECTION_PWM, 0, 10000);

//       digitalWrite(WHEELS_REVERSE, LOW);
//       analogWrite(WHEELS_PWM, 0, 10000);
//     } else {
//       digitalWrite(DIRECTION_LEFT, d > 0 ? HIGH : LOW);
//       digitalWrite(DIRECTION_RIGHT, d > 0 ? LOW : (d == 0 ? LOW : HIGH));
//       analogWrite(DIRECTION_PWM, abs(d), 10000);

//       digitalWrite(WHEELS_REVERSE, w > 0 ? LOW : HIGH);
//       analogWrite(WHEELS_PWM, abs(w), 10000);
//     }
//   }
// }
