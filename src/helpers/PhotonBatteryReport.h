#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

// Human channel text only. Standard binary temperature remains Celsius.
// MAX17048 CRATE is signed percent/hour, not current or watts.
inline bool photonBatteryReport(char* out, size_t size, uint16_t mv, float chip_c, float rate) {
  if (!out || !size) return false;
  int used;
  if (mv == 0) {
    used = snprintf(out, size, "battery=na");
  } else {
    const int pct = mv <= 3000 ? 0 : mv >= 4200 ? 100 : ((mv - 3000U) * 100U + 600U) / 1200U;
    used = snprintf(out, size, "battery=%.2fv %d%%", mv / 1000.0f, pct);
  }
  if (used < 0 || (size_t)used >= size) return false;
  if (isfinite(rate)) {
    int n = snprintf(out + used, size - used, " charge=%+.2f%%/hr", rate);
    if (n < 0 || (size_t)n >= size - used) return false;
    used += n;
  }
  if (isfinite(chip_c)) {
    int n = snprintf(out + used, size - used, " temp=%.1fF", chip_c * (9.0f / 5.0f) + 32.0f);
    if (n < 0 || (size_t)n >= size - used) return false;
  }
  return true;
}
