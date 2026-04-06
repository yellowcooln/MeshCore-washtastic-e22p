#pragma once

#include <Arduino.h>
#include <string.h>

#include <helpers/sensors/MicroNMEALocationProvider.h>

class PhotonGPSLocationProvider : public MicroNMEALocationProvider {
  static const unsigned long POWER_TOGGLE_GUARD_MS = 2000;
  static const unsigned long WAKE_SETTLE_MS = 250;
  static const unsigned long COMMAND_DELAY_MS = 40;

  bool _sleeping = false;
  unsigned long _last_power_toggle_ms = 0;

  void waitForPowerToggleGuard() {
    if (_last_power_toggle_ms == 0) return;

    unsigned long elapsed = millis() - _last_power_toggle_ms;
    if (elapsed < POWER_TOGGLE_GUARD_MS) {
      delay(POWER_TOGGLE_GUARD_MS - elapsed);
    }
  }

  void sendQueuedCommand(const char *sentence, unsigned long settle_ms = COMMAND_DELAY_MS) {
    sendSentence(sentence);
    if (settle_ms > 0) {
      delay(settle_ms);
    }
  }

  bool waitForAck(const char *needle, unsigned long timeout_ms = 600) {
    char window[32] = {0};
    size_t used = 0;
    unsigned long start = millis();

    while ((millis() - start) < timeout_ms) {
      while (gpsSerial().available()) {
        char c = gpsSerial().read();

        if (used < sizeof(window) - 1) {
          window[used++] = c;
          window[used] = 0;
        } else {
          memmove(window, window + 1, sizeof(window) - 2);
          window[sizeof(window) - 2] = c;
          window[sizeof(window) - 1] = 0;
        }

        if (strstr(window, needle) != NULL) {
          return true;
        }
      }
    }

    return false;
  }

  void configureQuietNmea() {
    // MicroNMEA only needs GGA and RMC for fix, altitude, satellites, and time.
    sendQueuedCommand("$PAIR062,0,1"); // GGA once per fix
    sendQueuedCommand("$PAIR062,1,0"); // GLL off
    sendQueuedCommand("$PAIR062,2,0"); // GSA off
    sendQueuedCommand("$PAIR062,3,1"); // GSV once per fix for satellites-in-view details
    sendQueuedCommand("$PAIR062,4,1"); // RMC once per fix
    sendQueuedCommand("$PAIR062,5,0"); // VTG off
    sendQueuedCommand("$PAIR062,6,0"); // ZDA off
    sendQueuedCommand("$PAIR062,7,0"); // GRS off
    sendQueuedCommand("$PAIR062,8,0"); // GST off
    sendQueuedCommand("$PAIR062,9,0"); // GNS off
  }

public:
  PhotonGPSLocationProvider(Stream &ser, mesh::RTCClock *clock = NULL)
      : MicroNMEALocationProvider(ser, clock) {}

  void begin() override {
    MicroNMEALocationProvider::begin();

    if (_sleeping) {
      waitForPowerToggleGuard();
      sendSentence("$PAIR002");
      waitForAck("$PAIR001,002,0");
      _sleeping = false;
      _last_power_toggle_ms = millis();
      delay(WAKE_SETTLE_MS);
    }

    configureQuietNmea();
  }

  void stop() override {
    if (!_sleeping) {
      waitForPowerToggleGuard();
      sendSentence("$PAIR003");
      if (waitForAck("$PAIR001,003,0")) {
        _sleeping = true;
        _last_power_toggle_ms = millis();
      }
    }

    MicroNMEALocationProvider::stop();
  }

  bool isEnabled() override {
    return !_sleeping && MicroNMEALocationProvider::isEnabled();
  }
};
