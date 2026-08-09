#pragma once

#include <helpers/ESP32Board.h>
#include "EtherMeshDefaults.h"

class EtherMeshRTCClock : public ESP32RTCClock {
public:
  void begin() {
    ESP32RTCClock::begin();
    setCurrentTime(ethermesh::normalizeInitialEpoch(getCurrentTime()));
  }
};

class EtherMeshBoard : public ESP32Board {
public:
  uint16_t getBattMilliVolts() override {
    return ethermesh::kHardwiredBatteryMilliVolts;
  }

  const char* getManufacturerName() const override {
    return "EtherMesh-1W";
  }
};
