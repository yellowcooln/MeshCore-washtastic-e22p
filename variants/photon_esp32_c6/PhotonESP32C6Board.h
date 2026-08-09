#pragma once

#include <Arduino.h>
#include <helpers/ESP32Board.h>

class PhotonESP32C6Board : public ESP32Board {
public:
  void begin() {
    ESP32Board::begin();
  }

  const char* getManufacturerName() const override {
    return "Photon ESP32-C6";
  }
};
