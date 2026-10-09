#pragma once

#include <stdint.h>

inline int batteryInfoPercentFromMillivolts(uint16_t mv) {
  if (mv <= 3000U) return 0;
  if (mv >= 4200U) return 100;

  const uint32_t scaled = (uint32_t)(mv - 3000U) * 100U;
  return (int)((scaled + 600U) / 1200U);
}

inline bool shouldSendBatteryInfoAdvert(bool advert_created, bool flood) {
  return advert_created && flood;
}
