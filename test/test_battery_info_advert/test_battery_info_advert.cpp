#include <gtest/gtest.h>

#include "helpers/BatteryInfoAdvert.h"

TEST(BatteryInfoAdvert, ConvertsPhotonFuelGaugeVoltageToPercent) {
  EXPECT_EQ(0, batteryInfoPercentFromMillivolts(3000));
  EXPECT_EQ(50, batteryInfoPercentFromMillivolts(3600));
  EXPECT_EQ(100, batteryInfoPercentFromMillivolts(4200));
  EXPECT_EQ(0, batteryInfoPercentFromMillivolts(0));
  EXPECT_EQ(100, batteryInfoPercentFromMillivolts(5000));
}

TEST(BatteryInfoAdvert, FollowsOnlySuccessfulFloodAdvertisements) {
  EXPECT_TRUE(shouldSendBatteryInfoAdvert(true, true));
  EXPECT_FALSE(shouldSendBatteryInfoAdvert(true, false));
  EXPECT_FALSE(shouldSendBatteryInfoAdvert(false, true));
  EXPECT_FALSE(shouldSendBatteryInfoAdvert(false, false));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
