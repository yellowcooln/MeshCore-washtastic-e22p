#pragma once

#include <helpers/ESP32Board.h>

class EtherMeshBoard : public ESP32Board {
public:
  const char* getManufacturerName() const override {
    return "EtherMesh-1W";
  }
};
