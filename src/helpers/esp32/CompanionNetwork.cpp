#include "CompanionNetwork.h"

#include <ctype.h>
#include <esp_arduino_version.h>

#if defined(WIFI_SSID)
  #include <WiFi.h>
#elif defined(ETHERNET_COMPANION)
  #include <ETH.h>
#endif

namespace {

void sanitizeHostname(const char* input, char output[], size_t len) {
  if (len == 0) return;

  size_t out = 0;
  bool last_was_dash = false;
  while (input && *input && out + 1 < len) {
    char c = *input++;
    if (isalnum((unsigned char)c)) {
      output[out++] = (char)tolower((unsigned char)c);
      last_was_dash = false;
    } else if ((c == ' ' || c == '_' || c == '-') && out > 0 && !last_was_dash) {
      output[out++] = '-';
      last_was_dash = true;
    }
  }

  while (out > 0 && output[out - 1] == '-') {
    out--;
  }

  if (out == 0) {
    const char* fallback = "meshcore";
    while (*fallback && out + 1 < len) {
      output[out++] = *fallback++;
    }
  }

  output[out] = '\0';
}

#if defined(ETHERNET_COMPANION)
void resetPhy() {
  #if defined(ETH_PHY_RESET) && ETH_PHY_RESET >= 0
    pinMode(ETH_PHY_RESET, OUTPUT);
    digitalWrite(ETH_PHY_RESET, LOW);
    delay(50);
    digitalWrite(ETH_PHY_RESET, HIGH);
    delay(50);
  #endif
}
#endif

}

namespace companion_network {

bool begin(const char* hostname) {
  char safe_hostname[33];
  sanitizeHostname(hostname, safe_hostname, sizeof(safe_hostname));

#if defined(WIFI_SSID)
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(safe_hostname);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  return true;
#elif defined(ETHERNET_COMPANION)
  resetPhy();
  ETH.setHostname(safe_hostname);
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    return ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
  #else
    return ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  #endif
#else
  (void)safe_hostname;
  return false;
#endif
}

void loop() {
}

bool hasIP() {
#if defined(WIFI_SSID)
  return WiFi.localIP() != IPAddress();
#elif defined(ETHERNET_COMPANION)
  return ETH.localIP() != IPAddress();
#else
  return false;
#endif
}

IPAddress localIP() {
#if defined(WIFI_SSID)
  return WiFi.localIP();
#elif defined(ETHERNET_COMPANION)
  return ETH.localIP();
#else
  return IPAddress();
#endif
}

}
