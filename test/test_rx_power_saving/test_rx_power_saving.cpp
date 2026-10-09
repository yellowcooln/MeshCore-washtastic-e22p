#include <gtest/gtest.h>

#include "helpers/radiolib/RXPowerSaving.h"

TEST(RXPowerSaving, ReferenceLevelsSf10Bw250Preamble16) {
  uint32_t rx_us = 0;
  uint32_t sleep_us = 0;

  ASSERT_TRUE(calcRxPowerSavingLevel(1, 10, 250.0f, 16, &rx_us, &sleep_us));
  EXPECT_EQ(49152UL, rx_us);
  EXPECT_EQ(8192UL, sleep_us);

  ASSERT_TRUE(calcRxPowerSavingLevel(5, 10, 250.0f, 16, &rx_us, &sleep_us));
  EXPECT_EQ(41871UL, rx_us);
  EXPECT_EQ(26851UL, sleep_us);

  ASSERT_TRUE(calcRxPowerSavingLevel(10, 10, 250.0f, 16, &rx_us, &sleep_us));
  EXPECT_EQ(32768UL, rx_us);
  EXPECT_EQ(50176UL, sleep_us);
}

TEST(RXPowerSaving, AutomaticPreambleTracksSpreadingFactor) {
  EXPECT_EQ(32, rxPowerSavingPreambleForSF(8));
  EXPECT_EQ(16, rxPowerSavingPreambleForSF(9));
}

TEST(RXPowerSaving, RejectsInvalidLevelAndRadioParameters) {
  uint32_t rx_us = 123;
  uint32_t sleep_us = 456;

  EXPECT_FALSE(calcRxPowerSavingLevel(0, 10, 250.0f, 16, &rx_us, &sleep_us));
  EXPECT_FALSE(calcRxPowerSavingLevel(11, 10, 250.0f, 16, &rx_us, &sleep_us));
  EXPECT_FALSE(calcRxPowerSavingLevel(5, 10, 0.0f, 16, &rx_us, &sleep_us));
  EXPECT_FALSE(calcRxPowerSavingLevel(5, 10, 250.0f, 8, &rx_us, &sleep_us));
}

TEST(RXPowerSaving, LevelDerivedTimingsRetuneAndManualModeDoesNot) {
  uint32_t rx_us = 10000;
  uint32_t sleep_us = 20000;

  EXPECT_FALSE(recalcRxPowerSavingFromLevel(0, 10, 250.0f, 0, &rx_us, &sleep_us));
  EXPECT_EQ(10000UL, rx_us);
  EXPECT_EQ(20000UL, sleep_us);

  EXPECT_TRUE(recalcRxPowerSavingFromLevel(5, 10, 250.0f, 16, &rx_us, &sleep_us));
  EXPECT_EQ(41871UL, rx_us);
  EXPECT_EQ(26851UL, sleep_us);
}

TEST(RXPowerSaving, PeriodBoundsAndSx1262MinimumAreExplicit) {
  EXPECT_FALSE(isValidRxPowerSavingPeriod(RX_POWERSAVING_MIN_PERIOD_US - 1));
  EXPECT_TRUE(isValidRxPowerSavingPeriod(RX_POWERSAVING_MIN_PERIOD_US));
  EXPECT_TRUE(isValidRxPowerSavingPeriod(RX_POWERSAVING_MAX_PERIOD_US));
  EXPECT_FALSE(isValidRxPowerSavingPeriod(RX_POWERSAVING_MAX_PERIOD_US + 1));
  EXPECT_GT(RX_POWERSAVING_SX1262_MIN_SLEEP_US, RX_POWERSAVING_MIN_PERIOD_US);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
