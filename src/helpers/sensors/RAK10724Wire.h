#pragma once
#include <Wire.h>
#include "RAK10724Status.h"

namespace rak10724 {
// Checked Wire transport shared by the two on-demand sensors. No rail GPIO writes.
class WireSensor {
  TwoWire* wire;
  uint8_t address;
public:
  WireSensor(TwoWire* w, uint8_t a) : wire(w), address(a) {}
  bool command(uint16_t cmd) {
    wire->beginTransmission(address);
    wire->write((uint8_t)(cmd >> 8)); wire->write((uint8_t)cmd);
    return wire->endTransmission() == 0;
  }
  bool receive(uint8_t* out, size_t n) {
    if (wire->requestFrom(address, (uint8_t)n) != n) return false;
    for (size_t i = 0; i < n; ++i) out[i] = wire->read();
    return true;
  }
  bool write(uint8_t reg, uint16_t value) {
    wire->beginTransmission(address); wire->write(reg);
    wire->write((uint8_t)(value >> 8)); wire->write((uint8_t)value);
    return wire->endTransmission() == 0;
  }
  bool read(uint8_t reg, uint16_t& value) {
    wire->beginTransmission(address); wire->write(reg);
    if (wire->endTransmission(false) != 0) return false;
    uint8_t bytes[2];
    if (!receive(bytes, sizeof(bytes))) return false;
    value = (bytes[0] << 8) | bytes[1];
    return true;
  }
  void wait() { delay(1); }
  bool initEnvironment() {
    bool ok = command(0x3517); wait();
    uint8_t id[3] = {};
    ok = ok && command(0xEFC8) && receive(id, sizeof(id));
    const bool asleep = command(0xB098);
    return ok && asleep && shtCRC(id, 2) == id[2] &&
           ((((id[0] << 8) | id[1]) & 0x083F) == 0x0807);
  }
  bool initPower() {
    uint16_t manufacturer = 0, device = 0;
    const bool ok = read(0xFE, manufacturer) && read(0xFF, device) &&
                    manufacturer == 0x5449 && (device >> 4) == 0x227;
    const bool asleep = write(0, 0x6120);
    return ok && asleep;
  }
};
}
