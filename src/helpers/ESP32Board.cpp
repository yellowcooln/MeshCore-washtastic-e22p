#ifdef ESP_PLATFORM

#include "ESP32Board.h"
#include <target.h>

#if defined(ADMIN_PASSWORD) && !defined(DISABLE_WIFI_OTA)   // Repeater or Room Server only
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <SPIFFS.h>
#include <esp_arduino_version.h>

#if defined(OTA_OVER_ETHERNET)
  #include <ETH.h>
#endif

namespace {

#if defined(OTA_OVER_ETHERNET)
void sanitizeHostname(const char* input, char output[], size_t len) {
  if (len == 0) return;

  size_t out = 0;
  bool last_dash = false;
  while (input && *input && out + 1 < len) {
    char c = *input++;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
      if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
      output[out++] = c;
      last_dash = false;
    } else if ((c == ' ' || c == '_' || c == '-') && out > 0 && !last_dash) {
      output[out++] = '-';
      last_dash = true;
    }
  }

  while (out > 0 && output[out - 1] == '-') out--;

  if (out == 0) {
    const char* fallback = "meshcore-ota";
    while (*fallback && out + 1 < len) {
      output[out++] = *fallback++;
    }
  }

  output[out] = 0;
}

bool startEthernetForOTA(const char* hostname, IPAddress& ip) {
  char safe_hostname[33];
  sanitizeHostname(hostname, safe_hostname, sizeof(safe_hostname));

  #if defined(ETH_PHY_RESET) && ETH_PHY_RESET >= 0
    pinMode(ETH_PHY_RESET, OUTPUT);
    digitalWrite(ETH_PHY_RESET, LOW);
    delay(50);
    digitalWrite(ETH_PHY_RESET, HIGH);
    delay(50);
  #endif

  ETH.setHostname(safe_hostname);
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    if (!ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE)) {
      return false;
    }
  #else
    if (!ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE)) {
      return false;
    }
  #endif

  unsigned long started = millis();
  while (millis() - started < 10000) {
    ip = ETH.localIP();
    if (ip != IPAddress()) {
      return true;
    }
    delay(100);
  }
  return false;
}
#endif

}

bool ESP32Board::startOTAUpdate(const char* id, char reply[]) {
  inhibit_sleep = true;   // prevent sleep during OTA
  IPAddress ota_ip;

  #if defined(OTA_OVER_ETHERNET)
    if (!startEthernetForOTA(id, ota_ip)) {
      strcpy(reply, "Error: Ethernet OTA failed");
      return false;
    }
  #else
    WiFi.softAP("MeshCore-OTA", NULL);
    ota_ip = WiFi.softAPIP();
  #endif

  sprintf(reply, "Started: http://%s/update", ota_ip.toString().c_str());
  MESH_DEBUG_PRINTLN("startOTAUpdate: %s", reply);

  static char id_buf[60];
  sprintf(id_buf, "%s (%s)", id, getManufacturerName());
  #if defined(OTA_OVER_ETHERNET)
    static char home_buf[256];
    sprintf(home_buf,
            "<h2>MeshCore Repeater OTA</h2><p>ID: %s</p><p><a href='/update'>Open upload page</a></p>",
            id);
  #else
    static char home_buf[90];
    sprintf(home_buf, "<H2>Hi! I am a MeshCore Repeater. ID: %s</H2>", id);
  #endif

  AsyncWebServer* server = new AsyncWebServer(80);

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", home_buf);
  });
  server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/packet_log", "text/plain");
  });

  AsyncElegantOTA.setID(id_buf);
  AsyncElegantOTA.begin(server);    // Start ElegantOTA
  server->begin();

  return true;
}

#else
bool ESP32Board::startOTAUpdate(const char* id, char reply[]) {
  return false; // not supported
}
#endif

void ESP32Board::powerOff() {
  enterDeepSleep(0); // Do not wakeup
}

void ESP32Board::enterDeepSleep(uint32_t secs) {
  // Power off the display if any
#ifdef DISPLAY_CLASS
  display.turnOff();
#endif

  // Power off LoRa
  radio_driver.powerOff();

  // Keep LoRa inactive during deepsleep
  digitalWrite(P_LORA_NSS, HIGH);
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
  gpio_hold_en((gpio_num_t)P_LORA_NSS);
#else
  rtc_gpio_hold_en((gpio_num_t)P_LORA_NSS);
#endif

  // Power off GPS if any
  if (sensors.getLocationProvider() != NULL) {
    sensors.getLocationProvider()->stop();
  }

  // Flush serial buffers
  Serial.flush();
  delay(100);

  // Clear stale wakeup sources to avoid ghost wakeup
  // This is required when Power Management and automatic lightsleep are enabled
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  if (secs > 0) {
    esp_sleep_enable_timer_wakeup(secs * 1000000ULL);
  }

  // Finally set ESP32 into deepsleep
  esp_deep_sleep_start(); // CPU halts here and never returns!
}
#endif
