#include <gtest/gtest.h>
#include <cstring>
#include <helpers/sensors/RAK10724Status.h>

// Inject only the unavailable I2C transport; exercise production sampling logic.
struct Bus {
  bool ready = true, fail = false, overflow = false, shutdown_fail = false;
  int waits = 0, writes = 0, fail_register = -1;
  uint16_t last_config = 0;
  bool write(uint8_t reg, uint16_t value) {
    EXPECT_EQ(reg, 0); last_config = value; ++writes;
    return !fail && !(shutdown_fail && value == 0x6120);
  }
  bool read(uint8_t reg, uint16_t& value) {
    if (fail || reg == fail_register) return false;
    if (reg == 6) value = (ready ? 8 : 0) | (overflow ? 4 : 0);
    else if (reg == 1) value = static_cast<uint16_t>(-80);
    else if (reg == 2) value = 3000;
    else if (reg == 3) value = 38;
    else { ADD_FAILURE(); return false; }
    return true;
  }
  void wait() { ++waits; }
};
struct ShtBus {
  bool fail = false, corrupt = false, shutdown_fail = false;
  uint16_t last_command = 0;
  int waits = 0, commands = 0, reads = 0;
  bool command(uint16_t value) {
    last_command = value; ++commands;
    return !fail && !(shutdown_fail && value == 0xB098);
  }
  bool receive(uint8_t* out, size_t n) {
    EXPECT_EQ(n, 6U); ++reads;
    const uint8_t bytes[] = {0x66, 0x66, 0x93, 0x80, 0, 0xA2};
    memcpy(out, bytes, n); if (corrupt) out[2] ^= 1; return !fail;
  }
  void wait() { ++waits; }
};
TEST(RAK10724Status, MissingSensorsAreNotZeroReadings) {
  rak10724::Snapshot s; char body[160];
  ASSERT_GT(rak10724::formatStatus(body, sizeof(body), 3400, s, true, true), 0U);
  EXPECT_NE(strstr(body, "battery=3.40v 33%"), nullptr);
  EXPECT_EQ(strstr(body, "LOW"), nullptr);
  EXPECT_EQ(strstr(body, "OK"), nullptr);
  EXPECT_NE(strstr(body, "temp=na"), nullptr);
  EXPECT_NE(strstr(body, "bus=na"), nullptr);
}
TEST(RAK10724Status, OriginalBatteryPercentageWithoutAlertLabels) {
  rak10724::Snapshot s; char body[160];
  for (uint16_t mv : {3500, 3501, 2900, 4500}) {
    ASSERT_GT(rak10724::formatStatus(body, sizeof(body), mv, s, false, false), 0U);
    EXPECT_EQ(strstr(body, "LOW"), nullptr);
    EXPECT_EQ(strstr(body, "OK"), nullptr);
    EXPECT_EQ(strstr(body, "est="), nullptr);
    if (mv == 2900) EXPECT_NE(strstr(body, "battery=2.90v 0%"), nullptr);
    if (mv == 4500) EXPECT_NE(strstr(body, "battery=4.50v 100%"), nullptr);
  }
}
TEST(RAK10724Status, UnavailableBatteryNeverGeneratesLowWarning) {
  rak10724::Snapshot s; char body[160];
  for (uint16_t mv : {0, 65535}) {
    ASSERT_GT(rak10724::formatStatus(body, sizeof(body), mv, s, true, true), 0U);
    EXPECT_NE(strstr(body, "battery=na"), nullptr);
    EXPECT_EQ(strstr(body, "LOW"), nullptr);
  }
}
TEST(RAK10724Status, OversizedMessageIsDroppedNotTruncated) {
  rak10724::Snapshot s; char body[8] = "bad";
  EXPECT_EQ(rak10724::formatStatus(body, sizeof(body), 4000, s, true, true), 0U);
  EXPECT_STREQ(body, "");
  EXPECT_EQ(rak10724::formatStatus(nullptr, 0, 4000, s, true, true), 0U);
}
TEST(RAK10724Power, SignedCurrentUnitsAndPowerDown) {
  rak10724::Snapshot s; Bus bus;
  ASSERT_TRUE(rak10724::samplePower(bus, s));
  EXPECT_TRUE(s.power_valid); EXPECT_FLOAT_EQ(s.current_ma, -100);
  EXPECT_FLOAT_EQ(s.bus_v, 3.75); EXPECT_FLOAT_EQ(s.power_mw, -380);
  EXPECT_EQ(bus.last_config, 0x6120); EXPECT_EQ(bus.writes, 2);
  char body[160];
  ASSERT_GT(rak10724::formatStatus(body, sizeof(body), 4000, s, true, true), 0U);
  EXPECT_NE(strstr(body, "I=-100.0mA"), nullptr);
  EXPECT_NE(strstr(body, "P=-380mW"), nullptr);
}
TEST(RAK10724Power, ConversionTimeoutIsBoundedAndSleeps) {
  rak10724::Snapshot s; s.power_valid = true; Bus bus; bus.ready = false;
  EXPECT_FALSE(rak10724::samplePower(bus, s)); EXPECT_FALSE(s.power_valid);
  EXPECT_EQ(bus.waits, 10); EXPECT_EQ(bus.last_config, 0x6120);
}
TEST(RAK10724Power, EachRegisterFailureInvalidatesOldReading) {
  for (int reg : {1, 2, 3, 6}) {
    rak10724::Snapshot s; s.power_valid = true; Bus bus; bus.fail_register = reg;
    EXPECT_FALSE(rak10724::samplePower(bus, s)); EXPECT_FALSE(s.power_valid);
    EXPECT_EQ(bus.last_config, 0x6120);
  }
}
TEST(RAK10724Power, OverflowAndShutdownFailureAreUnavailable) {
  rak10724::Snapshot s; Bus bus; bus.overflow = true;
  EXPECT_FALSE(rak10724::samplePower(bus, s)); EXPECT_FALSE(s.power_valid);
  bus.overflow = false; bus.shutdown_fail = true;
  EXPECT_FALSE(rak10724::samplePower(bus, s)); EXPECT_FALSE(s.power_valid);
}
TEST(RAK10724Environment, CRCUnitsAndSleep) {
  rak10724::Snapshot s; ShtBus bus;
  ASSERT_TRUE(rak10724::sampleEnvironment(bus, s)); EXPECT_TRUE(s.environment_valid);
  EXPECT_NEAR(s.temperature_c, 25, 0.01); EXPECT_NEAR(s.humidity_pct, 50, 0.01);
  EXPECT_EQ(bus.last_command, 0xB098); EXPECT_EQ(bus.reads, 1);
  char body[160];
  ASSERT_GT(rak10724::formatStatus(body, sizeof(body), 3500, s, true, true), 0U);
  EXPECT_NE(strstr(body, "battery=3.50v 42%"), nullptr);
  EXPECT_NE(strstr(body, "temp=77.0F"), nullptr);
}
TEST(RAK10724Environment, CorruptCRCInvalidatesOldReadingAndSleeps) {
  rak10724::Snapshot s; s.environment_valid = true; ShtBus bus; bus.corrupt = true;
  EXPECT_FALSE(rak10724::sampleEnvironment(bus, s)); EXPECT_FALSE(s.environment_valid);
  EXPECT_EQ(bus.last_command, 0xB098);
}
TEST(RAK10724Environment, NACKDoesNotRetryForever) {
  rak10724::Snapshot s; ShtBus bus; bus.fail = true;
  EXPECT_FALSE(rak10724::sampleEnvironment(bus, s)); EXPECT_FALSE(s.environment_valid);
  EXPECT_EQ(bus.last_command, 0xB098); EXPECT_LE(bus.reads, 1); EXPECT_EQ(bus.waits, 2);
}
TEST(RAK10724Environment, ShutdownFailureIsUnavailable) {
  rak10724::Snapshot s; ShtBus bus; bus.shutdown_fail = true;
  EXPECT_FALSE(rak10724::sampleEnvironment(bus, s)); EXPECT_FALSE(s.environment_valid);
}
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv); return RUN_ALL_TESTS();
}
