#pragma once

#include <MeshCore.h>
#include <Arduino.h>
#include <Wire.h>
#include <helpers/NRF52Board.h>

#ifdef XIAO_NRF52

class MeshsmithPhotonNRFBoard : public NRF52BoardDCDC {
public:
  MeshsmithPhotonNRFBoard() : NRF52Board("XIAO_NRF52_OTA") {}
  void begin();

#if defined(P_LORA_TX_LED)
  void onBeforeTransmit() override {
    digitalWrite(P_LORA_TX_LED, LOW);   // turn TX LED on
    #if defined(LED_BLUE)
       // turn off that annoying blue LED before transmitting
       digitalWrite(LED_BLUE, HIGH);
    #endif
  }
  void onAfterTransmit() override {
    digitalWrite(P_LORA_TX_LED, HIGH);   // turn TX LED off
    #if defined(LED_BLUE)
       // do it after transmitting too, just in case
       digitalWrite(LED_BLUE, HIGH);
    #endif
  }
#endif

  uint16_t getBattMilliVolts() override {
    // Read voltage from MAX17048G fuel gauge via I2C
    // MAX17048 I2C address is 0x36, VCELL register is 0x02
    // Returns voltage in 78.125µV units (1.25mV / 16)
    Wire.beginTransmission(0x36);
    Wire.write(0x02);  // VCELL register
    Wire.endTransmission(false);
    Wire.requestFrom(0x36, 2);
    if (Wire.available() >= 2) {
      uint16_t vcell = (Wire.read() << 8) | Wire.read();
      // Convert to millivolts: vcell * 78.125µV / 1000 = vcell * 0.078125 mV
      // Simplified: (vcell * 5) / 64 gives mV
      return (uint32_t)(vcell * 5) / 64;
    }
    return 0;  // Return 0 if read fails
  }

  const char *getManufacturerName() const override {
    return MANUFACTURER_STRING;
  }
};

#endif
