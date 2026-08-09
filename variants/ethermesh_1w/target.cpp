#include <Arduino.h>
#include "target.h"

EtherMeshBoard board;

static SPIClass spi;
RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);
WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);
SensorManager sensors;

bool radio_init() {
  fallback_clock.begin();
  rtc_clock.begin(Wire);

#ifdef P_LORA_RF_EN
  pinMode(P_LORA_RF_EN, OUTPUT);
  digitalWrite(P_LORA_RF_EN, HIGH);
#endif

  spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI, P_LORA_NSS);

  bool success = radio.std_init(&spi);
  if (success) {
    radio.setTxParams(LORA_TX_POWER, RADIOLIB_SX126X_PA_RAMP_800U);
  }
  return success;
}

uint32_t radio_get_rng_seed() {
  return radio.random(0x7FFFFFFF);
}

void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr) {
  radio.setFrequency(freq);
  radio.setSpreadingFactor(sf);
  radio.setBandwidth(bw);
  radio.setCodingRate(cr);
}

void radio_set_tx_power(int8_t dbm) {
  radio.setOutputPower(dbm);
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}
