#include <gtest/gtest.h>

#include "../../variants/ethermesh_1w/EtherMeshDefaults.h"

TEST(EtherMeshDefaults, HardwiredPowerReportsFullBatteryVoltage) {
  EXPECT_EQ(ethermesh::kHardwiredBatteryMilliVolts, 4200u);
}

TEST(EtherMeshDefaults, InvalidClockStartsAtBeginningOf2026) {
  EXPECT_EQ(ethermesh::normalizeInitialEpoch(0), 1767225600u);
  EXPECT_EQ(ethermesh::normalizeInitialEpoch(1767225599u), 1767225600u);
}

TEST(EtherMeshDefaults, ValidPushedClockIsPreserved) {
  EXPECT_EQ(ethermesh::normalizeInitialEpoch(1767225600u), 1767225600u);
  EXPECT_EQ(ethermesh::normalizeInitialEpoch(1786291200u), 1786291200u);
}

TEST(EtherMeshDefaults, FirmwareVersionIsBoardSpecific) {
  EXPECT_STREQ(ethermesh::kFirmwareVersion, "v1.17.0-EtherMesh");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
