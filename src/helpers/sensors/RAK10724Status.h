#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
namespace rak10724 {
struct Snapshot {
  bool environment_valid = false;
  bool power_valid = false;
  float temperature_c = 0, humidity_pct = 0;
  float bus_v = 0, current_ma = 0, power_mw = 0;
};
inline size_t formatStatus(char* out, size_t capacity, uint16_t battery_mv,
                          const Snapshot& sample,
                          bool power_saving, bool rxps) {
  if (!capacity) return 0;
  char battery[32], temp[16], hum[16], bus[16], current[16], power[16];
  if (battery_mv == 0 || battery_mv == 0xFFFF) {
    snprintf(battery, sizeof(battery), "na");
  } else {
    // Preserve the original BatteryInfo voltage curve, rounding and text format.
    const float v = (float)battery_mv / 1000.0f;
    int pct = (int)roundf((v - 3.0f) / (4.2f - 3.0f) * 100.0f);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    snprintf(battery, sizeof(battery), "%.2fv %d%%", battery_mv / 1000.0, pct);
  }
  const bool env = sample.environment_valid && isfinite(sample.temperature_c) &&
                   isfinite(sample.humidity_pct);
  const bool ina = sample.power_valid && isfinite(sample.bus_v) &&
                   isfinite(sample.current_ma) && isfinite(sample.power_mw);
  if (env) {
    snprintf(temp, sizeof(temp), "%.1fF", sample.temperature_c * 9.0f / 5.0f + 32.0f);
    snprintf(hum, sizeof(hum), "%.1f%%", sample.humidity_pct);
  } else {
    snprintf(temp, sizeof(temp), "na"); snprintf(hum, sizeof(hum), "na");
  }
  if (ina) {
    snprintf(bus, sizeof(bus), "%.2fv", sample.bus_v);
    snprintf(current, sizeof(current), "%.1fmA", sample.current_ma);
    snprintf(power, sizeof(power), "%.0fmW", sample.power_mw);
  } else {
    snprintf(bus, sizeof(bus), "na"); snprintf(current, sizeof(current), "na");
    snprintf(power, sizeof(power), "na");
  }
  int n = snprintf(out, capacity,
      "battery=%s temp=%s hum=%s bus=%s I=%s P=%s ps=%d rxps=%d",
      battery, temp, hum, bus, current, power, power_saving ? 1 : 0, rxps ? 1 : 0);
  if (n <= 0) return 0;
  // Do not transmit a truncated sensor value/unit: caller can drop oversized text.
  if ((size_t)n >= capacity) { out[0] = '\0'; return 0; }
  return (size_t)n;
}
template <typename Bus> bool samplePower(Bus& bus, Snapshot& sample) {
  sample.power_valid = false;
  // INA260: AVG=1, voltage/current conversion=1.1 ms, one-shot both.
  bool ok = bus.write(0, 0x6123);
  uint16_t flags = 0, voltage = 0, current = 0, power = 0;
  bool ready = false;
  for (int i = 0; ok && i < 10; ++i) {
    bus.wait();
    ok = bus.read(6, flags);
    if (ok && (flags & 0x0008)) { ready = true; break; }
  }
  ok = ok && ready && !(flags & 0x0004) && bus.read(1, current) &&
       bus.read(2, voltage) && bus.read(3, power);
  // Always attempt power-down, including NACK, overflow and conversion timeout.
  const bool asleep = bus.write(0, 0x6120);
  if (!ok || !asleep) return false;
  sample.bus_v = voltage * 0.00125f;
  sample.current_ma = (int16_t)current * 1.25f;
  sample.power_mw = power * 10.0f * (sample.current_ma < 0 ? -1.0f : 1.0f);
  sample.power_valid = true;
  return true;
}
inline uint8_t shtCRC(const uint8_t* bytes, size_t size) {
  uint8_t crc = 0xFF;
  for (size_t i = 0; i < size; ++i) {
    crc ^= bytes[i];
    for (int bit = 0; bit < 8; ++bit)
      crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : crc << 1;
  }
  return crc;
}
template <typename Bus> bool sampleEnvironment(Bus& bus, Snapshot& sample) {
  sample.environment_valid = false;
  bool ok = bus.command(0x3517); // wake
  bus.wait();
  ok = ok && bus.command(0x609C); // low-power, no clock stretch, temp first
  bus.wait(); // >0.8 ms conversion
  uint8_t data[6] = {};
  ok = ok && bus.receive(data, sizeof(data)); // bounded; NEVER retry forever on NACK
  const bool asleep = bus.command(0xB098);
  if (!ok || !asleep || shtCRC(data, 2) != data[2] || shtCRC(data + 3, 2) != data[5])
    return false;
  sample.temperature_c = ((data[0] << 8) | data[1]) * (175.0f / 65536.0f) - 45.0f;
  sample.humidity_pct = ((data[3] << 8) | data[4]) * (100.0f / 65536.0f);
  sample.environment_valid = true;
  return true;
}
const Snapshot& snapshot();
}
