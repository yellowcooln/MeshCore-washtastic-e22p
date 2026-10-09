#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

inline int batteryInfoPercentFromMillivolts(uint16_t mv) {
  if (mv <= 3000U) return 0;
  if (mv >= 4200U) return 100;

  const uint32_t scaled = (uint32_t)(mv - 3000U) * 100U;
  return (int)((scaled + 600U) / 1200U);
}

inline void formatBatteryInfoReport(char* out, size_t size, uint16_t battery_mv, float mcu_temp_c) {
  // The nRF52840 die-temperature sensor needs no external sensor module.
  // Only the channel text changes units; standard telemetry remains Celsius.
  const float battery_v = (float)battery_mv / 1000.0f;
  const int percent = batteryInfoPercentFromMillivolts(battery_mv);
  if (isfinite(mcu_temp_c)) {
    const float temp_f = mcu_temp_c * (9.0f / 5.0f) + 32.0f;
    snprintf(out, size, "battery=%.2fv %d%% temp=%.1fF", battery_v, percent, temp_f);
  } else {
    snprintf(out, size, "battery=%.2fv %d%%", battery_v, percent);
  }
}

inline bool shouldSendBatteryInfoAdvert(bool advert_created, bool flood) {
  return advert_created && flood;
}
