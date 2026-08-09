#pragma once

#include <stdint.h>

namespace ethermesh {

constexpr uint16_t kHardwiredBatteryMilliVolts = 4200;
constexpr uint32_t kDefaultEpoch2026 = 1767225600UL;  // 2026-01-01 00:00:00 UTC
constexpr const char* kFirmwareVersion = "v1.17.0-EtherMesh";

constexpr uint32_t normalizeInitialEpoch(uint32_t epoch) {
  return epoch < kDefaultEpoch2026 ? kDefaultEpoch2026 : epoch;
}

}  // namespace ethermesh
