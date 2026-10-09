#include <gtest/gtest.h>

#include "helpers/BatteryInfoAdvert.h"

TEST(BatteryInfoAdvert, ConvertsBatteryVoltageToEstimatedPercent) {
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

TEST(BatteryInfoAdvert, FormatsBatteryAndBuiltInTemperatureInFahrenheit) {
  char body[128];
  formatBatteryInfoReport(body, sizeof(body), 3600, 25.0f);
  EXPECT_STREQ("battery=3.60v 50% temp=77.0F", body);
  formatBatteryInfoReport(body, sizeof(body), 4200, 0.0f);
  EXPECT_STREQ("battery=4.20v 100% temp=32.0F", body);
  formatBatteryInfoReport(body, sizeof(body), 3000, -40.0f);
  EXPECT_STREQ("battery=3.00v 0% temp=-40.0F", body);
}

TEST(BatteryInfoAdvert, OmitsUnavailableTemperatureAndExternalSensorFields) {
  char body[128];
  formatBatteryInfoReport(body, sizeof(body), 3600, NAN);
  EXPECT_STREQ("battery=3.60v 50%", body);
  formatBatteryInfoReport(body, sizeof(body), 3600, INFINITY);
  EXPECT_STREQ("battery=3.60v 50%", body);
}

TEST(BatteryInfoAdvert, BoundsOutputAndTerminatesSmallBuffers) {
  char body[8];
  formatBatteryInfoReport(body, sizeof(body), 4200, 25.0f);
  EXPECT_EQ('\0', body[sizeof(body) - 1]);
  EXPECT_STREQ("battery", body);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
